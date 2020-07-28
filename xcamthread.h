#ifndef XCAMTHREAD_H
#define XCAMTHREAD_H

#include <QObject>
#include <QDialog>
#include <QThread>
#include <QEvent>
#include <QTime>
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

class xCWdgt;

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;

enum CameraThread_Event_Types
{
    CAMERA_FRAME = QEvent::User,
    CAMERA_AV_BRIGHTNESS = QEvent::User+1
};

class CameraThread_Event : public QEvent
{
public:
    CameraThread_Event(QImage *p):QEvent(QEvent::User)
    {
        pImg = p;
    }
    QImage *image(){return pImg;}
protected:
    QImage *pImg = NULL;
};

class xCamThread : public QThread
{
public:
    explicit xCamThread(xCWdgt *d);
    ~xCamThread();
    void setCamera( CInstantCamera *c){camera=c;}

    void start(){abb=false;QThread::start();}
    void stop(){abb=true;}
    void startStreaming(QString _p, QString _s){_streamingCount=0;_streamingPath=_p;_suffix=_s;_isStreaming=true;}
    void stopStreaming(){_isStreaming=false;}
    bool isStreaming(){return  _isStreaming;}

protected:
    virtual void run();
    CInstantCamera *camera;
    bool abb;
    xCWdgt *pParentWdgt=nullptr;

    bool _isStreaming = false;
    QString _streamingPath;
    QString _suffix;
    long _streamingCount = 0;
};

#endif // XCAMTHREAD_H
