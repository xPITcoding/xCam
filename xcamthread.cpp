#include "xcamthread.h"
#include "xcwdgt.h"
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QCoreApplication>
#include <stdlib.h>

xCamThread::xCamThread(xCWdgt *parent):QThread ()
{
    pParentWdgt=parent;
    abb=false;
}

xCamThread::~xCamThread()
{}

void xCamThread::run()
{
    while (!abb)
    {
        if (camera)
        {
            try
            {

                // Start the grabbing of c_countOfImagesToGrab images.
                // The camera device is parameterized with a default configuration which
                // sets up free-running continuous acquisition.
                camera->StartGrabbing( 1000);

                // This smart pointer will receive the grab result data.
                CGrabResultPtr ptrGrabResult;
                CPylonImage iimg;
                quint8 *pImageBuffer;
                int dimx,dimy;

                // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
                // when c_countOfImagesToGrab images have been retrieved.
                while ( camera->IsGrabbing() && !abb)
                {
                    // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
                    camera->RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

                    // Image grabbed successfully?
                    if (ptrGrabResult->GrabSucceeded())
                    {
                        dimx=ptrGrabResult->GetWidth();
                        dimy=ptrGrabResult->GetHeight();
                        iimg.AttachGrabResultBuffer(ptrGrabResult);
                        pImageBuffer = (quint8 *) ptrGrabResult->GetBuffer();
                        // memory handling
                        QImage bild((uchar*)iimg.GetBuffer(),dimx, dimy, QImage::Format_Grayscale8);

                        // streaming
                        if (isStreaming())
                        {
                            bild.save(_streamingPath+"/"+_suffix+QString("_%1.bmp").arg(_streamingCount++,6,10,QChar('0')));
                        }

                        // should by asynchron!!
                        QCoreApplication::postEvent(pParentWdgt,new CameraThread_Event(new QImage(bild)));
                        //pParentWdgt->emit MSG(QString("av. intensity: %1").arg(_sumBuffer));
                    }
                    else
                    {
                        pParentWdgt->emit MSG(QString("Error: %1 %2").arg(ptrGrabResult->GetErrorCode()).arg((const char*)ptrGrabResult->GetErrorDescription()));
                    }
                }
                camera->StopGrabbing();
            }
            catch (const GenericException &e)
            {
                // Error handling.
                pParentWdgt->emit MSG(QString("Error: An exception occurred. %1").arg(e.GetDescription()));
            }
        }
    }
}
