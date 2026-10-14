#include "GameWidget.h"

#include <QPainter>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QFont>
#include <QLinearGradient>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , m_direccion(Derecha)
    , m_juegoActivo(false)
    , m_pausado(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(ANCHO * CELDA + 20, ALTO * CELDA + TOP_BAR + 16);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 6, 10, 8);
    mainLayout->setSpacing(4);

    auto *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    m_labelPuntos = new QLabel(tr("Puntos: 0"), this);
    m_labelPuntos->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QFont puntosFont = m_labelPuntos->font();
    puntosFont.setPointSize(10);
    puntosFont.setBold(true);
    m_labelPuntos->setFont(puntosFont);

    m_labelNivel = new QLabel(tr("Nivel: 1"), this);
    m_labelNivel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont nivelFont = m_labelNivel->font();
    nivelFont.setPointSize(10);
    nivelFont.setBold(true);
    m_labelNivel->setFont(nivelFont);

    topLayout->addWidget(m_labelPuntos);
    topLayout->addStretch();
    topLayout->addWidget(m_labelNivel);

    mainLayout->addLayout(topLayout);

    connect(&m_timer, &QTimer::timeout, this, &GameWidget::onTick);
    nuevoJuego();
}

int GameWidget::intervaloTimer() const
{
    return qMax(50, 140 - (m_nivel - 1) * 10);
}

void GameWidget::actualizarUI()
{
    m_labelPuntos->setText(tr("Puntos: %1").arg(m_puntos));
    m_labelNivel->setText(tr("Nivel: %1").arg(m_nivel));
}

void GameWidget::actualizarNivel()
{
    int nuevoNivel = 1 + m_puntos / PUNTOS_POR_NIVEL;
    if (nuevoNivel != m_nivel) {
        m_nivel = nuevoNivel;
        m_timer.setInterval(intervaloTimer());
        emit nivelCambiado(m_nivel);
    }
}

void GameWidget::nuevoJuego()
{
    m_serpiente.clear();
    int startX = ANCHO / 2;
    int startY = ALTO / 2;
    for (int i = 0; i < 3; ++i)
        m_serpiente.append(QPoint(startX - i, startY));

    m_direccion = Derecha;
    m_puntos = 0;
    m_nivel = 1;
    m_juegoActivo = true;
    m_pausado = false;
    m_timer.start(intervaloTimer());

    colocarComida();
    actualizarUI();
    setFocus();
    update();
}

void GameWidget::colocarComida()
{
    QVector<QPoint> vacias;
    for (int x = 0; x < ANCHO; ++x)
        for (int y = 0; y < ALTO; ++y) {
            QPoint p(x, y);
            if (!m_serpiente.contains(p))
                vacias.append(p);
        }
    if (!vacias.isEmpty())
        m_comida = vacias[QRandomGenerator::global()->bounded(vacias.size())];
}

void GameWidget::onTick()
{
    if (!m_juegoActivo || m_pausado) return;

    QPoint cabeza = m_serpiente.first();

    switch (m_direccion) {
    case Arriba:    cabeza.setY(cabeza.y() - 1); break;
    case Abajo:     cabeza.setY(cabeza.y() + 1); break;
    case Izquierda: cabeza.setX(cabeza.x() - 1); break;
    case Derecha:   cabeza.setX(cabeza.x() + 1); break;
    }

    if (cabeza.x() < 0 || cabeza.x() >= ANCHO || cabeza.y() < 0 || cabeza.y() >= ALTO) {
        m_juegoActivo = false;
        m_timer.stop();
        emit juegoTerminado(m_puntos);
        update();
        return;
    }

    if (m_serpiente.contains(cabeza)) {
        m_juegoActivo = false;
        m_timer.stop();
        emit juegoTerminado(m_puntos);
        update();
        return;
    }

    m_serpiente.prepend(cabeza);

    if (cabeza == m_comida) {
        m_puntos += 10;
        actualizarNivel();
        actualizarUI();
        emit puntosCambiados(m_puntos);
        colocarComida();
    } else {
        m_serpiente.removeLast();
    }

    update();
}

void GameWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette pal = QApplication::palette();
    QColor bg = pal.color(QPalette::Window);
    QColor fg = pal.color(QPalette::WindowText);
    QColor mid = pal.color(QPalette::Mid);
    QColor base = pal.color(QPalette::Base);
    QColor altBase = pal.color(QPalette::AlternateBase);

    painter.fillRect(rect(), bg);

    int boardX = 10;
    int boardY = TOP_BAR;
    int boardW = ANCHO * CELDA;
    int boardH = ALTO * CELDA;

    painter.setPen(Qt::NoPen);
    painter.setBrush(base);
    painter.drawRoundedRect(boardX - 2, boardY - 2, boardW + 4, boardH + 4, 8, 8);

    painter.setPen(QPen(mid, 1));
    painter.setBrush(Qt::NoBrush);
    for (int x = 0; x <= ANCHO; ++x)
        painter.drawLine(boardX + x * CELDA, boardY, boardX + x * CELDA, boardY + boardH);
    for (int y = 0; y <= ALTO; ++y)
        painter.drawLine(boardX, boardY + y * CELDA, boardX + boardW, boardY + y * CELDA);

    int total = m_serpiente.size();
    for (int i = total - 1; i >= 0; --i) {
        const QPoint &p = m_serpiente[i];
        QColor c;
        if (i == 0) {
            c = QColor(30, 200, 90);
        } else {
            double ratio = static_cast<double>(i) / qMax(1, total);
            int g = static_cast<int>(200 - ratio * 80);
            int r = static_cast<int>(30 + ratio * 10);
            int b = static_cast<int>(90 - ratio * 50);
            c = QColor(r, g, b);
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(c);
        painter.drawRoundedRect(boardX + p.x() * CELDA + 1, boardY + p.y() * CELDA + 1,
                               CELDA - 2, CELDA - 2, 5, 5);

        if (i == 0) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, 50));
            int ex = boardX + p.x() * CELDA + 3;
            int ey = boardY + p.y() * CELDA + 3;
            painter.drawEllipse(ex, ey, CELDA / 2 - 2, CELDA / 2 - 2);
        }
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(230, 60, 60));
    int fx = boardX + m_comida.x() * CELDA + CELDA / 2;
    int fy = boardY + m_comida.y() * CELDA + CELDA / 2;
    painter.drawEllipse(fx - 7, fy - 7, 14, 14);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 60));
    painter.drawEllipse(fx - 3, fy - 5, 5, 4);

    if (m_pausado && m_juegoActivo) {
        painter.fillRect(rect(), QColor(0, 0, 0, 130));
        painter.setPen(fg);
        QFont font = painter.font();
        font.setPixelSize(30);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, tr("PAUSA"));
        font.setPixelSize(14);
        font.setBold(false);
        painter.setFont(font);
        painter.drawText(rect().adjusted(0, 32, 0, 32), Qt::AlignCenter,
                         tr("Presiona P para continuar"));
    }

    if (!m_juegoActivo) {
        painter.fillRect(rect(), QColor(0, 0, 0, 170));

        painter.setPen(fg);
        QFont font = painter.font();
        font.setPixelSize(32);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, tr("GAME OVER"));

        font.setPixelSize(16);
        font.setBold(false);
        painter.setFont(font);
        painter.drawText(rect().adjusted(0, 38, 0, 38), Qt::AlignCenter,
                         tr("Puntuacion: %1  |  Nivel: %2").arg(m_puntos).arg(m_nivel));

        font.setPixelSize(13);
        painter.setFont(font);
        painter.drawText(rect().adjusted(0, 68, 0, 68), Qt::AlignCenter,
                         tr("Presiona Enter para reiniciar"));
    }
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_P || event->key() == Qt::Key_Escape) {
        if (m_juegoActivo) {
            m_pausado = !m_pausado;
            update();
        }
        return;
    }

    if (!m_juegoActivo) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            nuevoJuego();
        return;
    }

    if (m_pausado) return;

    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        if (m_direccion != Abajo) m_direccion = Arriba;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        if (m_direccion != Arriba) m_direccion = Abajo;
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        if (m_direccion != Derecha) m_direccion = Izquierda;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        if (m_direccion != Izquierda) m_direccion = Derecha;
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
