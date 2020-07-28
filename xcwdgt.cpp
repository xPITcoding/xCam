#include "xcwdgt.h"
#include "ui_xcwdgt.h"
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <QMenu>
#include <QPainter>
#include <QFileDialog>

// Namespace for using pylon objects.
using namespace Pylon;

using namespace GenApi;

// Settings for using Basler GigE cameras.
#include <pylon/gige/BaslerGigEInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;

xCWdgt::xCWdgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::xCWdgt)
{
    ui->setupUi(this);
    PylonInitialize();

    ui->pCamImgGV->setScene(new QGraphicsScene());
    ui->pCamImgGV->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->pCamImgGV->scene()->setBackgroundBrush(QBrush(QPixmap(":/images/texture2.png")));
    pCamThread=new xCamThread(this);
    pCamThread->setCamera(pCamera);

    connect(this,SIGNAL(MSG(const QString&)),this,SLOT(message(const QString&)));
    connect(ui->pCameraWidthSB,SIGNAL(valueChanged(int)),this,SLOT(cameraParameterChanged()));
    connect(ui->pCameraWidthSlider,SIGNAL(valueChanged(int)),this,SLOT(cameraParameterChanged()));
    connect(ui->pCameraHeightSB,SIGNAL(valueChanged(int)),this,SLOT(cameraParameterChanged()));
    connect(ui->pCameraHeightSlider,SIGNAL(valueChanged(int)),this,SLOT(cameraParameterChanged()));
    connect(ui->pFrameRateDSB,SIGNAL(valueChanged(double)),this,SLOT(cameraParameterChanged()));
    connect(ui->pFrameRateSlider,SIGNAL(valueChanged(int)),this,SLOT(frameRateSliderMoved(int)));
    connect(ui->pExposureDSB,SIGNAL(valueChanged(double)),this,SLOT(cameraParameterChanged()));
    connect(ui->pExposureSlider,SIGNAL(valueChanged(int)),this,SLOT(exposureTimeSliderMoved(int)));
    connect(ui->pGainSB,SIGNAL(valueChanged(int)),this,SLOT(cameraParameterChanged()));
    connect(ui->pGainSlider,SIGNAL(valueChanged(int)),this,SLOT(cameraParameterChanged()));
    connect(ui->pZoomSB,SIGNAL(valueChanged(int)),this,SLOT(zoomChanged()));
    connect(ui->pZoomSlider,SIGNAL(valueChanged(int)),this,SLOT(zoomChanged()));
    connect(ui->pBinningSB,SIGNAL(valueChanged(int)),this,SLOT(cameraParameterChanged()));

    pMenu = new QMenu();
    pMenu->addAction(ui->actionconnect);
    pMenu->addAction(ui->actionstart_stop_acquiring);
    pMenu->addAction(ui->actionsettings);
    pMenu->addAction(ui->actioninfo_panel);
    pMenu->addAction(ui->actionset_output_stream);

    connect(ui->actionset_output_stream,SIGNAL(triggered(bool)),this,SLOT(startStopStreaming(bool)));
    connect(ui->actionconnect,SIGNAL(triggered(bool)),this,SLOT(connectCamera(bool)));
    connect(ui->actionsettings,SIGNAL(triggered(bool)),this,SLOT(openSettings(bool)));
    connect(ui->actioninfo_panel,SIGNAL(triggered(bool)),this,SLOT(openInfo(bool)));
    connect(ui->actionstart_stop_acquiring,SIGNAL(triggered(bool)),this,SLOT(startStopGrabbing(bool)));
    connect(ui->pCamImgGV,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(popupMenu(const QPoint&)));

    pInfoPanelAnimation = new QPropertyAnimation(ui->pStatusPanel, "geometry");
    pInfoPanelAnimation->setDuration(500);
    pInfoPanelAnimation->setStartValue(QRect(795, 5, 0, 481));
    pInfoPanelAnimation->setEndValue(QRect(535, 5, 260, 481));
    pSettingsPanelAnimation = new QPropertyAnimation(ui->pSettingsPanel, "geometry");
    pSettingsPanelAnimation->setDuration(500);
    pSettingsPanelAnimation->setStartValue(QRect(795, 485, 0, 80));
    pSettingsPanelAnimation->setEndValue(QRect(5, 485, 790, 110));
    ui->pStatusPanel->setGeometry(QRect(795,5,0,481));
    ui->pSettingsPanel->setGeometry(QRect(795, 485, 0, 110));

    pStartEffect = new QSoundEffect();
    pStartEffect->setSource(QUrl::fromLocalFile(":/sounds/start.wav"));
    pStopEffect = new QSoundEffect();
    pStopEffect->setSource(QUrl::fromLocalFile(":/sounds/stop.wav"));
    pSlideEffect = new QSoundEffect();
    pSlideEffect->setSource(QUrl::fromLocalFile(":/sounds/slide.wav"));
    pErrorEffect = new QSoundEffect();
    pErrorEffect->setSource(QUrl::fromLocalFile(":/sounds/error.wav"));

    installEventFilter(this);
    updateStatus();
    cameraParameterChanged();
}

