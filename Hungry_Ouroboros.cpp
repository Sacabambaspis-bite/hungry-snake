#include "Hungry_Ouroboros.h"
#include "ui_Hungry_Ouroboros.h"
#include <QRandomGenerator>
#include <QFontMetrics>
#include <QSettings>
#include <QDebug>

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
    , m_normalHighScore(0)
    , m_hardHighScore(0)
    , nDirection(2)
    , m_eatPlayer(nullptr)
    , m_gameoverPlayer(nullptr)
    , m_bgmPlayer(nullptr)
    , m_foodMoveEnabled(false)
    , m_foodMoveTimer(nullptr)
    , m_foodMoveInterval(600)
    , m_animProgress(0.0f)
    , m_isMoving(false)
    , m_animationTimer(nullptr)
    , m_flashPhase(0.0)
    , m_foodMoving(false)
    , m_foodAnimProgress(0.0f)
{
    ui->setupUi(this);
    this->setGeometry(QRect(500, 250, 500, 520));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onGameUpdate);

    // ========== 初始化音效 ==========
    m_eatPlayer = new QMediaPlayer(this);
    auto *eatAudioOut = new QAudioOutput(m_eatPlayer);
    m_eatPlayer->setAudioOutput(eatAudioOut);
    eatAudioOut->setVolume(0.5f);
    m_eatPlayer->setSource(QUrl("qrc:/sounds/btn15.mp3"));

    m_gameoverPlayer = new QMediaPlayer(this);
    auto *gameoverAudioOut = new QAudioOutput(m_gameoverPlayer);
    m_gameoverPlayer->setAudioOutput(gameoverAudioOut);
    gameoverAudioOut->setVolume(0.5f);
    m_gameoverPlayer->setSource(QUrl("qrc:/sounds/btn09.mp3"));

    m_bgmPlayer = new QMediaPlayer(this);
    auto *bgmAudioOut = new QAudioOutput(m_bgmPlayer);
    m_bgmPlayer->setAudioOutput(bgmAudioOut);
    bgmAudioOut->setVolume(0.3f);
    m_bgmPlayer->setSource(QUrl("qrc:/sounds/MusMus-BGM-186.mp3"));
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);

    // 创建动画计时器（每 16ms 一帧，约 60fps）
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &MainWindow::updateAnimation);
    m_animationTimer->start(16);

    // 读取保存的最高分
    QSettings settings("MyCompany", "HungryOuroboros");
    m_normalHighScore = settings.value("NormalHighScore", 0).toInt();
    m_hardHighScore = settings.value("HardHighScore", 0).toInt();
}

