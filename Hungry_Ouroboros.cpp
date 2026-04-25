#include "Hungry_Ouroboros.h"
#include "ui_Hungry_Ouroboros.h"
#include <QRandomGenerator>
#include <QFontMetrics>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , blsrun(false)
    , speed(500)
    , isStart(false)
    , blsover(false)
    , m_inWelcomeScreen(true)    // 启动时显示欢迎界面
    , nScore(0)
    , nDirection(2)
    // ========== 新增：初始化音效对象 ==========
    , m_eatPlayer(nullptr)
    , m_gameoverPlayer(nullptr)
    , m_bgmPlayer(nullptr)
{
    ui->setupUi(this);
    this->setGeometry(QRect(500, 250, 500, 520));

    // 创建计时器（只创建一次）
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onGameUpdate);

    // ========== 新增：初始化音效 ==========
    // 1. 吃食物音效（使用 QSoundEffect）
    m_eatPlayer = new QMediaPlayer(this);
    m_eatPlayer->setAudioOutput(new QAudioOutput(this));
    m_eatPlayer->audioOutput()->setVolume(0.5f);
    m_eatPlayer->setSource(QUrl("qrc:/sounds/btn15.mp3"));            // 音量为 50%

    // 2. 游戏结束音效
    m_gameoverPlayer = new QMediaPlayer(this);
    m_gameoverPlayer->setAudioOutput(new QAudioOutput(this));
    m_gameoverPlayer->audioOutput()->setVolume(0.5f);
    m_gameoverPlayer->setSource(QUrl("qrc:/sounds/btn09.mp3"));

    // 3. 背景音乐（使用 QMediaPlayer，支持循环播放）
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmPlayer->setAudioOutput(new QAudioOutput(this));
    m_bgmPlayer->audioOutput()->setVolume(0.3f);
    m_bgmPlayer->setSource(QUrl("qrc:/sounds/MusMus-BGM-186.mp3"));
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // ================= 欢迎界面 =================
    if (m_inWelcomeScreen) {
        painter.fillRect(rect(), QColor(30, 30, 30)); // 深色背景

        // 标题
        QFont titleFont("Courier New", 30, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(Qt::cyan);
        QString title = "贪吃蛇";
        QFontMetrics fmTitle(titleFont);
        int titleWidth = fmTitle.horizontalAdvance(title);
        int titleX = (width() - titleWidth) / 2;
        painter.drawText(titleX, 150, title);

        // 提示
        QFont promptFont("Arial", 16);
        painter.setFont(promptFont);
        painter.setPen(Qt::white);
        QString prompt = "开始游戏吗？";
        QFontMetrics fmPrompt(promptFont);
        int promptWidth = fmPrompt.horizontalAdvance(prompt);
        int promptX = (width() - promptWidth) / 2;
        painter.drawText(promptX, 220, prompt);

        // 按钮尺寸和位置
        int btnWidth = 100;
        int btnHeight = 40;
        int btnY = 300;
        int yesBtnX = width() / 2 - btnWidth - 20;
        int noBtnX = width() / 2 + 20;

        m_yesButtonRect = QRect(yesBtnX, btnY, btnWidth, btnHeight);
        m_noButtonRect = QRect(noBtnX, btnY, btnWidth, btnHeight);

        // “是”按钮
        painter.setPen(Qt::darkGreen);
        painter.setBrush(Qt::green);
        painter.drawRoundedRect(m_yesButtonRect, 10, 10);
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(m_yesButtonRect, Qt::AlignCenter, "是");

        // “否”按钮
        painter.setPen(Qt::darkRed);
        painter.setBrush(Qt::red);
        painter.drawRoundedRect(m_noButtonRect, 10, 10);
        painter.setPen(Qt::white);
        painter.drawText(m_noButtonRect, Qt::AlignCenter, "否");

        return; // 不执行后续游戏绘制
    }

    // ================= 游戏界面 =================
    if (!blsrun)
        InitSnake();

    // 外框
    painter.setPen(Qt::black);
    painter.setBrush(Qt::gray);
    painter.drawRect(10, 10, 480, 480);

    // 内框背景
    painter.setPen(Qt::darkGray);
    painter.setBrush(Qt::black);
    painter.drawRect(20, 20, 460, 460);
    painter.drawPixmap(20, 20, 460, 460, QPixmap(":/myImages/bg.png"));

    // 网格线
    painter.setPen(Qt::darkGray);
    for (int i = 2; i <= 47; i++) {
        painter.drawLine(20, i * 10, 480, i * 10);
        painter.drawLine(i * 10, 20, i * 10, 480);
    }

    // ----- 绘制蛇和食物（必须在文字之前绘制，避免文字被遮挡）-----
    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::white);
    painter.drawRects(&vSnakeRect[0], vSnakeRect.size());
    painter.drawPixmap(food, QPixmap(":/myImages/fd.png"));

    // ----- 得分文字 -----
    QFont font2("Arial", 10);
    painter.setFont(font2);
    painter.setPen(Qt::darkBlue);
    painter.setBrush(Qt::blue);
    painter.drawText(20, 510, ScoreLabel);
    painter.drawText(60, 510, QString::number(nScore));

    // ----- 游戏状态文字（居中，最上层）-----
    if (!sDisplay.isEmpty()) {
        QFont font1("Courier New", 30);
        painter.setFont(font1);
        painter.setPen(Qt::cyan);
        painter.setBrush(Qt::cyan);

        QFontMetrics fm(font1);
        int textWidth = fm.horizontalAdvance(sDisplay);
        int textHeight = fm.height();
        QRect gameArea(20, 20, 460, 460);
        int x = gameArea.center().x() - textWidth / 2;
        int y = gameArea.center().y() + textHeight / 4;
        painter.drawText(x, y, sDisplay);
    }

    if (blsover)
        timer->stop();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_inWelcomeScreen) {
        QPoint clickPos = event->pos();
        if (m_yesButtonRect.contains(clickPos)) {
            // 点击“是”：进入游戏并直接开始移动
            m_inWelcomeScreen = false;
            InitSnake();                // 会设置 isStart = false，停止计时器
            // 覆盖设置，让游戏直接开始
            isStart = true;
            nDirection = 2;             // 默认向下移动
            timer->start(speed);
            sDisplay = " ";             // 清除“游戏开始”文字

            // ========== 新增：开始游戏时播放背景音乐 ==========
            if (m_bgmPlayer) {
                m_bgmPlayer->play();
            }

            update();
        } else if (m_noButtonRect.contains(clickPos)) {
            // 点击“否”：退出程序
            close();
        }
        return;
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::InitSnake()
{
    blsrun = true;
    blsover = false;
    isStart = false;
    nDirection = 2;        // 默认向下
    sDisplay = "游戏开始";
    ScoreLabel = "得分：";
    nScore = 0;

    // 生成第一个食物（不与蛇身重叠）
    vSnakeRect.resize(5);
    for (int i = 0; i < vSnakeRect.size(); i++) {
        QRect rect(240, 240 + 10 * i, 10, 10);
        vSnakeRect[vSnakeRect.size() - 1 - i] = rect;
    }
    SnakeHead = vSnakeRect.first();
    food = CreatRect();

    // 停止计时器，等待键盘启动（本版本已在点击“是”后自动启动，此处保留原逻辑）
    timer->stop();
}

QRect MainWindow::CreatRect()
{
    QRect newFood;
    bool valid;
    int maxAttempts = 1000;
    do {
        valid = true;
        int x = QRandomGenerator::global()->bounded(0, 42);
        int y = QRandomGenerator::global()->bounded(0, 42);
        newFood = QRect(30 + x * 10, 30 + y * 10, 10, 10);
        for (const QRect &seg : vSnakeRect) {
            if (newFood == seg) {
                valid = false;
                break;
            }
        }
        maxAttempts--;
    } while (!valid && maxAttempts > 0);
    return newFood;
}

void MainWindow::onGameUpdate()
{
    sDisplay = " ";
    SnakeHead = vSnakeRect.first();

    IsEat();
    IsHit();

    // 移动蛇身：将每个节点向后移动
    for (int j = 0; j < vSnakeRect.size() - 1; j++) {
        vSnakeRect[vSnakeRect.size() - 1 - j] = vSnakeRect[vSnakeRect.size() - 2 - j];
    }

    // 根据方向移动蛇头
    switch (nDirection) {
    case 1: // 上
        SnakeHead.setTop(SnakeHead.top() - 10);
        SnakeHead.setBottom(SnakeHead.bottom() - 10);
        break;
    case 2: // 下
        SnakeHead.setTop(SnakeHead.top() + 10);
        SnakeHead.setBottom(SnakeHead.bottom() + 10);
        break;
    case 3: // 左
        SnakeHead.setLeft(SnakeHead.left() - 10);
        SnakeHead.setRight(SnakeHead.right() - 10);
        break;
    case 4: // 右
        SnakeHead.setLeft(SnakeHead.left() + 10);
        SnakeHead.setRight(SnakeHead.right() + 10);
        break;
    default:
        break;
    }
    vSnakeRect[0] = SnakeHead;

    // 边界碰撞检测
    if (SnakeHead.left() < 30 || SnakeHead.right() > 470 ||
        SnakeHead.top() < 30 || SnakeHead.bottom() > 470) {
        sDisplay = "真是一条蠢蛇！";
        blsover = true;

        // ========== 新增：游戏结束时停止背景音乐并播放结束音效 ==========
        if (m_bgmPlayer) {
            m_bgmPlayer->stop();
        }
        if (m_gameoverPlayer) {
            m_gameoverPlayer->play();
        }
    }

    update();
}

void MainWindow::IsEat()
{
    if (SnakeHead == food) {
        SnakeHead = food;
        vSnakeRect.push_back(vSnakeRect.last()); // 增加一节
        food = CreatRect();
        nScore += 10;
        if (speed > 100) {
            speed -= 10;
            timer->stop();
            timer->start(speed);
        }

        // ========== 新增：播放吃食物音效 ==========
        if (m_eatPlayer) {
            m_eatPlayer->stop();   // 先停止（防止连续吃时上一次未播完）
            m_eatPlayer->play();
        }
    }
}

void MainWindow::IsHit()
{
    for (int i = 1; i < vSnakeRect.size(); i++) {
        if (SnakeHead == vSnakeRect[i]) {
            sDisplay = "不可以吃掉自己哦";
            blsover = true;

            // ========== 新增：自身碰撞也触发游戏结束音效和停止背景音乐 ==========
            if (m_bgmPlayer) {
                m_bgmPlayer->stop();
            }
            if (m_gameoverPlayer) {
                m_gameoverPlayer->stop();
                m_gameoverPlayer->play();
            }
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 欢迎界面不响应键盘
    if (m_inWelcomeScreen)
        return;

    int newDir = 0;
    switch (event->key()) {
    case Qt::Key_Up:    newDir = 1; break;
    case Qt::Key_Down:  newDir = 2; break;
    case Qt::Key_Left:  newDir = 3; break;
    case Qt::Key_Right: newDir = 4; break;
    default: return;
    }

    // 游戏结束后，按方向键重置并开始新游戏
    if (blsover) {
        InitSnake();
        // 重置后继续执行以启动游戏
        // ========== 新增：重新开始时播放背景音乐 ==========
        if (m_bgmPlayer) {
            m_bgmPlayer->play();
        }
    }

    // 尚未开始：接受按键并启动计时器
    if (!isStart && !blsover) {
        isStart = true;
        nDirection = newDir;
        timer->start(speed);
        update();
        return;
    }

    // 游戏进行中：防止反向移动
    if (isStart) {
        if ((nDirection == 1 && newDir == 2) ||
            (nDirection == 2 && newDir == 1) ||
            (nDirection == 3 && newDir == 4) ||
            (nDirection == 4 && newDir == 3)) {
            return; // 反向按键无效
        }
        nDirection = newDir;
    }
}