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
    , m_inWelcomeScreen(true)
    , m_isHardMode(false)
    , nScore(0)
    , nDirection(2)
    , m_eatPlayer(nullptr)
    , m_gameoverPlayer(nullptr)
    , m_bgmPlayer(nullptr)
{
    ui->setupUi(this);
    this->setGeometry(QRect(500, 250, 500, 520));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onGameUpdate);

    // ========== 初始化音效 ==========
    m_eatPlayer = new QMediaPlayer(this);
    m_eatPlayer->setAudioOutput(new QAudioOutput(this));
    m_eatPlayer->audioOutput()->setVolume(0.5f);
    m_eatPlayer->setSource(QUrl("qrc:/sounds/btn15.mp3"));

    m_gameoverPlayer = new QMediaPlayer(this);
    m_gameoverPlayer->setAudioOutput(new QAudioOutput(this));
    m_gameoverPlayer->audioOutput()->setVolume(0.5f);
    m_gameoverPlayer->setSource(QUrl("qrc:/sounds/btn09.mp3"));

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
        painter.fillRect(rect(), QColor(30, 30, 30));

        QFont titleFont("Courier New", 30, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(Qt::cyan);
        QString title = "贪吃蛇";
        QFontMetrics fmTitle(titleFont);
        int titleWidth = fmTitle.horizontalAdvance(title);
        int titleX = (width() - titleWidth) / 2;
        painter.drawText(titleX, 120, title);

        QFont promptFont("Arial", 14);
        painter.setFont(promptFont);
        painter.setPen(Qt::white);
        QString prompt = "选择游戏模式";
        QFontMetrics fmPrompt(promptFont);
        int promptWidth = fmPrompt.horizontalAdvance(prompt);
        int promptX = (width() - promptWidth) / 2;
        painter.drawText(promptX, 180, prompt);

        int btnWidth = 140;
        int btnHeight = 40;
        int btnY1 = 250;
        int btnY2 = 310;

        int normalX = (width() - btnWidth) / 2;
        m_normalBtnRect = QRect(normalX, btnY1, btnWidth, btnHeight);

        int hardX = (width() - btnWidth) / 2;
        m_hardBtnRect = QRect(hardX, btnY2, btnWidth, btnHeight);

        int exitBtnWidth = 100;
        int exitX = (width() - exitBtnWidth) / 2;
        m_exitBtnRect = QRect(exitX, 370, exitBtnWidth, btnHeight);

        // 普通模式按钮
        painter.setPen(Qt::darkGreen);
        painter.setBrush(QColor(50, 180, 50));
        painter.drawRoundedRect(m_normalBtnRect, 10, 10);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(m_normalBtnRect, Qt::AlignCenter, "普通模式");

        // 困难模式按钮
        painter.setPen(Qt::darkRed);
        painter.setBrush(QColor(220, 80, 80));
        painter.drawRoundedRect(m_hardBtnRect, 10, 10);
        painter.setPen(Qt::white);
        painter.drawText(m_hardBtnRect, Qt::AlignCenter, "困难模式");

        // 退出按钮
        painter.setPen(Qt::darkGray);
        painter.setBrush(Qt::gray);
        painter.drawRoundedRect(m_exitBtnRect, 10, 10);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(m_exitBtnRect, Qt::AlignCenter, "退出");

        return;
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

    // 障碍物（困难模式）
    if (m_isHardMode) {
        painter.setBrush(QColor(139, 69, 19));
        painter.setPen(Qt::darkYellow);
        for (const QRect &obs : m_obstacles) {
            painter.drawRect(obs);
        }
    }

    // 蛇和食物
    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::white);
    painter.drawRects(&vSnakeRect[0], vSnakeRect.size());
    painter.drawPixmap(food, QPixmap(":/myImages/fd.png"));

    // 得分文字
    QFont font2("Arial", 10);
    painter.setFont(font2);
    painter.setPen(Qt::darkBlue);
    painter.setBrush(Qt::blue);
    painter.drawText(20, 510, ScoreLabel);
    painter.drawText(60, 510, QString::number(nScore));

    // 游戏状态文字（居中，最上层）
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

    // ================= 游戏结束菜单 =================
    if (blsover) {
        // 半透明遮罩
        painter.fillRect(rect(), QColor(0, 0, 0, 150));

        // 提示文字
        QFont overFont("Arial", 20, QFont::Bold);
        painter.setFont(overFont);
        painter.setPen(Qt::white);
        QString overText = "游戏结束";
        QFontMetrics fmOver(overFont);
        int overTextWidth = fmOver.horizontalAdvance(overText);
        int overTextX = (width() - overTextWidth) / 2;
        painter.drawText(overTextX, 220, overText);

        // 按钮尺寸和位置
        int menuBtnWidth = 160;
        int menuBtnHeight = 45;
        int menuBtnY1 = 280;
        int menuBtnY2 = 340;

        int restartX = (width() - menuBtnWidth) / 2;
        m_restartBtnRect = QRect(restartX, menuBtnY1, menuBtnWidth, menuBtnHeight);

        int backX = (width() - menuBtnWidth) / 2;
        m_backToMenuBtnRect = QRect(backX, menuBtnY2, menuBtnWidth, menuBtnHeight);

        // “再来一局”按钮
        painter.setPen(Qt::darkBlue);
        painter.setBrush(QColor(70, 130, 180));
        painter.drawRoundedRect(m_restartBtnRect, 10, 10);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(m_restartBtnRect, Qt::AlignCenter, "再来一局");

        // “返回主菜单”按钮
        painter.setPen(Qt::darkGray);
        painter.setBrush(QColor(180, 180, 180));
        painter.drawRoundedRect(m_backToMenuBtnRect, 10, 10);
        painter.setPen(Qt::black);
        painter.drawText(m_backToMenuBtnRect, Qt::AlignCenter, "返回主菜单");
    }

    if (blsover)
        timer->stop();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 欢迎界面按钮处理
    if (m_inWelcomeScreen) {
        QPoint clickPos = event->pos();

        if (m_normalBtnRect.contains(clickPos)) {
            m_isHardMode = false;
            StartGame();
            return;
        } else if (m_hardBtnRect.contains(clickPos)) {
            m_isHardMode = true;
            StartGame();
            return;
        } else if (m_exitBtnRect.contains(clickPos)) {
            close();
            return;
        }
        return;
    }

    // 游戏结束菜单按钮处理
    if (blsover) {
        QPoint clickPos = event->pos();

        if (m_restartBtnRect.contains(clickPos)) {
            // 再来一局：保持当前模式不变，重新开始
            InitSnake();                // 重置蛇、障碍物、食物等
            isStart = true;
            nDirection = 2;             // 默认向下移动
            timer->start(speed);
            sDisplay = " ";             // 清除提示文字
            blsover = false;            // 重置结束状态
            // 重新播放背景音乐
            if (m_bgmPlayer) {
                m_bgmPlayer->play();
            }
            update();
            return;
        } else if (m_backToMenuBtnRect.contains(clickPos)) {
            // 返回主菜单
            m_inWelcomeScreen = true;
            blsrun = false;
            blsover = false;
            isStart = false;
            timer->stop();
            if (m_bgmPlayer) {
                m_bgmPlayer->stop();
            }
            update();
            return;
        }
        return;
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::StartGame()
{
    m_inWelcomeScreen = false;
    InitSnake();
    isStart = true;
    nDirection = 2;
    timer->start(speed);
    sDisplay = " ";

    if (m_bgmPlayer) {
        m_bgmPlayer->play();
    }
    update();
}

void MainWindow::InitSnake()
{
    blsrun = true;
    blsover = false;
    isStart = false;
    nDirection = 2;
    sDisplay = "游戏开始";
    ScoreLabel = "得分：";
    nScore = 0;

    if (m_isHardMode) {
        speed = 300;
    } else {
        speed = 500;
    }

    vSnakeRect.clear();
    vSnakeRect.resize(5);
    for (int i = 0; i < vSnakeRect.size(); i++) {
        QRect rect(240, 240 + 10 * i, 10, 10);
        vSnakeRect[vSnakeRect.size() - 1 - i] = rect;
    }
    SnakeHead = vSnakeRect.first();

    if (m_isHardMode) {
        GenerateObstacles();
    } else {
        m_obstacles.clear();
    }

    food = CreatRect();
    timer->stop();
}

QRect MainWindow::CreatRect()
{
    QRect newFood;
    bool valid;
    int maxAttempts = 1000;
    do {
        valid = true;
        int x = QRandomGenerator::global()->bounded(0, 46);  // 0~45
        int y = QRandomGenerator::global()->bounded(0, 46);
        newFood = QRect(20 + x * 10, 20 + y * 10, 10, 10);

        for (const QRect &seg : vSnakeRect) {
            if (newFood == seg) {
                valid = false;
                break;
            }
        }

        if (valid && m_isHardMode) {
            for (const QRect &obs : m_obstacles) {
                if (newFood == obs) {
                    valid = false;
                    break;
                }
            }
        }
        maxAttempts--;
    } while (!valid && maxAttempts > 0);
    return newFood;
}

void MainWindow::GenerateObstacles()
{
    m_obstacles.clear();
    // 障碍物数量 5~10
    int numObstacles = QRandomGenerator::global()->bounded(5, 11);
    int maxAttempts = 3000;  // 提高尝试次数，因为增加了间距限制

    for (int i = 0; i < numObstacles && maxAttempts > 0; ) {
        maxAttempts--;
        // 随机方向：水平或垂直
        bool horizontal = QRandomGenerator::global()->bounded(2) == 0;
        int length = QRandomGenerator::global()->bounded(3, 9);  // 长度 3~8

        // 起始坐标限制在 30~460 之间（不贴着墙壁）
        int startX = 20 + QRandomGenerator::global()->bounded(1, 45) * 10;  // 30,40,...,450
        int startY = 20 + QRandomGenerator::global()->bounded(1, 45) * 10;

        QVector<QRect> tempObs;
        bool valid = true;

        // 生成临时障碍物线段
        for (int j = 0; j < length; ++j) {
            int x = startX + (horizontal ? j * 10 : 0);
            int y = startY + (horizontal ? 0 : j * 10);

            // 边界检查：确保每个单元格都不超出 30~460 范围（即不贴着墙壁）
            if (x < 30 || x > 450 || y < 30 || y > 450) {
                valid = false;
                break;
            }
            tempObs.append(QRect(x, y, 10, 10));
        }

        if (!valid) continue;

        // 1. 检查与蛇身是否重叠
        for (const QRect &seg : vSnakeRect) {
            for (const QRect &obsCell : tempObs) {
                if (seg == obsCell) {
                    valid = false;
                    break;
                }
            }
            if (!valid) break;
        }
        if (!valid) continue;

        // 2. 检查与已有障碍物是否重叠
        for (const QRect &existingObs : m_obstacles) {
            for (const QRect &obsCell : tempObs) {
                if (existingObs == obsCell) {
                    valid = false;
                    break;
                }
            }
            if (!valid) break;
        }
        if (!valid) continue;

        // 3. 检查是否与已有障碍物间隔过近（至少间隔两个空方块）
        //    即 max(|dx|,|dy|) / 10 >= 3  等价于 在格子坐标上差值的绝对值 >= 3
        bool tooClose = false;
        for (const QRect &existingObs : m_obstacles) {
            for (const QRect &obsCell : tempObs) {
                int dx = std::abs(obsCell.x() - existingObs.x()) / 10;
                int dy = std::abs(obsCell.y() - existingObs.y()) / 10;
                // 如果切比雪夫距离小于 3，则认为太近
                if (std::max(dx, dy) < 3) {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose) break;
        }
        if (tooClose) continue;

        // 通过所有检查，添加此障碍物
        m_obstacles.append(tempObs);
        ++i;
    }

    // 可选的日志：如果生成的障碍物数量不足，不影响游戏进行
}

void MainWindow::onGameUpdate()
{
    sDisplay = " ";
    SnakeHead = vSnakeRect.first();

    IsEat();
    IsHit();

    for (int j = 0; j < vSnakeRect.size() - 1; j++) {
        vSnakeRect[vSnakeRect.size() - 1 - j] = vSnakeRect[vSnakeRect.size() - 2 - j];
    }

    switch (nDirection) {
    case 1:
        SnakeHead.setTop(SnakeHead.top() - 10);
        SnakeHead.setBottom(SnakeHead.bottom() - 10);
        break;
    case 2:
        SnakeHead.setTop(SnakeHead.top() + 10);
        SnakeHead.setBottom(SnakeHead.bottom() + 10);
        break;
    case 3:
        SnakeHead.setLeft(SnakeHead.left() - 10);
        SnakeHead.setRight(SnakeHead.right() - 10);
        break;
    case 4:
        SnakeHead.setLeft(SnakeHead.left() + 10);
        SnakeHead.setRight(SnakeHead.right() + 10);
        break;
    default:
        break;
    }
    vSnakeRect[0] = SnakeHead;

    // 边界碰撞检测
    if (SnakeHead.left() < 20 || SnakeHead.right() > 480 ||
        SnakeHead.top() < 20 || SnakeHead.bottom() > 480) {
        sDisplay = "真是一条蠢蛇！";
        blsover = true;
        if (m_bgmPlayer) m_bgmPlayer->stop();
        if (m_gameoverPlayer) m_gameoverPlayer->play();
    }

    // 障碍物碰撞（困难模式）
    if (!blsover && m_isHardMode) {
        for (const QRect &obs : m_obstacles) {
            if (SnakeHead == obs) {
                sDisplay = "撞到障碍物了！";
                blsover = true;

                if (m_bgmPlayer) m_bgmPlayer->stop();
                if (m_gameoverPlayer) m_gameoverPlayer->play();
                break;
            }
        }
    }

    update();
}

void MainWindow::IsEat()
{
    if (SnakeHead == food) {
        SnakeHead = food;
        vSnakeRect.push_back(vSnakeRect.last());
        food = CreatRect();
        nScore += 10;
        if (speed > 100) {
            speed -= 10;
            timer->stop();
            timer->start(speed);
        }

        if (m_eatPlayer) {
            m_eatPlayer->stop();
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

            if (m_bgmPlayer) m_bgmPlayer->stop();
            if (m_gameoverPlayer) {
                m_gameoverPlayer->stop();
                m_gameoverPlayer->play();
            }
            break;
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 欢迎界面或游戏结束后不响应键盘
    if (m_inWelcomeScreen || blsover)
        return;

    int newDir = 0;
    switch (event->key()) {
    case Qt::Key_Up:    newDir = 1; break;
    case Qt::Key_Down:  newDir = 2; break;
    case Qt::Key_Left:  newDir = 3; break;
    case Qt::Key_Right: newDir = 4; break;
    default: return;
    }

    // 游戏进行中：允许改变方向
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