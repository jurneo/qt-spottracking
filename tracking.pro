#-------------------------------------------------
#
# Project created by QtCreator 2012-05-25T15:06:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tracking
TEMPLATE = app
CONFIG += console
WORKSPACE = $$quote($$(SISWORKSPACE))

SOURCES += main.cpp\
        mainwindow.cpp \
    wincapture.cpp \
    cameradevice.cpp \
    imageitem.cpp \
    imagescene.cpp \
    crossitem.cpp \
    lasersportfinder.cpp

HEADERS  += mainwindow.h \
    wincapture.h \
    cameradevice.h \
    imageitem.h \
    imagescene.h \
    crossitem.h \
    lasersportfinder.h

FORMS    += mainwindow.ui

INCLUDEPATH += \
               $$WORKSPACE/libraries/opencv/include $$WORKSPACE/libraries/opencv/include/cv \
               $$WORKSPACE/libraries/opencv/include/highgui $$WORKSPACE/libraries/opencv/include/cxcore

win32:INCLUDEPATH += $$quote(C:/Program Files/Microsoft SDKs/Windows/v7.1/Include)
LIBS +=  -lole32 -luser32 -lgdiplus \
        -L$$WORKSPACE/libraries/opencv/lib -lcv -lhighgui -lcxcore \
         -L$$quote(C:/Program Files/Microsoft SDKs/Windows/v7.1/Lib) -lstrmiids
