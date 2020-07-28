#ifndef XCWDGT_H
#define XCWDGT_H

#include <QWidget>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
#include <QSoundEffect>
#include "xcamthread.h"
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;

QT_BEGIN_NAMESPACE
namespace Ui { class xCWdgt; }
QT_END_NAMESPACE

struct XCAM_PARAM
{
    // current Values
    int _width, _height;
    float _frameRate;
    float _exposureTime;
    float _gain;
    int _binning;
    // status
    bool _valid = false;
};

class CPixelFormatAndAoiConfiguration : public Pylon::CConfigurationEventHandler
{
public:
    XCAM_PARAM _param;
    void OnOpened( Pylon::CInstantCamera& camera)
    {
        try
        {
            // Allow all the names in the namespace GenApi to be used without qualification.
            using namespace GenApi;
            // Get the camera control object.
            INodeMap &control = camera.GetNodeMap();
            // Get the parameters for setting the image area of interest (Image AOI).
            const CIntegerPtr width = control.GetNode("Width");
            const CIntegerPtr height = control.GetNode("Height");
            const CBooleanPtr centerX = control.GetNode("CenterX");
            const CBooleanPtr centerY = control.GetNode("CenterY");
            const CIntegerPtr gainRaw = control.GetNode("GainRaw");
            const CFloatPtr exposureTime = control.GetNode("ExposureTimeAbs");
            const CBooleanPtr setFrameRate = control.GetNode("AcquisitionFrameRateEnable");
            const CFloatPtr aquFrameRate = control.GetNode("AcquisitionFrameRateAbs");
            CEnumerationPtr(control.GetNode("ExposureAuto"))->FromString("Off");
            CEnumerationPtr(control.GetNode("GainAuto"))->FromString("Off");

            // Maximize the Image AOI.
            if (IsWritable(centerX)) centerX->SetValue(true);
            if (IsWritable(centerY)) centerY->SetValue(true);
            if (IsWritable(gainRaw)) gainRaw->SetValue(_param._gain);
            if (IsWritable(exposureTime)) exposureTime->SetValue(_param._exposureTime*1000);
            if (IsWritable(setFrameRate)) setFrameRate->SetValue(true);
            if (IsWritable(aquFrameRate)) aquFrameRate->SetValue(_param._frameRate);

            width->SetValue(_param._width);
            height->SetValue(_param._height);
            // Set the pixel data format.
            CEnumerationPtr(control.GetNode("PixelFormat"))->FromString("Mono8");
        }
        catch (const Pylon::GenericException& e)
        {
            throw RUNTIME_EXCEPTION( "Could not apply configuration. const GenericException caught in OnOpened method msg=%hs", e.what());
        }
    }
};

class xCWdgt : public QWidget
{
    Q_OBJECT

public:
    xCWdgt(QWidget *parent = nullptr);
    ~xCWdgt();

    XCAM_PARAM parameter(){ if (pCamConfig)
                                return pCamConfig->_param;
                            else
                                return XCAM_PARAM();}

signals:
    void MSG(const QString&);

public slots:
    void connectCamera(bool);
    void start();
    void stop();

protected slots:
    void message(const QString&);
    void cameraParameterChanged();
    void dispImage(QImage*);
    void exposureTimeSliderMoved(int);
    void frameRateSliderMoved(int);
    void popupMenu(const QPoint&);
    void startStopGrabbing(bool);
    void openSettings(bool);
    void openInfo(bool);
    void startStopStreaming(bool);
    void zoomChanged();
    virtual bool eventFilter(QObject *o, QEvent *e) override;

protected:
    void updateStatus();
    void updateSettingsPanel();
    void updateDeviceInfo();

private:
    Ui::xCWdgt *ui;

    CPixelFormatAndAoiConfiguration *pCamConfig     = nullptr;
    CBaslerUsbInstantCamera *pCamera                = nullptr;
    xCamThread *pCamThread                          = nullptr;
    QGraphicsPixmapItem *pCurrentCamImgItem         = nullptr;
    bool _cameraConnected = false;
    bool _cameraError = false;
    bool _cameraRunning = false;

    QMenu *pMenu = nullptr;
    QPropertyAnimation *pInfoPanelAnimation         = nullptr;
    QPropertyAnimation *pSettingsPanelAnimation     = nullptr;

    QSoundEffect *pSlideEffect = nullptr;
    QSoundEffect *pStartEffect = nullptr;
    QSoundEffect *pStopEffect = nullptr;
    QSoundEffect *pErrorEffect = nullptr;

    QTime _lastTime;

};
#endif // XCWDGT_H
