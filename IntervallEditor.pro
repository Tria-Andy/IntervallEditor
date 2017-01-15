#-------------------------------------------------
#
# Project created by QtCreator 2017-01-02T07:46:46
#
#-------------------------------------------------

QT       += core gui
QT       += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = IntervallEditor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    standardworkouts.cpp \
    calculation.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    del_editvalues.h \
    standardworkouts.h \
    calculation.h

FORMS    += mainwindow.ui
