#include "analist.h"

Analist::Analist()
{
    oldSetPoint = 0;
}

void Analist::calc(double nivel, double setPoint, double timeStamp)
{
    reset(setPoint);
    calcMpTp(nivel, setPoint);
    calcTr(nivel, setPoint, timeStamp);
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
        tr = 0;
        for(int i = 0; i < 4; i++) trOldTime[i] = 0;
        mp = -100;
        tp = 0;
        tsOldTime = 0;
    }
}

double Analist::calcTs(double nivel, double setPoint, double timeStamp)
{
    double tsAux[4];

    if (ts == 0) tsOldTime = timeStamp; //botar no reset

    if (direction){
        if (nivel < abs(setPoint - initialLevel)*0.02 || nivel > abs(setPoint - initialLevel)*0.02)
            tsAux[0] = timeStamp - tsOldTime;

        if (nivel < abs(setPoint - initialLevel)*0.05 || nivel > abs(setPoint - initialLevel)*0.05)
            tsAux[1] = timeStamp - tsOldTime;

        if (nivel < abs(setPoint - initialLevel)*0.07 || nivel > abs(setPoint - initialLevel)*0.07)
            tsAux[2] = timeStamp - tsOldTime;

        if (nivel < abs(setPoint - initialLevel)*0.1 || nivel > abs(setPoint - initialLevel)*0.1)
            tsAux[3] = timeStamp - tsOldTime;
    }

    if (tsOpt == 2) ts = tsAux[0];
    else if (tsOpt == 5) ts = tsAux[1];
    else if (tsOpt == 7) ts = tsAux[2];
    else ts = tsAux[3];

    return ts;
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



double Analist::calcTr(double nivel, double setPoint, double timeStamp)
{

    //if (tr[0] == 0) trOldTime[0] = timeStamp; //botar no reset

    double trAux[4];

    if (tr == 0) {
        trOldTime[0] = timeStamp;
        initialLevel = nivel;
    }

    if (direction){
        if (nivel <= setPoint)
            trAux[0] = timeStamp - trOldTime[0];

        if (nivel <= (setPoint-initialLevel)*0.05)
            trOldTime[1] = timeStamp;
        else if (nivel <= setPoint*0.95)
            trAux[1] = timeStamp - trOldTime[1];

        if (nivel <= (setPoint-initialLevel)*0.1)
            trOldTime[2] = timeStamp;
        else if (nivel <= setPoint*0.9)
            trAux[2] = timeStamp - trOldTime[2];
    }
    else {
        if (nivel >= setPoint)
            trAux[0] = timeStamp - trOldTime[0];

        if (nivel >= (initialLevel-setPoint)*0.05)
            trOldTime[1] = timeStamp;
        else if (nivel >= (initialLevel-setPoint)*0.95)
            trAux[1] = timeStamp - trOldTime[1];

        if (nivel >= (initialLevel-setPoint)*0.1)
            trOldTime[2] = timeStamp;
        else if (nivel >= (initialLevel-setPoint)*0.9)
            trAux[2] = timeStamp - trOldTime[2];

    }

    if (trOpt == 0) tr = trAux[0];
    else if (trOpt == 5) tr = trAux[1];
    else tr = trAux[2];

    return tr;
}

