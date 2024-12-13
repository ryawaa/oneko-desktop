#ifndef QTNEKO_H
#define QTNEKO_H

#include <QGraphicsView>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QMap>
#include <QVector>
#include <QPair>

class QGraphicsScene;
class QGraphicsPixmapItem;

struct SpriteOffsets
{
    QMap<QString, QVector<QPair<int, int>>> sets;
};

class CatWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CatWidget(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateFrame();

private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *spriteItem;
    QPixmap spriteSheet;
    SpriteOffsets spriteSets;

    qreal nekoPosX = 100;
    qreal nekoPosY = 100;
    qreal mousePosX = 0;
    qreal mousePosY = 0;

    qreal nekoSpeed = 10;
    int frameCount = 0;
    int idleTime = 0;
    QString idleAnimation;
    int idleAnimationFrame = 0;

    QTimer *timer = nullptr;

    void frame();
    void idle();
    void resetIdleAnimation();
    void setSprite(const QString &name, int frameIndex);
    void updateWindowPosition();
};

#endif // QTNEKO_H
