CONFIG += testcase testcase_lowdpi
TARGET = tst_qgraphicsscene
QT += widgets widgets-private testlib
QT += core-private gui-private
SOURCES  += tst_qgraphicsscene.cpp
RESOURCES += images.qrc
win32: QMAKE_USE += user32

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_NO_CAST_TO_ASCII

RESOURCES += testdata.qrc
