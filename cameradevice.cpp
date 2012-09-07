#include "cameradevice.h"
#include <cmath>
#include <algorithm>
#include <cv/cv.h>
#include <highgui/highgui.h>
#include <windows.h>
#include <signal.h>
#include <iostream>
#include "wincapture.h"
#include <qtgui>

namespace {

UINT m_nOn_Off = 0;
BOOL m_boFreeze = false;
BOOL m_boCapture = false;
BOOL m_boOpen = false;
BOOL m_chkMirror = false;
BOOL m_chkFlip = false;
LONG lRet, ntotalcard = 0;
LONG bright, contrast, satu, hue;
LONG lMax, lMin, lDefault;
const int SHIFTX = 10;
const int SHIFTY = 10;
bool initialized = false;
bool gFlag = true;
const float epsilon = 1e-6;
const int CAPTURE_SPEED = 20;
const int FRAME_WAIT = 5;
IplImage* g_bgndImg = 0;
IplImage* g_fgndImg = 0;
IplImage* g_dispImg = 0;
std::vector<std::pair<int, int> > g_result;
int g_counter = 0;
}

CameraDevice* CameraDevice::m_device = 0;

class CameraDeviceImpl
{
public:
    CameraDeviceImpl() : m_image(0), m_timerId(0)
    {
    }

    QImage* m_image;
    unsigned char* m_grey;
    int m_timerId;
};

CameraDevice::CameraDevice() : QWidget()
{
    m_impl.reset(new CameraDeviceImpl);
    m_device = this;
    hide();
}

CameraDevice* CameraDevice::instance()
{
    return m_device;
}

CameraDevice::~CameraDevice()
{
    if (m_boOpen)
    {
    }

    killTimer(m_impl->m_timerId);
    m_impl->m_timerId = 0;
    hide();
    cvReleaseImage( &g_dispImg );
    cvReleaseImage( &g_bgndImg );
    mswin32::CleanUp();
    gFlag = false;
    m_device = 0;
}

void CameraDevice::brightnessChanged(int curPos)
{
}

void CameraDevice::contrastChanged(int curPos)
{
}

void CameraDevice::satureateChanged(int curPos)
{
}

void CameraDevice::hueChanged(int curPos)
{
}

void handle_segmentation_fault(int code)
{
    QMessageBox box(QMessageBox::Critical, "camera test", "A segmentation fault is encountered.\n\n" \
                    "Please check the log file to find out any details.\t\n");
    box.exec();
    //exit(-1);
    gFlag = false;
}

void initializeCvImages()
{
    const int h = mswin32::GetFrameHeight();
    const int w = mswin32::GetFrameWidth();
    CvSize imgSize = cvSize(w, h);
    g_fgndImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
    g_bgndImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 3);
    g_dispImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 3);
}

void findLaserSpot()
{
    g_result.clear();
    if (g_counter<FRAME_WAIT)
    {
        //cvCvtColor(g_dispImg, g_bgndImg, CV_RGB2GRAY);
        //cvCvtColor(g_dispImg, g_bgndImg, CV_BGR2HSV);
        ++g_counter;
        return;
    }
    //cvCvtColor(g_dispImg, g_fgndImg, CV_RGB2GRAY);
    cvCvtColor(g_dispImg, g_bgndImg, CV_BGR2HSV);
    //cvSub(g_fgndImg, g_bgndImg, g_fgndImg);

    double param1 = 200;
    CvMoments moments;
    //cvSmooth(g_fgndImg, g_fgndImg, CV_MEDIAN, 3);
    cvInRangeS(g_bgndImg, cvScalar(170, 150, 50, 0), cvScalar(190, 240, 255, 0), g_fgndImg);
    //cvThreshold( g_fgndImg, g_fgndImg, param1, 255, CV_THRESH_BINARY);
    //cvDilate(g_fgndImg,g_fgndImg,CV_SHAPE_RECT,3);
    IplConvKernel *kernel;
    kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);
    cvDilate(g_fgndImg,g_fgndImg,kernel,1);
