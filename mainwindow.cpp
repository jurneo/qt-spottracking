#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imagescene.h"
#include <cameradevice.h>
#include <imageitem.h>
//#include "pnmio.h"
//#include "klt.h"
#include <qtimer>

namespace {
int nId = 0;
int nTId = 0;
ImageItem* m_item = 0;
ImageScene* m_scene = 0;
//KLT_TrackingContext tc;
//KLT_FeatureList fl;
//int nFeatures = 1;
//int ncols, nrows;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ImageItem* imageItem = new ImageItem;
    m_item = imageItem;
    m_item->setZValue(-5);
    m_item->setOpacity(0.8);
    ImageScene* scene = new ImageScene;
    m_scene = scene;
    ui->graphicsView->setScene(scene);
    scene->addItem(imageItem);

    CameraDevice* dev = new CameraDevice;
    dev->initialize((HWND) dev->winId());
    dev->onStart();
    QSize s = dev->imageSize();
    scene->setSceneRect(0,0,s.width(), s.height());

    nId = startTimer(30);
    nTId = startTimer(1000);

//    nFeatures = 1;
//    tc = KLTCreateTrackingContext();
//    tc->mindist = 51;
//    tc->window_width  = 51;
//    tc->window_height = 51;
//    KLTChangeTCPyramid(tc, 51);
//    KLTUpdateTCBorder(tc);
//    fl = KLTCreateFeatureList(nFeatures);
//    ncols = dev->imageSize().width();
//    nrows = dev->imageSize().height();

    QTimer::singleShot(1000, this, SLOT(initialize()));
}

MainWindow::~MainWindow()
{
    CameraDevice* dev = CameraDevice::instance();
    delete dev;
    killTimer(nId);
    killTimer(nTId);
    delete ui;
}

void MainWindow::initialize()
{
    /*
    unsigned char* img  = CameraDevice::instance()->getGreyImage();
    KLTSelectGoodFeatures(tc, img, ncols, nrows, fl);
    if (fl->feature != 0)
    {
        for (int indx = 0 ; indx < fl->nFeatures ; indx++)
        {
            if (fl->feature[indx]->val >= 0)
            {
                float x = fl->feature[indx]->x;
                float y = fl->feature[indx]->y;
                m_scene->moveItem(x,y);
            }
        }
    }
    */
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    if (nId == e->timerId())
    {
        CameraDevice* dev = CameraDevice::instance();
        m_item->updateImage(dev->getFrame());

        // update the result
        m_scene->clearScene();
        std::vector<std::pair<int, int> > blobs = dev->getBlobs();
        for(int i = 0; i<blobs.size(); ++i)
        {
            std::pair<int, int> b = blobs.at(i);
            m_scene->moveItem(i, b.first, b.second);
        }
        m_scene->update();
    }
    if (nTId == e->timerId())
    {

    }
    QObject::timerEvent(e);
}
