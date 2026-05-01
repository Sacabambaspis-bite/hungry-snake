#ifndef HUNGRY_OUROBOROS_H
#define HUNGRY_OUROBOROS_H

#include <QMainWindow>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QRect>
#include <QVector>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>

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
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onGameUpdate();

private:
    Ui::MainWindow *ui;

    // 游戏状态
    bool blsrun;
    bool blsover;
    bool isStart;
    bool m_inWelcomeScreen;
    bool m_isHardMode;

    // 游戏数据
    int speed;
    int nDirection;
    int nScore;
    QString sDisplay;
    QString ScoreLabel;

    QRect food;
    QRect SnakeHead;
    QVector<QRect> vSnakeRect;
    QVector<QRect> m_obstacles;

    QTimer *timer;

    // 欢迎界面按钮区域
    QRect m_normalBtnRect;
    QRect m_hardBtnRect;
    QRect m_exitBtnRect;

    // 游戏结束菜单按钮区域
    QRect m_restartBtnRect;      // “再来一局”按钮
    QRect m_backToMenuBtnRect;   // “返回主菜单”按钮

    // 初始化与辅助函数
    void InitSnake();
    QRect CreatRect();
    void IsEat();
    void IsHit();
    void GenerateObstacles();
    void StartGame();

    // 音效播放器
    QMediaPlayer *m_eatPlayer;
    QMediaPlayer *m_gameoverPlayer;
    QMediaPlayer *m_bgmPlayer;
};

#endif // HUNGRY_OUROBOROS_H