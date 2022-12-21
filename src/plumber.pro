#-------------------------------------------------
#
# Project created by QtCreator 2020-04-17T10:57:52
#
#-------------------------------------------------

QT       += core gui network multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

TARGET = plumber
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

# Set program version
VERSION = 3.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

SOURCES += \
        RangeSlider.cpp \
        controlbutton.cpp \
        elidedlabel.cpp \
        engine.cpp \
        error.cpp \
        main.cpp \
        mainwindow.cpp \
        onlinesearchsuggestion.cpp \
        remotepixmaplabel.cpp \
        request.cpp \
        screenshot.cpp \
        searchprovider.cpp \
        seekslider.cpp \
        settings.cpp \
        utils.cpp \
        waitingspinnerwidget.cpp

HEADERS += \
        RangeSlider.h \
        controlbutton.h \
        elidedlabel.h \
        engine.h \
        error.h \
        mainwindow.h \
        onlinesearchsuggestion.h \
        remotepixmaplabel.h \
        request.h \
        screenshot.h \
        searchprovider.h \
        seekslider.h \
        settings.h \
        utils.h \
        waitingspinnerwidget.h

FORMS += \
        console.ui \
        error.ui \
        mainwindow.ui \
        screenshot.ui \
        searchprovider.ui \
        settings.ui \
        track.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc \
    qbreeze.qrc