//    cvErode(g_fgndImg,g_fgndImg,kernel,1);
    cvSmooth( g_fgndImg, g_fgndImg, CV_MEDIAN);
    cvMoments(g_fgndImg, &moments, 1);
    //cvGetCentralMoment(&moments, 0, 0);
    if(std::fabs(moments.m00)<=epsilon)
    {
        //g_counter = 3;
        return;
    }

    double centroidX = 0.0, centroidY = 0.0;
    const int h = mswin32::GetFrameHeight();
    centroidY = h-moments.m01/moments.m00 + SHIFTY;
    centroidX = moments.m10/moments.m00 + SHIFTX;
    g_result.push_back(std::pair<int, int>(centroidX, centroidY));
}

void CameraDevice::initialize(HWND winid)
{
    try
    {
        signal(SIGSEGV, handle_segmentation_fault);
        mswin32::Initialize(winid);
        gFlag = true;

        const int h = mswin32::GetFrameHeight();
        const int w = mswin32::GetFrameWidth();
        m_impl->m_image = new QImage(w, h, QImage::Format_RGB888);
        m_impl->m_grey = (unsigned char *) malloc(w * h * sizeof(char));
        int cntr = 0;
        for(int j = 0; j<h; ++j)
        {
            for(int i = 0; i<w; ++i)
            {
                m_impl->m_image->setPixel(i, j, qRgb(128,128,128));
                m_impl->m_grey[cntr] = 0;
                ++cntr;
            }
        }

        initializeCvImages();
    }
    catch(...)
    {
        std::cout << "crash\n";
        return;
    }

    if (!gFlag)  return;

    m_impl->m_timerId = startTimer(CAPTURE_SPEED);
}

void CameraDevice::setDisplay(int posx, int posy, int width, int height)
{
    if (m_boOpen)
    {
        if (initialized)
        {
        }
    }
}

void CameraDevice::onStart()
{
    if (m_boOpen)
    {
        if (!initialized)
        {
            initialized = true;
        }

        if (m_boFreeze)
        {
            m_boFreeze = FALSE;
        }
    }
}

void CameraDevice::onStop()
{
    if (m_boOpen)
    {
        if (!initialized) return;
        if (!m_boFreeze)
        {
            m_boFreeze = TRUE;
        }
    }
}

void CameraDevice::onGrab()
{
    if (m_boOpen)
    {
        if (!initialized) return;

    }
}

const QImage* CameraDevice::getFrame() const
{
    return m_impl->m_image;
}

unsigned char* CameraDevice::getGreyImage()
{
    return m_impl->m_grey;
}

QSize CameraDevice::imageSize()
{
    return QSize(mswin32::GetFrameWidth(), mswin32::GetFrameHeight());
}

void CameraDevice::timerEvent(QTimerEvent* e)
{
    if (m_impl->m_timerId == e->timerId())
    {
        int k = 0;
        const int h = mswin32::GetFrameHeight();
        const int w = mswin32::GetFrameWidth();
        int rs = 0, gs = 1, bs = 2;
        unsigned char* p = mswin32::GetFrame();
        if (p == 0)  return;
        memcpy(g_dispImg->imageData, p, 3*h*w*sizeof(char));

        findLaserSpot();

        char* rawimg = g_fgndImg->imageData;
        int c = 0;
        for(int j = h-1; j>=0; --j)
        {
            for(int i = 0; i<w; ++i)
            {
                QRgb rgb = qRgb(rawimg[c], rawimg[c], rawimg[c]);
                //QRgb rgb = qRgb(rawimg[k+bs], rawimg[k+gs], rawimg[k+rs]);
                m_impl->m_image->setPixel(i, j, rgb);
                k += 3;
                ++c;
            }
        }
    }
}

std::vector<std::pair<int, int> > CameraDevice::getBlobs()
{
    return g_result;
}