MainWindow::~MainWindow()
{
    if (m_foodMoveTimer) {
        m_foodMoveTimer->stop();
        delete m_foodMoveTimer;
    }
    if (m_animationTimer) {
        m_animationTimer->stop();
        delete m_animationTimer;
    }
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


    // 网格线（仅普通模式显示）
    if (!m_isHardMode) {
        painter.setPen(Qt::darkGray);
        for (int i = 2; i <= 47; i++) {
            painter.drawLine(20, i * 10, 480, i * 10);
            painter.drawLine(i * 10, 20, i * 10, 480);
        }
    }

    // 障碍物（困难模式）
    if (m_isHardMode) {
        painter.setPen(Qt::darkYellow);  // 边框颜色
        for (const QRect &obs : m_obstacles) {
            // 渐变填充：左上角亮棕，右下角色暗棕
            QLinearGradient gradient(obs.topLeft(), obs.bottomRight());
            gradient.setColorAt(0, QColor(180, 100, 50));
            gradient.setColorAt(1, QColor(100, 60, 30));
            painter.setBrush(gradient);
            painter.drawRoundedRect(obs, 3, 3);
        }
    }

    // 蛇和食物
    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::white);
    // 绘制蛇身（白色渐变）
    painter.setPen(Qt::lightGray);
    for (int i = 1; i < vSnakeRect.size(); ++i) {
        QPointF pos = getAnimatedPosition(i);
        QRect rect(pos.x(), pos.y(), 10, 10);
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, QColor(255, 255, 255));
        gradient.setColorAt(1, QColor(200, 200, 200));
        painter.setBrush(gradient);
        painter.drawRoundedRect(rect, 2, 2);
        painter.setBrush(QColor(255, 255, 255, 100));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect.x(), rect.y(), 4, 3, 1, 1);
    }

    // 绘制蛇头（浅灰色渐变）
    if (!vSnakeRect.isEmpty()) {
        QPointF headPos = getAnimatedPosition(0);
        QRect headRect(headPos.x(), headPos.y(), 10, 10);
        QLinearGradient headGrad(headRect.topLeft(), headRect.bottomRight());
        headGrad.setColorAt(0, QColor(210, 210, 210));
        headGrad.setColorAt(1, QColor(150, 150, 150));
        painter.setBrush(headGrad);
        painter.drawRoundedRect(headRect, 2, 2);
        painter.setBrush(Qt::darkGray);
        painter.drawEllipse(headRect.x() + 2, headRect.y() + 2, 3, 3);
        painter.drawEllipse(headRect.x() + 6, headRect.y() + 2, 3, 3);
    }

    // 食物绘制
    // 计算当前要绘制的位置（插值或逻辑位置）
    QPointF drawPos;
    if (m_foodMoving) {
        float t = m_foodAnimProgress;
        drawPos = m_foodPrevPos + (m_foodCurrPos - m_foodPrevPos) * t;
    } else {
        drawPos = QPointF(food.x(), food.y());
    }

    // 食物闪烁效果（原有的透明度计算）
    painter.save();
    double opacity = 0.7 + 0.3 * (1.0 + sin(m_flashPhase)) / 2.0;
    painter.setOpacity(opacity);
    painter.drawPixmap(QRect(drawPos.x(), drawPos.y(), 10, 10), QPixmap(":/myImages/fd.png"));
    painter.restore();

    // 暂停文字
    if (m_paused && !blsover && !m_inWelcomeScreen) {
        QFont pauseFont("Arial", 24, QFont::Bold);
        painter.setFont(pauseFont);
        painter.setPen(Qt::white);
        painter.setBrush(Qt::black);
        QString pauseText = "暂停";
        QFontMetrics fm(pauseFont);
        int textWidth = fm.horizontalAdvance(pauseText);
        QRect gameArea(20, 20, 460, 460);
        int x = gameArea.center().x() - textWidth / 2;
        int y = gameArea.center().y() + 15;
        // 增加半透明背景框，使文字更清晰
        painter.fillRect(x - 10, y - fm.height() - 5, textWidth + 20, fm.height() + 10, QColor(0,0,0,180));
        painter.drawText(x, y, pauseText);
    }

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
        QRect gameArea(20, 20, 460, 460);
        int x = gameArea.center().x() - textWidth / 2;
        int y = gameArea.top() + 80;
        painter.drawText(x, y, sDisplay);
    }

    // ================= 游戏结束菜单 =================
    if (blsover) {
        // 半透明遮罩
        painter.fillRect(rect(), QColor(0, 0, 0, 150));

        // 得分文字
        QFont scoreFont("Arial", 14);
        painter.setFont(scoreFont);
        painter.setPen(Qt::yellow);
        QString currentScoreText = QString("当前得分：%1").arg(nScore);
        QString highScoreText = QString("最高得分：%1").arg(m_isHardMode ? m_hardHighScore : m_normalHighScore);
        QFontMetrics fmScore(scoreFont);
        int currentW = fmScore.horizontalAdvance(currentScoreText);
        int highW = fmScore.horizontalAdvance(highScoreText);
        painter.drawText((width() - currentW) / 2, 180, currentScoreText);
        painter.drawText((width() - highW) / 2, 210, highScoreText);

        // 游戏结束大字
        QFont overFont("Arial", 20, QFont::Bold);
        painter.setFont(overFont);
        painter.setPen(Qt::white);
        QString overText = "游戏结束";
        QFontMetrics fmOver(overFont);
        int overTextWidth = fmOver.horizontalAdvance(overText);
        int overTextX = (width() - overTextWidth) / 2;
        painter.drawText(overTextX, 260, overText);

        // 按钮尺寸和位置
        int menuBtnWidth = 160;
        int menuBtnHeight = 45;
        int menuBtnY1 = 320;
        int menuBtnY2 = 380;

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
            // 再来一局：直接调用 StartGame 重置一切
            StartGame();
            update();
            return;
        } else if (m_backToMenuBtnRect.contains(clickPos)) {
            // 返回主菜单：清理食物移动计时器
            if (m_foodMoveTimer) {
                m_foodMoveTimer->stop();
                delete m_foodMoveTimer;
                m_foodMoveTimer = nullptr;
            }
            if (m_animationTimer) m_animationTimer->stop();
            m_inWelcomeScreen = true;
            blsrun = false;
            blsover = false;
            isStart = false;
            timer->stop();
            if (m_bgmPlayer) m_bgmPlayer->stop();
            update();
            return;
        }
        return;
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::StartGame()
{
    m_foodMoving = false;
    m_inWelcomeScreen = false;
    m_paused = false;
    InitSnake();
    isStart = true;
    nDirection = 2;
    timer->start(speed);
    sDisplay = " ";
    m_isMoving = false;
    m_animProgress = 0.0f;

    // 重置食物移动速度
    if (m_isHardMode) {
        m_foodMoveInterval = 400;
    } else {
        m_foodMoveInterval = 600;
    }

    // 创建食物移动计时器（两种模式都启用移动）
    if (m_foodMoveTimer) {
        m_foodMoveTimer->stop();
        delete m_foodMoveTimer;
        m_foodMoveTimer = nullptr;
    }
    m_foodMoveEnabled = true;
    m_foodMoveTimer = new QTimer(this);
    connect(m_foodMoveTimer, &QTimer::timeout, this, &MainWindow::moveFood);
    m_foodMoveTimer->start(m_foodMoveInterval);

    if (m_bgmPlayer) m_bgmPlayer->play();
    update();
}

