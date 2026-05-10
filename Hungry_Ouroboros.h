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
#include <QTime>
#include <QPointF>
#include <cmath>

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
    void updateAnimation();             // 更新动画进度

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
    bool m_paused;   // 暂停标志
    // 平滑动画相关
    QTime m_lastMoveTime;
    QVector<QPointF> m_prevPositions;
    QVector<QPointF> m_currPositions;
    float m_animProgress;
    bool m_isMoving;
    QTimer *m_animationTimer;
    double m_flashPhase;
    // 食物平滑移动
    bool m_foodMoving;
    QPointF m_foodPrevPos;
    QPointF m_foodCurrPos;
    float m_foodAnimProgress;
    QTime m_foodMoveStartTime;
    // 食物穿墙动画
    bool m_warpAnimActive;
    float m_warpAnimProgress;
    QPointF m_warpStartPos;
    QPointF m_warpEndPos;
    QTime m_warpAnimStartTime;
    float m_warpDuration;

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
    float m_warpProbability;
    bool m_lastMoveWasWarp;

    // 闪烁特效
    bool m_snakeFlashing;            // 是否正在闪烁
    int m_flashCounter;              // 闪烁计数器（用于控制闪烁次数）
    float m_flashIntensity;          // 当前透明度/亮度（0~1）

    // 小星星粒子
    struct StarParticle {
        QPointF pos;
        QPointF vel;
        float life;
        float size;
        QColor color;
    };
    QVector<StarParticle> m_stars;
    void addStarEffect(const QPointF &pos);   // 添加一组星星

    void InitSnake();
    QRect CreatRect();
    void GenerateObstacles();
    void eatFood();
    void StartGame();
    void updateHighScore();
    QPointF getAnimatedPosition(int index) const;
    void exitToMenu();
};

#endif // HUNGRY_OUROBOROS_H
