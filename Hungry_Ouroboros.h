#ifndef HUNGRY_OUROBOROS_H
#define HUNGRY_OUROBOROS_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QRect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onGameUpdate();
    void moveFood();            // 食物移动槽函数

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QVector<QRect> vSnakeRect;
    QPixmap m_snakeBodyPixmap;
    QRect food;
    bool blsrun;
    int speed;
    bool isStart;
    bool blsover;
    QString sDisplay;
    QString ScoreLabel;
    int nScore;
    int nDirection;
    QRect SnakeHead;

    // 音效
    QMediaPlayer *m_eatPlayer;
    QMediaPlayer *m_gameoverPlayer;
    QMediaPlayer *m_bgmPlayer;

    // 界面状态
    bool m_inWelcomeScreen;
    bool m_isHardMode;
    QRect m_normalBtnRect;
    QRect m_hardBtnRect;
    QRect m_exitBtnRect;
    QRect m_restartBtnRect;
    QRect m_backToMenuBtnRect;

    // 障碍物
    QVector<QRect> m_obstacles;

    // 最高分
    int m_normalHighScore;
    int m_hardHighScore;

    // 食物移动相关
    bool m_foodMoveEnabled;
    QTimer *m_foodMoveTimer;
    int m_foodMoveInterval;

    void InitSnake();
    QRect CreatRect();
    void GenerateObstacles();
    void eatFood();
    void StartGame();
    void updateHighScore();
};

#endif // HUNGRY_OUROBOROS_H