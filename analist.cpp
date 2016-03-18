#include "analist.h"

Analist::Analist()
{
    oldSetPoint = 0;
}

void Analist::calc(double nivel, double setPoint, double timeStamp)
{
    reset(setPoint);
    calcMpTp(nivel, setPoint);
}

double Analist::getTs()
{
    return ts;
}

double Analist::getMp()
{
    return mp;
}

double Analist::getTp()
{
    return tp;
}

double Analist::getTr()
{
    return tr;
}

void Analist::setTsOpt(int)
{

}

void Analist::setTrOpt(int)
{

}

void Analist::reset(double setPoint)
{
    // detecta mudanca de setPoint para iniciar o calculo das variaveis
    if(setPoint != oldSetPoint) {
        qDebug() << "Entrou Reset";
        oldSetPoint = setPoint;
        ts = 0;
        mp = -100;
        tp = 0;
        tr = 0;
        tsOldTime = 0;
        trOldTime = 0;
    }
}

double Analist::calcTs(double nivel, double setPoint)
{

}

double Analist::calcMpTp(double nivel, double setPoint)
{
    qDebug() << nivel << setPoint << mp;
    if(mp == -100) {
        if(nivel > setPoint) direction = false; // Esta descendo
        else direction = true;
        mp = 0;
        qDebug() << "Entrou MP";
        mpInitialTime = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    }
    if((direction && nivel-setPoint>mp) || (!direction && nivel-setPoint<mp)){
        mp=nivel-setPoint;
        qDebug() << mp;
        tp=QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - mpInitialTime;
    }

}



double Analist::calcTr(double nivel, double setPoint)
{

}

