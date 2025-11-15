# Qt项目文件，用于测试新变换系统
QT += core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = test-transform-system

SOURCES += \
    transform-system.cpp \
    test-transform-system.cpp

HEADERS += \
    transform-system.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target