QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    xcamthread.cpp \
    xcwdgt.cpp

HEADERS += \
    xcamthread.h \
    xcwdgt.h

FORMS += \
    xcwdgt.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd_party/tiff-4.0.10/libtiff/release/ -ltiff
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rd_party/tiff-4.0.10/libtiff/debug/ -ltiff
else:unix: LIBS += -L$$PWD/../../3rd_party/tiff-4.0.10/libtiff/ -ltiff

INCLUDEPATH += $$PWD/../../3rd_party/tiff-4.0.10/libtiff
DEPENDPATH += $$PWD/../../3rd_party/tiff-4.0.10/libtiff


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/release/ -lpylonbase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/debug/ -lpylonbase
else:unix: LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/ -lpylonbase

INCLUDEPATH += $$PWD/../../../../../opt/pylon5/include
DEPENDPATH += $$PWD/../../../../../opt/pylon5/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/release/ -lGenApi_gcc_v3_0_Basler_pylon_v5_0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/debug/ -lGenApi_gcc_v3_0_Basler_pylon_v5_0
else:unix: LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/ -lGenApi_gcc_v3_0_Basler_pylon_v5_0

INCLUDEPATH += $$PWD/../../../../../opt/pylon5/include/GenApi
DEPENDPATH += $$PWD/../../../../../opt/pylon5/include/GenApi

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/release/ -lGCBase_gcc_v3_0_Basler_pylon_v5_0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/debug/ -lGCBase_gcc_v3_0_Basler_pylon_v5_0
else:unix: LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/ -lGCBase_gcc_v3_0_Basler_pylon_v5_0

INCLUDEPATH += $$PWD/../../../../../opt/pylon5/include/Base
DEPENDPATH += $$PWD/../../../../../opt/pylon5/include/Base

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/release/ -lpylonc
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/debug/ -lpylonc
else:unix: LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/ -lpylonc

INCLUDEPATH += $$PWD/../../../../../opt/pylon5/lib64
DEPENDPATH += $$PWD/../../../../../opt/pylon5/lib64

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/release/ -lpylonutility
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/debug/ -lpylonutility
else:unix: LIBS += -L$$PWD/../../../../../opt/pylon5/lib64/ -lpylonutility

INCLUDEPATH += $$PWD/../../../../../opt/pylon5/lib64
DEPENDPATH += $$PWD/../../../../../opt/pylon5/lib64

RESOURCES += \
    xCam_resources.qrc
