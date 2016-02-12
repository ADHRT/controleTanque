#-------------------------------------------------
#
# Project created by QtCreator 2016-02-05T11:32:52
#
#-------------------------------------------------


#  ADICIONAR ESSE PRINTSUPPORT AÍ FOI O QU FEZ ACABAR UM ERRO BIZARRO QUE ESTAVA TENDO

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = controleTanque
TEMPLATE = app


SOURCES += main.cpp\
        supervisorio.cpp \
    qcustomplot.cpp \
    commthread.cpp

HEADERS  += supervisorio.h \
    qcustomplot.h \
    commthread.h

FORMS += \
    supervisorio.ui

RESOURCES += \
    images.qrc


