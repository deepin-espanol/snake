#ifndef SNAKE_MAINWINDOW_H
#define SNAKE_MAINWINDOW_H

#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class GameWidget;
class QPushButton;

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    GameWidget *m_gameWidget = nullptr;
    QPushButton *m_btnNuevoJuego = nullptr;
};

#endif
