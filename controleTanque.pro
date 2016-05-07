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

CONFIG += c++11

SOURCES += main.cpp\
        supervisorio.cpp \
    qcustomplot.cpp \
    commthread.cpp \
    analist.cpp

HEADERS  += supervisorio.h \
    qcustomplot.h \
    commthread.h \
    quanser.h \
    analist.h

FORMS += \
    supervisorio.ui

RESOURCES += \
    images.qrc

LIBS += -llapack -lblas -L"/usr/local/Cellar/armadillo/6.700.4/lib/" -larmadillo

INCLUDEPATH += /usr/local/Cellar/armadillo/6.700.4/include/