void xCWdgt::popupMenu(const QPoint& p)
{
    pMenu->popup(mapToGlobal(p));
}

void xCWdgt::startStopGrabbing(bool b)
{
    b ? start() : stop();    
}

void xCWdgt::startStopStreaming(bool b)
{
    if (b)
    {
        QString path = QFileDialog::getExistingDirectory(0,"pick target directory");
        if (!path.isEmpty() && pCamThread)
        {
            pCamThread->startStreaming(path,QString("xCam_frame_")+QDateTime::currentDateTime().toString("yyMMddhhmm"));
        }
        else
            ui->actionset_output_stream->setChecked(false);
    }
    else
        pCamThread->stopStreaming();
}

void xCWdgt::openSettings(bool b)
{
    if (b)
    {
        pSettingsPanelAnimation->setStartValue(QRect(795, 485, 0, 110));
        pSettingsPanelAnimation->setEndValue(QRect(5, 485, 790, 110));
        pSettingsPanelAnimation->start();
        pSlideEffect->play();
    }
    else
    {
        pSettingsPanelAnimation->setStartValue(QRect(5, 485, 790, 110));
        pSettingsPanelAnimation->setEndValue(QRect(795, 485, 0, 110));
        pSettingsPanelAnimation->start();
        pSlideEffect->play();
    }
}

void xCWdgt::openInfo(bool b)
{
    if (b)
    {
        pInfoPanelAnimation->setStartValue(QRect(795, 5, 0, 481));
        pInfoPanelAnimation->setEndValue(QRect(535, 5, 260, 481));
        pInfoPanelAnimation->start();
        pSlideEffect->play();
    }
    else
    {
        pInfoPanelAnimation->setEndValue(QRect(795, 5, 0, 481));
        pInfoPanelAnimation->setStartValue(QRect(535, 5, 260, 481));
        pInfoPanelAnimation->start();
        pSlideEffect->play();
    }
}

xCWdgt::~xCWdgt()
{
    delete ui;
}

void xCWdgt::message(const QString &txt)
{
    ui->pMSGBrowser->append(txt);
    if (txt.contains("Error")) pErrorEffect->play();
}

void xCWdgt::start()
{
    pStartEffect->play();
    pCamThread->start();
    _cameraRunning = true;
    updateStatus();
    updateSettingsPanel();
    zoomChanged();
}

void xCWdgt::stop()
{
    pStopEffect->play();
    pCamThread->stop();
    while (pCamThread->isRunning())
    {
       QThread::msleep(500);
    }
    _cameraRunning = false;
    updateStatus();
    updateSettingsPanel();
}

void xCWdgt::dispImage(QImage* img)
{
   if (!img) return;
   QPixmap pix(QPixmap::fromImage(*img));
   QPainter pain(&pix);
   pain.setPen(QPen(Qt::red));
   pCamThread->isStreaming() ?
               pain.drawText(QPointF(10,20),QString("streaming %1 fps").arg(1.0f/((float)_lastTime.msecsTo(QTime::currentTime()))*1000.f,0,'f',0)) :
               pain.drawText(QPointF(10,20),QString("%1 fps").arg(1.0f/((float)_lastTime.msecsTo(QTime::currentTime()))*1000.f,0,'f',0));
   pain.end();
    if (!pCurrentCamImgItem)
    {
        pCurrentCamImgItem = new QGraphicsPixmapItem(pix,0);
        ui->pCamImgGV->scene()->addItem(pCurrentCamImgItem);
    }
    else {
        pCurrentCamImgItem->setPixmap(pix);
        //pCurrentCamImgItem->update();
    }

    _lastTime = QTime::currentTime();
    if (img) delete img;
}

