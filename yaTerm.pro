TEMPLATE = app

QT += qml quick widgets serialport
CONFIG += c++11
#QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    simpleterminal.cpp \
    stringlistmodel.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    simpleterminal.h \
    stringlistmodel.h
