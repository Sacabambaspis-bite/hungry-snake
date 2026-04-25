#ifndef HUNGRY_OUROBOROS_H
#define HUNGRY_OUROBOROS_H

#include <QMainWindow>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QRect>
#include <QVector>
#include <QSoundEffect>      // 新增：用于播放短音效
#include <QMediaPlayer>      // 新增：用于播放背景音乐
#include <QAudioOutput>      // 新增：QMediaPlayer 的音频输出（Qt6 必需）

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
    void onGameUpdate();   // 原 mainwindow_update 重命名

private:
    Ui::MainWindow *ui;

    // 游戏状态
    bool blsrun;           // 是否已完成初始化
    bool blsover;          // 游戏是否结束
    bool isStart;          // 游戏是否正在运行（蛇在移动）
    bool m_inWelcomeScreen;// 是否显示欢迎界面

    // 游戏数据
    int speed;             // 计时器间隔（毫秒）
    int nDirection;        // 1:上 2:下 3:左 4:右
    int nScore;
    QString sDisplay;
    QString ScoreLabel;

    QRect food;
    QRect SnakeHead;
    QVector<QRect> vSnakeRect;

    QTimer *timer;

    // 欢迎界面按钮区域
    QRect m_yesButtonRect;
    QRect m_noButtonRect;

    // 初始化函数
    void InitSnake();
    QRect CreatRect();
    void IsEat();
    void IsHit();

    // 音效相关成员
    QMediaPlayer *m_eatPlayer;        // 吃食物音效播放器
    QMediaPlayer *m_gameoverPlayer;   // 游戏结束音效播放器
    QMediaPlayer *m_bgmPlayer;        // 背景音乐播放器
};

#endif // HUNGRY_OUROBOROS_H