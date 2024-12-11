#include "oneko.h"
#include <QApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QCursor>
#include <QMenu>
#include <QRandomGenerator>
#include <QDebug>
#include <QDir>

CatWidget::CatWidget(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    resize(32, 32);

    // Initialize sprite sets (based off oneko.js)
    spriteSets.sets["idle"] = {{-3, -3}};
    spriteSets.sets["alert"] = {{-7, -3}};
    spriteSets.sets["scratchSelf"] = {{-5, 0}, {-6, 0}, {-7, 0}};
    spriteSets.sets["scratchWallN"] = {{0, 0}, {0, -1}};
    spriteSets.sets["scratchWallS"] = {{-7, -1}, {-6, -2}};
    spriteSets.sets["scratchWallE"] = {{-2, -2}, {-2, -3}};
    spriteSets.sets["scratchWallW"] = {{-4, 0}, {-4, -1}};
    spriteSets.sets["tired"] = {{-3, -2}};
    spriteSets.sets["sleeping"] = {{-2, 0}, {-2, -1}};
    spriteSets.sets["N"] = {{-1, -2}, {-1, -3}};
    spriteSets.sets["NE"] = {{0, -2}, {0, -3}};
    spriteSets.sets["E"] = {{-3, 0}, {-3, -1}};
    spriteSets.sets["SE"] = {{-5, -1}, {-5, -2}};
    spriteSets.sets["S"] = {{-6, -3}, {-7, -2}};
    spriteSets.sets["SW"] = {{-5, -3}, {-6, -1}};
    spriteSets.sets["W"] = {{-4, -2}, {-4, -3}};
    spriteSets.sets["NW"] = {{-1, 0}, {-1, -1}};

    label = new QLabel(this);
    label->setScaledContents(true);
    label->setFixedSize(32, 32);

    QString spritePath = QDir::currentPath() + "/oneko.gif";
    spriteSheet = QPixmap(spritePath);
    if (spriteSheet.isNull())
    {
        qWarning() << "Could not load oneko.gif from current directory.";
        QTimer::singleShot(0, qApp, &QApplication::quit);
        return;
    }

    nekoPosX = 100;
    nekoPosY = 100;
    move(nekoPosX - 16, nekoPosY - 16);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CatWidget::updateFrame);
    timer->start(100);
}

void CatWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    QAction *closeAction = menu.addAction("Close oneko");
    connect(closeAction, &QAction::triggered, qApp, &QApplication::quit);
    menu.exec(event->globalPos());
}

void CatWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        QContextMenuEvent ce(QContextMenuEvent::Mouse, event->pos(), event->globalPos());
        contextMenuEvent(&ce);
    }
}

void CatWidget::updateFrame()
{
    frame();
}

void CatWidget::frame()
{
    frameCount++;
    QPoint globalMouse = QCursor::pos();
    mousePosX = globalMouse.x();
    mousePosY = globalMouse.y();

    qreal diffX = nekoPosX - mousePosX;
    qreal diffY = nekoPosY - mousePosY;
    qreal distance = std::sqrt(diffX * diffX + diffY * diffY);

    if (distance < nekoSpeed || distance < 48)
    {
        idle();
        return;
    }

    if (!idleAnimation.isEmpty())
    {
        idleAnimation.clear();
        idleAnimationFrame = 0;
    }

    if (idleTime > 1)
    {
        setSprite("alert", 0);
        idleTime = qMin(idleTime, 7);
        idleTime -= 1;
        return;
    }

    qreal dx = diffX / distance;
    qreal dy = diffY / distance;

    QString verticalDir;
    if (dy > 0.5)
        verticalDir = "N";
    else if (dy < -0.5)
        verticalDir = "S";

    QString horizontalDir;
    if (dx > 0.5)
        horizontalDir = "W";
    else if (dx < -0.5)
        horizontalDir = "E";

    QString direction = verticalDir + horizontalDir;
    setSprite(direction.isEmpty() ? "idle" : direction, frameCount);

    nekoPosX -= dx * nekoSpeed;
    nekoPosY -= dy * nekoSpeed;

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen)
    {
        QRect screenRect = screen->geometry();
        nekoPosX = qMin(qMax((qreal)16, nekoPosX), (qreal)screenRect.right() - 16);
        nekoPosY = qMin(qMax((qreal)16, nekoPosY), (qreal)screenRect.bottom() - 16);
    }

    updateWindowPosition();
}

void CatWidget::idle()
{
    idleTime += 1;

    if (idleTime > 10 && QRandomGenerator::global()->bounded(200) == 0 && idleAnimation.isEmpty())
    {
        QStringList available{"sleeping", "scratchSelf"};

        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen)
        {
            QRect sr = screen->geometry();
            if (nekoPosX < 32)
                available << "scratchWallW";
            if (nekoPosY < 32)
                available << "scratchWallN";
            if (nekoPosX > sr.right() - 32)
                available << "scratchWallE";
            if (nekoPosY > sr.bottom() - 32)
                available << "scratchWallS";
        }

        idleAnimation = available.at(QRandomGenerator::global()->bounded(available.size()));
    }

    if (idleAnimation.isEmpty())
    {
        setSprite("idle", 0);
        return;
    }

    if (idleAnimation == "sleeping")
    {
        if (idleAnimationFrame < 8)
        {
            setSprite("tired", 0);
        }
        else
        {
            setSprite("sleeping", idleAnimationFrame / 4);
        }
        if (idleAnimationFrame > 192)
        {
            resetIdleAnimation();
        }
    }
    else if (idleAnimation == "scratchWallN" ||
             idleAnimation == "scratchWallS" ||
             idleAnimation == "scratchWallE" ||
             idleAnimation == "scratchWallW" ||
             idleAnimation == "scratchSelf")
    {
        setSprite(idleAnimation, idleAnimationFrame);
        if (idleAnimationFrame > 9)
        {
            resetIdleAnimation();
        }
    }
    else
    {
        setSprite("idle", 0);
    }

    idleAnimationFrame += 1;
}

void CatWidget::resetIdleAnimation()
{
    idleAnimation.clear();
    idleAnimationFrame = 0;
}

void CatWidget::setSprite(const QString &name, int frameIndex)
{
    if (!spriteSets.sets.contains(name))
    {
        setSprite("idle", frameIndex);
        return;
    }

    const auto &frames = spriteSets.sets[name];
    if (frames.isEmpty())
    {
        setSprite("idle", frameIndex);
        return;
    }

    QPair<int, int> frame = frames.at(frameIndex % frames.size());
    int offsetX = frame.first * 32;
    int offsetY = frame.second * 32;

    int cropX = -offsetX;
    int cropY = -offsetY;

    if (cropX < 0 || cropY < 0 ||
        cropX + 32 > spriteSheet.width() || cropY + 32 > spriteSheet.height())
    {
        qWarning() << "Warning: Attempting to crop outside the sprite sheet.";
        return;
    }

    QPixmap frameImage = spriteSheet.copy(cropX, cropY, 32, 32);
    label->setPixmap(frameImage);
}

void CatWidget::updateWindowPosition()
{
    move((int)(nekoPosX - 16), (int)(nekoPosY - 16));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CatWidget w;
    w.show();
    return app.exec();
}
