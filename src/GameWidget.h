#ifndef SNAKE_GAMEWIDGET_H
#define SNAKE_GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPoint>

class QLabel;

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    enum Direccion { Arriba, Abajo, Izquierda, Derecha };

    explicit GameWidget(QWidget *parent = nullptr);

    void nuevoJuego();
    int nivel() const { return m_nivel; }
    int puntos() const { return m_puntos; }

signals:
    void puntosCambiados(int puntos);
    void nivelCambiado(int nivel);
    void juegoTerminado(int puntuacionFinal);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTick();

private:
    void colocarComida();
    void actualizarNivel();
    void actualizarUI();
    int intervaloTimer() const;

    static const int CELDA = 22;
    static const int ANCHO = 20;
    static const int ALTO = 20;
    static const int PUNTOS_POR_NIVEL = 50;
    static const int TOP_BAR = 36;

    QVector<QPoint> m_serpiente;
    QPoint m_comida;
    Direccion m_direccion;
    QTimer m_timer;
    int m_puntos = 0;
    int m_nivel = 1;
    bool m_juegoActivo = false;
    bool m_pausado = false;

    QLabel *m_labelPuntos = nullptr;
    QLabel *m_labelNivel = nullptr;
};

#endif
