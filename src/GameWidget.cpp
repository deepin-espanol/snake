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
#include <QSettings>

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
    mainLayout->addStretch();  // marcadores arriba, sobre el tablero

    m_record = QSettings().value("record", 0).toInt();
    connect(&m_timer, &QTimer::timeout, this, &GameWidget::onTick);
    nuevoJuego();
}

int GameWidget::intervaloTimer() const
{
    return qMax(50, 140 - (m_nivel - 1) * 10);
}

void GameWidget::actualizarUI()
{
    m_labelPuntos->setText(tr("Puntos: %1  |  Récord: %2").arg(m_puntos).arg(qMax(m_puntos, m_record)));
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
    m_colaDirecciones.clear();
    m_victoria = false;
    m_esperando = true;
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

void GameWidget::alternarPausa()
{
    if (!m_juegoActivo) return;
    m_pausado = !m_pausado;
    update();
}

// Cada giro se aplica en su propio tick y se valida contra la direccion
// que tendra la serpiente en ese momento: asi dos teclas rapidas (p. ej.
// arriba + izquierda yendo a la derecha) ya no la hacen chocar consigo misma.
void GameWidget::encolarDireccion(Direccion d)
{
    Direccion ultima = m_colaDirecciones.isEmpty() ? m_direccion : m_colaDirecciones.last();
    auto opuesta = [](Direccion a, Direccion b) {
        return (a == Arriba && b == Abajo) || (a == Abajo && b == Arriba)
            || (a == Izquierda && b == Derecha) || (a == Derecha && b == Izquierda);
    };
    if (d == ultima || opuesta(d, ultima)) return;
    if (m_colaDirecciones.size() < 3)
        m_colaDirecciones.enqueue(d);
}

void GameWidget::terminar(bool victoria)
{
    m_juegoActivo = false;
    m_victoria = victoria;
    m_timer.stop();
    if (m_puntos > m_record) {
        m_record = m_puntos;
        QSettings().setValue("record", m_record);
    }
    actualizarUI();
    emit juegoTerminado(m_puntos);
    update();
}

void GameWidget::onTick()
{
    if (!m_juegoActivo || m_pausado || m_esperando) return;

    if (!m_colaDirecciones.isEmpty())
        m_direccion = m_colaDirecciones.dequeue();

    QPoint cabeza = m_serpiente.first();

    switch (m_direccion) {
    case Arriba:    cabeza.setY(cabeza.y() - 1); break;
    case Abajo:     cabeza.setY(cabeza.y() + 1); break;
    case Izquierda: cabeza.setX(cabeza.x() - 1); break;
    case Derecha:   cabeza.setX(cabeza.x() + 1); break;
    }

    if (cabeza.x() < 0 || cabeza.x() >= ANCHO || cabeza.y() < 0 || cabeza.y() >= ALTO) {
        terminar(false);
        return;
    }

    // La ultima casilla de la cola se libera en este mismo movimiento
    // (salvo que coma), asi que perseguir la cola no es un choque.
    const bool come = (cabeza == m_comida);
    const int revisar = come ? m_serpiente.size() : m_serpiente.size() - 1;
    for (int i = 0; i < revisar; ++i) {
        if (m_serpiente[i] == cabeza) {
            terminar(false);
            return;
        }
    }

    m_serpiente.prepend(cabeza);

    if (come) {
        m_puntos += 10;
        actualizarNivel();
        actualizarUI();
        emit puntosCambiados(m_puntos);
        if (m_serpiente.size() >= ANCHO * ALTO) {
            terminar(true);  // tablero lleno
            return;
        }
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

    if (m_esperando && m_juegoActivo && !m_pausado) {
        painter.setPen(fg);
        QFont font = painter.font();
        font.setPixelSize(16);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(0, TOP_BAR + ALTO * CELDA / 2 + 30, width(), 30), Qt::AlignCenter,
                         tr("Pulsa una flecha (o WASD) para empezar"));
    }

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
        painter.drawText(rect(), Qt::AlignCenter, m_victoria ? tr("¡GANASTE!") : tr("GAME OVER"));

        font.setPixelSize(16);
        font.setBold(false);
        painter.setFont(font);
        painter.drawText(rect().adjusted(0, 38, 0, 38), Qt::AlignCenter,
                         tr("Puntuación: %1  |  Nivel: %2  |  Récord: %3").arg(m_puntos).arg(m_nivel).arg(m_record));

        font.setPixelSize(13);
        painter.setFont(font);
        painter.drawText(rect().adjusted(0, 68, 0, 68), Qt::AlignCenter,
                         tr("Presiona Enter para reiniciar"));
    }
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_P || event->key() == Qt::Key_Escape) {
        alternarPausa();
        return;
    }

    if (!m_juegoActivo) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            nuevoJuego();
        return;
    }

    if (m_pausado) return;

    switch (event->key()) {
    case Qt::Key_Up: case Qt::Key_W: case Qt::Key_Down: case Qt::Key_S:
    case Qt::Key_Left: case Qt::Key_A: case Qt::Key_Right: case Qt::Key_D:
        // La primera tecla arranca la partida (ir a la izquierda no es valido al inicio)
        if (m_esperando && event->key() != Qt::Key_Left && event->key() != Qt::Key_A)
            m_esperando = false;
        break;
    default:
        break;
    }

    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        encolarDireccion(Arriba);
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        encolarDireccion(Abajo);
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        encolarDireccion(Izquierda);
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        encolarDireccion(Derecha);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void GameWidget::focusOutEvent(QFocusEvent *event)
{
    if (m_juegoActivo && !m_pausado && !m_esperando) {
        m_pausado = true;
        update();
    }
    QWidget::focusOutEvent(event);
}
