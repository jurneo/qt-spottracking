#include "crossitem.h"
#include <qtgui>

CrossItem::CrossItem(qreal x, qreal y, QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{
    setRect(x-10, y-10, 20, 20);
    setFlags(QGraphicsItem::ItemIsMovable);
}

void CrossItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    QRectF r(boundingRect());
    qreal x1 = r.center().x();
    qreal y1 = r.center().y();
    QLineF line1(r.left(), y1, r.right(), y1);
    QLineF line2(x1, r.top(), x1, r.bottom());
    QPen p(Qt::red);
    painter->setPen(p);
    painter->drawLine(line1);
    painter->drawLine(line2);
    //QGraphicsRectItem::paint(painter, option, widget);
}
