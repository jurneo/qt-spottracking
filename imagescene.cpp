#include "imagescene.h"
#include <iostream>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <crossitem.h>

namespace {
QGraphicsRectItem* m_rectItem = 0;
QRectF m_rectOrigin;
QVector<CrossItem*> m_crossItems;
bool gFlag = false;
}

ImageScene::ImageScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_rectItem = new QGraphicsRectItem();
    addItem(m_rectItem);
    for(int i = 0; i<5; ++i)
    {
        CrossItem* item = new CrossItem(-10,-10);
        addItem(item);
        m_crossItems.push_back(item);
    }
}

void ImageScene::clearScene()
{
    for(int i = 0; i<5; ++i)
    {
        m_crossItems.at(i)->setPos(-10,-10);
    }
}

void ImageScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
    {
        return;
    }
    foreach(CrossItem* item, m_crossItems)
    {
        if (item->isUnderMouse())
        {
            QGraphicsScene::mousePressEvent(mouseEvent);
            gFlag = true;
            std::cout << "is selected\n";
            return;
        }
    }
    QPointF p = mouseEvent->scenePos();
    m_rectOrigin.setRect(p.x(), p.y(), 1, 1);
    m_rectItem->setRect(p.x(), p.y(), 1, 1);
    m_rectItem->setPen(QPen(Qt::blue));

    QGraphicsScene::mousePressEvent(mouseEvent);
}

void ImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (gFlag)
    {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
        return;
    }

    QPointF p = mouseEvent->scenePos();
    p = m_rectItem->mapFromScene(p);
    QRectF r = m_rectOrigin;
    QPointF o = QPointF( r.left(), r.top());
    qreal w = qAbs( p.x()-o.x() );
    qreal h = qAbs( p.y()-o.y() );
    qreal x = qMin(p.x(), o.x());
    qreal y = qMin(p.y(), o.y());
    m_rectItem->setRect(x, y, w, h);

    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void ImageScene::moveItem(int idx, qreal x, qreal y)
{
    m_crossItems.at(idx)->setPos(x,y);
}

void ImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton || gFlag)
    {
        gFlag = false;
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
        return;
    }

    QPointF p = m_rectItem->rect().center();
    CrossItem* item = new CrossItem(p.x(), p.y());
    addItem(item);
    m_crossItems.push_back(item);

    m_rectItem->setRect(0,0,0,0);
    m_rectOrigin.setRect(0,0,0,0);

    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void ImageScene::keyPressEvent(QKeyEvent* event)
{
    QGraphicsScene::keyPressEvent(event);
}
