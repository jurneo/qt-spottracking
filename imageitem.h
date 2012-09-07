#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QGraphicsItem>
#include <boost/smart_ptr.hpp>

class ImageItem : public QGraphicsItem
{
public:
    ImageItem();
    explicit ImageItem(const QImage& image);
    ~ImageItem();

    void updateImage(const QImage* image);
    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    class PrivateImpl;
    boost::scoped_ptr<PrivateImpl> m_impl;
};

#endif // IMAGEITEM_H