bool xCWdgt::eventFilter(QObject *watched, QEvent *event)
{
    if (watched==this && event->type()==CAMERA_FRAME)
    {
        CameraThread_Event *pCamThreadEvent = dynamic_cast<CameraThread_Event*>(event);
        if (pCamThreadEvent)
        {
            // received pixmap from thread
            dispImage(pCamThreadEvent->image());
        }
    }
    return QWidget::eventFilter(watched,event);
}


void xCWdgt::zoomChanged()
{
    if (!pCurrentCamImgItem) return;
    pCurrentCamImgItem->setScale(ui->pZoomSB->value()/100.0f);
    ui->pCamImgGV->scene()->setSceneRect(ui->pCamImgGV->scene()->itemsBoundingRect());
    ui->pCamImgGV->centerOn(pCurrentCamImgItem->mapToScene(pCurrentCamImgItem->boundingRect().center()));
}

void xCWdgt::exposureTimeSliderMoved(int val)
{
    ui->pExposureDSB->setValue((float)val/10.0f);
    cameraParameterChanged();
}

void xCWdgt::frameRateSliderMoved(int val)
{
    ui->pFrameRateDSB->setValue((float)val/10.f);
    cameraParameterChanged();
}


void xCWdgt::cameraParameterChanged()
{
    disconnect(ui->pCameraWidthSB,          SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    disconnect(ui->pCameraWidthSlider,      SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    disconnect(ui->pCameraHeightSB,         SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    disconnect(ui->pCameraHeightSlider,     SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    disconnect(ui->pFrameRateDSB,           SIGNAL(valueChanged(double)),   this,SLOT(cameraParameterChanged()));
    disconnect(ui->pFrameRateSlider,        SIGNAL(valueChanged(int)),      this,SLOT(frameRateSliderMoved(int)));
    disconnect(ui->pExposureDSB,            SIGNAL(valueChanged(double)),   this,SLOT(cameraParameterChanged()));
    disconnect(ui->pExposureSlider,         SIGNAL(valueChanged(int)),      this,SLOT(exposureTimeSliderMoved(int)));
    disconnect(ui->pGainSB,                 SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    disconnect(ui->pGainSlider,             SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    disconnect(ui->pBinningSB,              SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));

    ui->pExposureSlider->setValue(ui->pExposureDSB->value()*10.0f);
    ui->pFrameRateSlider->setValue(ui->pFrameRateDSB->value()*10.0f);

    if (pCamera && pCamConfig)
    {
        try
        {
            pCamConfig->_param._width = ui->pCameraWidthSB->value();
            pCamConfig->_param._height = ui->pCameraHeightSB->value();
            pCamConfig->_param._gain = (float)ui->pGainSB->value()/10.0f;
            pCamConfig->_param._frameRate = ui->pFrameRateDSB->value();
            pCamConfig->_param._exposureTime = ui->pExposureDSB->value();
            pCamConfig->_param._binning = ui->pBinningSB->value();

            // Allow all the names in the namespace GenApi to be used without qualification.
            using namespace GenApi;
            // Get the camera control object.
            INodeMap &control = pCamera->GetNodeMap();
            // Get the parameters for setting the image area of interest (Image AOI).
            const CIntegerPtr width = control.GetNode("Width");
            const CIntegerPtr height = control.GetNode("Height");
            const CBooleanPtr centerX = control.GetNode("CenterX");
            const CBooleanPtr centerY = control.GetNode("CenterY");
            const CFloatPtr gain = control.GetNode("Gain");
            const CFloatPtr exposureTime = control.GetNode("ExposureTime");
            const CBooleanPtr setFrameRate = control.GetNode("AcquisitionFrameRateEnable");
            const CFloatPtr aquFrameRate = control.GetNode("AcquisitionFrameRate");
            CEnumerationPtr exposureAuto(control.GetNode("ExposureAuto"));
            CEnumerationPtr gainAuto(control.GetNode("GainAuto"));
            const CIntegerPtr binningX = control.GetNode("BinningHorizontal");
            const CIntegerPtr binningY = control.GetNode("BinningVertical");

            // Maximize the Image AOI.
            //if (IsWritable(centerX)) centerX->SetValue(true);
            //if (IsWritable(centerY)) centerY->SetValue(true);
            if (IsImplemented(gain) && IsWritable(gain)) gain->SetValue(pCamConfig->_param._gain);
            //else emit MSG("Error: set gain");
            if (IsImplemented(exposureTime) && IsWritable(exposureTime)) exposureTime->SetValue(pCamConfig->_param._exposureTime*1000);
            //else emit MSG("Error: set exposure time");
            if (IsImplemented(setFrameRate) && IsWritable(setFrameRate)) setFrameRate->SetValue(true);
            //else emit MSG("Error: set frame rate mode");
            if (IsImplemented(aquFrameRate) && IsWritable(aquFrameRate)) aquFrameRate->SetValue(pCamConfig->_param._frameRate);
            //else emit MSG("Error: set frame rate");
            if (IsImplemented(width) && IsWritable(width)) width->SetValue(pCamConfig->_param._width);
            //else emit MSG("Error: set width");
            if (IsImplemented(height) && IsWritable(height)) height->SetValue(pCamConfig->_param._height);
            //else emit MSG("Error: set height");
            if (IsImplemented(centerX) && IsWritable(centerX)) centerX->SetValue(true);
            //else emit MSG("Error: set centerX");
            if (IsImplemented(centerY) && IsWritable(centerY)) centerY->SetValue(true);
            //else emit MSG("Error: set centerY");
            if (IsImplemented(binningX) && IsWritable(binningX)) binningX->SetValue(pCamConfig->_param._binning);
            //else emit MSG("Error: set binningX");
            if (IsImplemented(binningY) && IsWritable(binningY)) binningY->SetValue(pCamConfig->_param._binning);
            //else emit MSG("Error: set binningY");
            // Set the pixel data format.
            CEnumerationPtr(control.GetNode("PixelFormat"))->FromString("Mono8");
        }
        catch (...)
        {
            //emit MSG("error in setting camera parameter");
        }
    }

    connect(ui->pCameraWidthSB,         SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    connect(ui->pCameraWidthSlider,     SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    connect(ui->pCameraHeightSB,        SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    connect(ui->pCameraHeightSlider,    SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    connect(ui->pFrameRateDSB,          SIGNAL(valueChanged(double)),   this,SLOT(cameraParameterChanged()));
    connect(ui->pFrameRateSlider,       SIGNAL(valueChanged(int)),      this,SLOT(frameRateSliderMoved(int)));
    connect(ui->pExposureDSB,           SIGNAL(valueChanged(double)),   this,SLOT(cameraParameterChanged()));
    connect(ui->pExposureSlider,        SIGNAL(valueChanged(int)),      this,SLOT(exposureTimeSliderMoved(int)));
    connect(ui->pGainSB,                SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    connect(ui->pGainSlider,            SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
    connect(ui->pBinningSB,             SIGNAL(valueChanged(int)),      this,SLOT(cameraParameterChanged()));
}

void xCWdgt::updateDeviceInfo()
{
    if (pCamera && _cameraConnected)
    {
        // display device info
        // 4lines
        QString s;
        s+="name: "+QString((const char*)pCamera->GetDeviceInfo().GetModelName())+"\n";
        s+="class : "+QString((const char*)pCamera->GetDeviceInfo().GetDeviceClass())+"\n";
        s+="number : "+QString((const char*)pCamera->GetDeviceInfo().GetSerialNumber())+"\n";
        s+="mode :"+QString((const char*)pCamera->GetDeviceInfo().GetTransferMode());
        ui->pInfoLab->setText(s);
    }
    else
        ui->pInfoLab->setText("");
}

void xCWdgt::updateSettingsPanel()
{
    // enable disable features
    // get limits from camera
    if (pCamera && _cameraConnected)
    {
        try
        {
            INodeMap& nodemap = pCamera->GetNodeMap();
            CIntegerPtr offsetX( nodemap.GetNode( "OffsetX"));
            CIntegerPtr offsetY( nodemap.GetNode( "OffsetY"));
            CIntegerPtr width( nodemap.GetNode( "Width"));
            CIntegerPtr height( nodemap.GetNode( "Height"));
            CIntegerPtr binning( nodemap.GetNode("BinningHorizontal"));

            ui->pCameraWidthSB->setRange(width->GetMin(),width->GetMax());
            ui->pCameraWidthSlider->setRange(width->GetMin(),width->GetMax());
            ui->pCameraWidthSB->setValue(width->GetValue());
            ui->pCameraWidthSlider->setValue(width->GetValue());
            ui->pCameraWidthSB->setEnabled(IsWritable(width));
            ui->pCameraWidthSlider->setEnabled(IsWritable((width)));

            ui->pCameraHeightSB->setRange(height->GetMin(),height->GetMax());
            ui->pCameraHeightSlider->setRange(height->GetMin(),height->GetMax());
            ui->pCameraHeightSB->setValue(height->GetValue());
            ui->pCameraHeightSlider->setValue(height->GetValue());
            ui->pCameraHeightSB->setEnabled(IsWritable(height));
            ui->pCameraHeightSlider->setEnabled(IsWritable((height)));

            ui->pBinningSB->setRange(binning->GetMin(),binning->GetMax());
            ui->pBinningSB->setSingleStep(binning->GetInc());
            ui->pBinningSB->setValue(binning->GetValue());
            ui->pBinningSB->setEnabled(IsWritable(binning));

            CEnumerationPtr gainAuto( nodemap.GetNode( "GainAuto"));
            if ( IsWritable( gainAuto))
            {
                gainAuto->FromString("Off");
            }

            CFloatPtr gain( nodemap.GetNode( "Gain"));
            ui->pGainSB->setRange(gain->GetMin()*10.0f,gain->GetMax()*10.0f);
            ui->pGainSB->setValue(gain->GetValue()*10.0f);
            ui->pGainSlider->setRange(gain->GetMin()*10.0f,gain->GetMax()*10.0f);
            ui->pGainSlider->setValue(gain->GetValue()*10.0f);
            ui->pGainSB->setEnabled(IsWritable(gain));
            ui->pGainSB->setEnabled(IsWritable(gain));


            CEnumEntryPtr exposureMode(nodemap.GetNode("ExposureMode"));
            if (IsWritable(exposureMode))
                exposureMode->FromString("ExposureMode_Timed");

            CFloatPtr exposureTime(nodemap.GetNode("ExposureTimeAbs"));
            CBooleanPtr enableFrameRate(nodemap.GetNode("AcquisitionFrameRateEnable"));
            const CFloatPtr aquFrameRate = nodemap.GetNode("AcquisitionFrameRateAbs");
            if ( IsWritable( enableFrameRate))
            {
                enableFrameRate->SetValue(true);
            }

            ui->pFrameRateDSB->setRange(aquFrameRate->GetMin(),aquFrameRate->GetMax());
            ui->pFrameRateDSB->setSingleStep(aquFrameRate->GetInc());
            ui->pFrameRateDSB->setValue(aquFrameRate->GetValue());
            ui->pFrameRateDSB->setEnabled(IsWritable(aquFrameRate));
            ui->pFrameRateSlider->setRange(aquFrameRate->GetMin()*10,aquFrameRate->GetMax()*10);
            ui->pFrameRateSlider->setSingleStep(aquFrameRate->GetInc()*10);
            ui->pFrameRateSlider->setValue(aquFrameRate->GetValue()*10);
            ui->pFrameRateSlider->setEnabled(IsWritable(aquFrameRate));

            emit MSG(QString("Exposure : %1 ... %2").arg(exposureTime->GetMin()).arg(exposureTime->GetMax()));
        }
        catch(...)
        {
        }

    }
}

void xCWdgt::updateStatus()
{
    if (_cameraError) pErrorEffect->play();
    if (_cameraConnected)
    {
        // enable all actions and the settings panel
        ui->actionconnect->setChecked(true);
        ui->actionsettings->setEnabled(true);
        ui->actionstart_stop_acquiring->setEnabled(true);
        ui->pSettingsPanel->setEnabled(true);
        updateDeviceInfo();
        // set conncet to disconnect
        if (_cameraError)
        {
            // display error
            ui->pStatusPixLab->setPixmap(QPixmap(":/images/traffic_light_red.png"));
            ui->pStatusLab->setText("camera error");
        }
        else
        {
            // get camera info and display it
            if (_cameraRunning)
            {
                // display green
                ui->pStatusPixLab->setPixmap(QPixmap(":/images/traffic_light_green.png"));
                ui->pStatusLab->setText("acquiring");
                // update settings panel
                updateSettingsPanel();
                // set start stop to stop
                ui->actionstart_stop_acquiring->setChecked(true);
            }
            else
            {
                // display yellow
                ui->pStatusPixLab->setPixmap(QPixmap(":/images/traffic_light_yellow.png"));
                ui->pStatusLab->setText("stopped");
                // upate settings panel
                updateSettingsPanel();
                // set start stop to start
                ui->actionstart_stop_acquiring->setChecked(false);
            }
        }
    }
    else
    {
        // disable all actions beside connect
        // disable settings panel
        // set connect to connect
        ui->actionconnect->setChecked(false);
        ui->actionsettings->setEnabled(false);
        ui->actionstart_stop_acquiring->setEnabled(false);
        ui->pSettingsPanel->setEnabled(false);
        // display empty traffic light
        ui->pStatusPixLab->setPixmap(QPixmap(":/images/traffic_light_empty.png"));
        ui->pStatusLab->setText("not connected");
    }
}

void xCWdgt::connectCamera(bool b)
{
    if (!_cameraConnected && b)
    {
        try
        {
            // Create an instant pCamera object with the pCamera device found first.
            if (pCamera!=NULL)
                delete pCamera;

            pCamera = new CBaslerUsbInstantCamera (CTlFactory::GetInstance().CreateFirstDevice());
            pCamConfig = new CPixelFormatAndAoiConfiguration();

            pCamConfig->_param._width = ui->pCameraWidthSB->value();
            pCamConfig->_param._height = ui->pCameraHeightSB->value();
            pCamConfig->_param._gain = ui->pGainSB->value();
            pCamConfig->_param._frameRate = ui->pFrameRateDSB->value();
            pCamConfig->_param._exposureTime = ui->pExposureDSB->value();

            if (pCamera->IsPylonDeviceAttached())
            {
                pCamera->RegisterConfiguration( pCamConfig, RegistrationMode_ReplaceAll, Cleanup_Delete);
                pCamera->Open();

                // Print the model name of the pCamera.
                emit MSG(QString("Using device %1").arg((const char*)pCamera->GetDeviceInfo().GetModelName()));

                // The parameter MaxNumBuffer can be used to control the count of buffers
                // allocated for grabbing. The default value of this parameter is 10.
                pCamera->MaxNumBuffer = 5;
                pCamera->PixelFormat.SetValue(PixelFormat_Mono8);
                pCamera->ExposureAuto.SetValue(ExposureAuto_Off);
                pCamera->BinningHorizontalMode.SetValue(BinningHorizontalMode_Average);
                pCamera->BinningVerticalMode.SetValue(BinningVerticalMode_Average);
                pCamThread->setCamera(pCamera);

                _cameraConnected = true;
                _cameraError = false;
            }
            else
            {
                emit MSG("No valid pylon device found");
                _cameraError = true;
                _cameraConnected = false;
                pCamera = NULL;
                pCamConfig = NULL;
            }
        }
        catch (const GenericException &e)
        {
            // Error handling.
            emit MSG(QString("An exception occurred. %1").arg(e.GetDescription()));
            _cameraError = true;
            _cameraConnected = false;
            pCamera = NULL;
            pCamConfig = NULL;
        }
    }
    if (!b && _cameraConnected)
    {
        pCamThread->stop();
        while (pCamThread->isRunning())
            QThread::msleep(100);
        if (pCamera) pCamera->Close();
        _cameraError = false;
        _cameraRunning = false;
        _cameraConnected = false;
    }
    updateStatus();
}