void MainWindow::InitSnake()
{
    blsrun = true;
    blsover = false;
    isStart = false;
    m_paused = false;
    nDirection = 2;
    sDisplay = "游戏开始";
    ScoreLabel = "得分：";
    nScore = 0;

    if (m_isHardMode) speed = 300;
    else speed = 500;

    vSnakeRect.clear();
    vSnakeRect.resize(5);
    for (int i = 0; i < vSnakeRect.size(); i++) {
        QRect rect(240, 240 + 10 * i, 10, 10);
        vSnakeRect[vSnakeRect.size() - 1 - i] = rect;
    }
    SnakeHead = vSnakeRect.first();

    if (m_isHardMode) GenerateObstacles();
    else m_obstacles.clear();

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
        int x = QRandomGenerator::global()->bounded(0, 46);
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

    if (!valid) {
        // 找不到合法位置，游戏胜利
        sDisplay = "晋升成功！";
        blsover = true;
        if (m_foodMoveTimer) m_foodMoveTimer->stop();
        if (timer) timer->stop();
        if (m_bgmPlayer) m_bgmPlayer->stop();
        if (m_gameoverPlayer) m_gameoverPlayer->play();
        update();
        return QRect(0, 0, 0, 0);
    }
    return newFood;
}

void MainWindow::GenerateObstacles()
{
    m_obstacles.clear();
    int numObstacles = QRandomGenerator::global()->bounded(7, 12);
    int maxAttempts = 3000;

    for (int i = 0; i < numObstacles && maxAttempts > 0; ) {
        maxAttempts--;
        bool horizontal = QRandomGenerator::global()->bounded(2) == 0;
        int length = QRandomGenerator::global()->bounded(5, 12);

        int startX = 20 + QRandomGenerator::global()->bounded(1, 45) * 10;
        int startY = 20 + QRandomGenerator::global()->bounded(1, 45) * 10;

        QVector<QRect> tempObs;
        bool valid = true;

        for (int j = 0; j < length; ++j) {
            int x = startX + (horizontal ? j * 10 : 0);
            int y = startY + (horizontal ? 0 : j * 10);
            if (x < 30 || x > 450 || y < 30 || y > 450) {
                valid = false;
                break;
            }
            tempObs.append(QRect(x, y, 10, 10));
        }

        if (!valid) continue;

        // 与蛇身重叠
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

        // 与已有障碍物重叠
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

        // 间隔至少两个方块
        bool tooClose = false;
        for (const QRect &existingObs : m_obstacles) {
            for (const QRect &obsCell : tempObs) {
                int dx = std::abs(obsCell.x() - existingObs.x()) / 10;
                int dy = std::abs(obsCell.y() - existingObs.y()) / 10;
                if (std::max(dx, dy) < 3) {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose) break;
        }
        if (tooClose) continue;

        m_obstacles.append(tempObs);
        ++i;
    }
}

// 统一的吃食物处理函数
void MainWindow::eatFood()
{
    m_foodMoving = false;   //
    vSnakeRect.insert(vSnakeRect.begin(), food);
    nScore += 10;

    // 蛇移动速度加快
    if (speed > 150) {
        speed -= 10;
        timer->setInterval(speed);
    }

    // 食物自身移动速度加快
    if (m_foodMoveEnabled && m_foodMoveTimer) {
        const int MIN_FOOD_INTERVAL = 50;
        m_foodMoveInterval = qMax(MIN_FOOD_INTERVAL, m_foodMoveInterval - 5);
        m_foodMoveTimer->setInterval(m_foodMoveInterval);
    }

    if (m_eatPlayer) {
        m_eatPlayer->stop();
        m_eatPlayer->play();
    }

    // 生成新食物
    food = CreatRect();
}

void MainWindow::onGameUpdate()
{
    if (blsover || !isStart || m_paused) return;

    // 1. 记录移动前的所有蛇节位置（像素坐标）
    m_prevPositions.clear();
    for (const QRect &rect : vSnakeRect) {
        m_prevPositions.append(QPointF(rect.x(), rect.y()));
    }

    // 2. 执行逻辑移动（原有代码，但需要微调）
    sDisplay = " ";
    QRect newHead = vSnakeRect.first();
    switch (nDirection) {
    case 1: newHead.moveTop(newHead.top() - 10); break;
    case 2: newHead.moveTop(newHead.top() + 10); break;
    case 3: newHead.moveLeft(newHead.left() - 10); break;
    case 4: newHead.moveLeft(newHead.left() + 10); break;
    default: break;
    }

    bool ate = (newHead == food);

    if (ate) {
        eatFood();  // 使用之前统一的吃食物函数
        if (blsover) {
            update();
            return;
        }
    } else {
        vSnakeRect.insert(vSnakeRect.begin(), newHead);
        vSnakeRect.pop_back();
    }

    // 3. 记录移动后的所有蛇节位置
    m_currPositions.clear();
    for (const QRect &rect : vSnakeRect) {
        m_currPositions.append(QPointF(rect.x(), rect.y()));
    }

    // 4. 启动动画插值
    m_lastMoveTime = QTime::currentTime();
    m_animProgress = 0.0f;
    m_isMoving = true;

    // 5. 碰撞检测（使用移动后的真实位置）
    QRect &head = vSnakeRect.first();
    if (head.left() < 20 || head.right() > 480 || head.top() < 20 || head.bottom() > 480) {
        sDisplay = "真是一条蠢蛇！";
        updateHighScore();
        blsover = true;
        if (m_foodMoveTimer) m_foodMoveTimer->stop();
        if (m_bgmPlayer) m_bgmPlayer->stop();
        if (m_gameoverPlayer) m_gameoverPlayer->play();
        m_isMoving = false;  // 停止动画
        update();
        return;
    }

    for (int i = 1; i < vSnakeRect.size(); ++i) {
        if (head == vSnakeRect[i]) {
            sDisplay = "不可以吃掉自己哦";
            updateHighScore();
            blsover = true;
            if (m_foodMoveTimer) m_foodMoveTimer->stop();
            if (m_bgmPlayer) m_bgmPlayer->stop();
            if (m_gameoverPlayer) m_gameoverPlayer->play();
            m_isMoving = false;
            update();
            return;
        }
    }

    if (m_isHardMode) {
        for (const QRect &obs : m_obstacles) {
            if (head == obs) {
                sDisplay = "撞到障碍物了！";
                updateHighScore();
                blsover = true;
                if (m_foodMoveTimer) m_foodMoveTimer->stop();
                if (m_bgmPlayer) m_bgmPlayer->stop();
                if (m_gameoverPlayer) m_gameoverPlayer->play();
                m_isMoving = false;
                update();
                return;
            }
        }
    }

    update();  // 触发重绘，开始动画
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 暂停处理（游戏进行中，未结束，未在欢迎界面）
    if (!m_inWelcomeScreen && !blsover && isStart) {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_P) {
            m_paused = !m_paused;
            if (m_paused) {
                if (timer->isActive()) timer->stop();
                if (m_foodMoveTimer && m_foodMoveTimer->isActive()) m_foodMoveTimer->stop();
                if (m_animationTimer) m_animationTimer->stop();
            } else {
                if (!timer->isActive()) timer->start(speed);
                if (m_foodMoveEnabled && m_foodMoveTimer && !m_foodMoveTimer->isActive())
                    m_foodMoveTimer->start(m_foodMoveInterval);
                if (m_animationTimer) m_animationTimer->start(16);
            }
            update();
            return;
        }
    }

    // 欢迎界面、游戏结束、暂停时不允许改变方向
    if (m_inWelcomeScreen || blsover || m_paused)
        return;

    int newDir = 0;
    switch (event->key()) {
    case Qt::Key_Up:    newDir = 1; break;
    case Qt::Key_Down:  newDir = 2; break;
    case Qt::Key_Left:  newDir = 3; break;
    case Qt::Key_Right: newDir = 4; break;
    default: return;
    }

    if (isStart) {
        if ((nDirection == 1 && newDir == 2) ||
            (nDirection == 2 && newDir == 1) ||
            (nDirection == 3 && newDir == 4) ||
            (nDirection == 4 && newDir == 3)) {
            return;
        }
        nDirection = newDir;
    }
}

