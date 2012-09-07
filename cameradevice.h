#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <windows.h>
#include <qwidget>
#include <qimage>
#include <boost/smart_ptr.hpp>

class CameraDeviceImpl;

/*! This class provides a wrapper to the underlying camera API details.
    It provides a shared resource of QImage object that is displayed by other widgets
    This is a singleton class, such that one instance for the whole app's life.
 */
class CameraDevice : public QWidget
{
public:
    CameraDevice();
    ~CameraDevice(); // final

    static CameraDevice* instance();

    void initialize(HWND id);
    void setDisplay(int posx, int posy, int width, int height);

    void brightnessChanged(int curPos);
    void contrastChanged(int curPos);
    void satureateChanged(int curPos);
    void hueChanged(int curPos);

    void onStart();
    void onStop();
    void onGrab();

    const QImage* getFrame() const;
    unsigned char* getGreyImage();

    QSize imageSize();
    std::vector<std::pair<int, int> > getBlobs();

protected:
    void timerEvent(QTimerEvent* e);

protected:
    static CameraDevice* m_device;
    boost::scoped_ptr<CameraDeviceImpl> m_impl;
};

#endif // CAMERADEVICE_H
