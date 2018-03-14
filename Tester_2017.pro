#-------------------------------------------------
#
# Project created by QtCreator 2017-04-25T15:38:22
#
#-------------------------------------------------

QT      += core gui
QT      += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Tester_2017
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    asciiProgram.cpp \
    dataToXml.cpp \
    main.cpp \
    MainWindow.cpp \
    serialPortManager.cpp \
    timeManager.cpp \
    i2c.cpp \
    actuator.cpp \
    stepper.cpp

HEADERS  += \
    asciiProgram.h \
    dataToXml.h \
    MainWindow.h \
    serialPortManager.h \
    timeManager.h \
    i2c.h \
    actuator.h \
    stepper.h

FORMS    += \
    MainWindow.ui

target.path = /home/pi/
INSTALLS += target

DISTFILES += \
    Tester_2017.pro.user

INCLUDEPATH +=$$[QT_SYSROOT]/usr/local/include
LIBS += -L$$[QT_SYSROOT]/usr/local/lib -lwiringPi

RESOURCES += \
    resource.qrc