void MainWindow::updateHighScore()
{
    QSettings settings("MyCompany", "HungryOuroboros");
    if (m_isHardMode) {
        if (nScore > m_hardHighScore) {
            m_hardHighScore = nScore;
            settings.setValue("HardHighScore", m_hardHighScore);
        }
    } else {
        if (nScore > m_normalHighScore) {
            m_normalHighScore = nScore;
            settings.setValue("NormalHighScore", m_normalHighScore);
        }
    }
}

void MainWindow::moveFood()
{
    if (!isStart || blsover || !m_foodMoveEnabled) return;

    QRect oldFood = food;
    QVector<QRect> candidates;
    QRect up(oldFood.left(), oldFood.top() - 10, 10, 10);
    QRect down(oldFood.left(), oldFood.top() + 10, 10, 10);
    QRect left(oldFood.left() - 10, oldFood.top(), 10, 10);
    QRect right(oldFood.left() + 10, oldFood.top(), 10, 10);

    auto isLegalPosition = [&](const QRect &rect) -> bool {
        if (rect.left() < 20 || rect.right() > 480 ||
            rect.top() < 20 || rect.bottom() > 480)
            return false;
        for (const QRect &seg : vSnakeRect) {
            if (rect == seg) return false;
        }
        if (m_isHardMode) {
            for (const QRect &obs : m_obstacles) {
                if (rect == obs) return false;
            }
        }
        return true;
    };

    if (isLegalPosition(up))    candidates.append(up);
    if (isLegalPosition(down))  candidates.append(down);
    if (isLegalPosition(left))  candidates.append(left);
    if (isLegalPosition(right)) candidates.append(right);

    if (!candidates.isEmpty()) {
        int idx = QRandomGenerator::global()->bounded(candidates.size());
        QRect newFoodRect = candidates[idx];

        // 记录动画数据
        m_foodPrevPos = QPointF(oldFood.x(), oldFood.y());
        m_foodCurrPos = QPointF(newFoodRect.x(), newFoodRect.y());
        m_foodMoving = true;
        m_foodAnimProgress = 0.0f;
        m_foodMoveStartTime = QTime::currentTime();

        // 更新逻辑位置（碰撞检测使用此位置）
        food = newFoodRect;
        update();

        // 如果移动后蛇头恰好在食物上，立即吃掉（保留原有逻辑）
        if (vSnakeRect.first() == food) {
            eatFood();
        }
    }
}

