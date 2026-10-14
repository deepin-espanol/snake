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
    resize(490, 540);
    setMinimumSize(490, 540);
    setMaximumSize(490, 540);

    m_gameWidget = new GameWidget(this);
    setCentralWidget(m_gameWidget);
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);

    m_btnNuevoJuego = new QPushButton(tr("Nuevo juego"));
    m_btnNuevoJuego->setFixedHeight(30);
    connect(m_btnNuevoJuego, &QPushButton::clicked, m_gameWidget, &GameWidget::nuevoJuego);
    titlebar()->addWidget(m_btnNuevoJuego, Qt::AlignLeft);

    QMenu *menu = new QMenu(this);
    QAction *quitAction = menu->addAction(tr("Salir"));
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    titlebar()->setMenu(menu);

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
