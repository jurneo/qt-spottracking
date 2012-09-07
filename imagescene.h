#ifndef IMAGESCENE_H
#define IMAGESCENE_H

#include <QGraphicsScene>

class ImageScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ImageScene(QObject *parent = 0);
    void moveItem(int idx, qreal x, qreal y);
    void clearScene();
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void keyPressEvent(QKeyEvent* event );
    
};

#endif // IMAGESCENE_H