QPointF MainWindow::getAnimatedPosition(int index) const
{
    if (!m_isMoving || m_prevPositions.isEmpty() || m_currPositions.isEmpty() || index >= m_prevPositions.size()) {
        if (index < vSnakeRect.size())
            return QPointF(vSnakeRect[index].x(), vSnakeRect[index].y());
        return QPointF();
    }
    float t = qMin(1.0f, m_animProgress);
    QPointF prev = m_prevPositions[index];
    QPointF curr = m_currPositions[index];
    return prev + (curr - prev) * t;
}

void MainWindow::updateAnimation()
{
    // 蛇移动动画更新（如果已实现）
    if (m_isMoving) {
        int elapsed = m_lastMoveTime.msecsTo(QTime::currentTime());
        float totalTime = float(speed);
        m_animProgress = elapsed / totalTime;
        if (m_animProgress >= 1.0f) {
            m_animProgress = 1.0f;
            m_isMoving = false;
        }
        update();
    }

    // 食物移动动画更新
    if (m_foodMoving) {
        int elapsed = m_foodMoveStartTime.msecsTo(QTime::currentTime());
        float totalTime = float(m_foodMoveInterval);
        m_foodAnimProgress = qMin(1.0f, elapsed / totalTime);
        if (m_foodAnimProgress >= 1.0f) {
            m_foodMoving = false;
        }
        update();
    }

    // 更新食物闪烁相位（原有代码）
    m_flashPhase += 0.15;
    if (m_flashPhase > 2 * M_PI)
        m_flashPhase -= 2 * M_PI;
}