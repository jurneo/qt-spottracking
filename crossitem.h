#ifndef CROSSITEM_H
#define CROSSITEM_H

#include <QGraphicsRectItem>

class CrossItem : public QGraphicsRectItem
{
public:
    explicit CrossItem(qreal x, qreal y, QGraphicsItem *parent = 0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);
signals:
    
public slots:
    
};

#endif // CROSSITEM_H
