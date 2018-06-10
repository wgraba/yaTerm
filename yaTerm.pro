TEMPLATE = app

QT += qml widgets serialport
CONFIG += c++17
#QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    src/main.cpp \
    src/simpleterminal.cpp \
    src/portswatcher.cpp \
    src/commandparser.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/simpleterminal.h \
    src/portswatcher.h \
    src/commandparser.h
