#include "imageitem.h"
#include <qpainter>

namespace {
const int IMAGE_WIDTH = 640;
const int IMAGE_HEIGHT = 480;
}

class ImageItem::PrivateImpl
{
public:
    PrivateImpl() : m_image(0)
    {
    }

    const QImage* m_image;
};

ImageItem::ImageItem() : m_impl(new PrivateImpl), QGraphicsItem()
{
}

ImageItem::ImageItem(const QImage& image) :
    m_impl(new PrivateImpl), QGraphicsItem()
{
    m_impl->m_image = &image;
}

ImageItem::~ImageItem()
{
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (m_impl->m_image == 0)
    {
        return;
    }
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawImage(boundingRect(), *m_impl->m_image);
}

QRectF ImageItem::boundingRect() const
{
    if (m_impl->m_image == NULL)
    {
        return QRectF(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
    }
    return m_impl->m_image->rect();
}

void ImageItem::updateImage(const QImage* image)
{
    m_impl->m_image = image;
}
