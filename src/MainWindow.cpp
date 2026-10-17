#include "MainWindow.h"
#include "GameWidget.h"

#include <DTitlebar>
#include <QAction>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    setWindowTitle("Snake");

    m_gameWidget = new GameWidget(this);
    setCentralWidget(m_gameWidget);

    m_btnNuevoJuego = new QPushButton(tr("Nuevo juego"));
    m_btnNuevoJuego->setFixedHeight(30);
    connect(m_btnNuevoJuego, &QPushButton::clicked, m_gameWidget, &GameWidget::nuevoJuego);
    titlebar()->addWidget(m_btnNuevoJuego, Qt::AlignLeft);

    QMenu *menu = new QMenu(this);
    QAction *newAction = menu->addAction(tr("Nuevo juego"));
    newAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(newAction, &QAction::triggered, m_gameWidget, &GameWidget::nuevoJuego);
    // La tecla P la gestiona el propio tablero
    QAction *pauseAction = menu->addAction(tr("Pausa (P)"));
    connect(pauseAction, &QAction::triggered, m_gameWidget, &GameWidget::alternarPausa);
    menu->addSeparator();
    QAction *quitAction = menu->addAction(tr("Salir"));
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    titlebar()->setMenu(menu);
    // Los atajos del menu de la barra de titulo solo funcionan con el menu
    // abierto; anadirlas tambien a la ventana los activa siempre.
    addAction(newAction);
    addAction(quitAction);

    // El tamano de la ventana incluye la barra de titulo de DTK
    setFixedSize(m_gameWidget->width(), m_gameWidget->height() + titlebar()->sizeHint().height());
    m_btnNuevoJuego->setFocusPolicy(Qt::NoFocus);

    connect(m_gameWidget, &GameWidget::juegoTerminado, this, [this](int puntos) {
        setWindowTitle(tr("Snake - Game Over (%1 pts)").arg(puntos));
    });
    connect(m_gameWidget, &GameWidget::puntosCambiados, this, [this](int puntos) {
        setWindowTitle(tr("Snake - %1 pts").arg(puntos));
    });
    connect(m_gameWidget, &GameWidget::nivelCambiado, this, [this](int nivel) {
        setWindowTitle(tr("Snake - Nivel %1").arg(nivel));
    });
}
