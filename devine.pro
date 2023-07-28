QT       += core gui xml widgets
lessThan(QT_MAJOR_VERSION, 6): QT += winextras

TEMPLATE=vcapp

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    about.cpp \
    columns.cpp \
    main.cpp \
    mainwindow.cpp \
    properties.cpp


HEADERS += \
    about.h \
    columns.h \
    mainwindow.h \
    properties.h

FORMS += \
    about.ui \
    columns.ui \
    mainwindow.ui \
    properties.ui

SOURCES += nativeevents.cpp
HEADERS += nativeevents.h
SOURCES += deviceinfoset.cpp
HEADERS += deviceinfoset.h
SOURCES += glue.cpp
HEADERS += glue.h
SOURCES += device.cpp
HEADERS += device.h

QMAKE_LFLAGS_DEBUG += /DEFAULTLIB:"Setupapi.lib"
QMAKE_LFLAGS_RELEASE += /DEFAULTLIB:"Setupapi.lib"
QMAKE_LFLAGS_DEBUG += /DEFAULTLIB:"Cfgmgr32.lib"
QMAKE_LFLAGS_RELEASE += /DEFAULTLIB:"Cfgmgr32.lib"

QMAKE_LFLAGS_RELEASE += /DEBUG

QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_CXXFLAGS_RELEASE += /Od

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    devine.qrc 

RC_ICONS = devine.ico

QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

MOC_DIR = .moc
RCC_DIR = .qrc
UI_DIR = .ui
