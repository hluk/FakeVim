include(../fakevim/fakevim.pri)

SOURCES += \
    fakevim_test.cpp \
    ../test/editor.cpp

HEADERS += fakevimplugin.h

CONFIG += qt
QT += testlib
