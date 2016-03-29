#include "analist.h"
#include <qmath.h>

Analist::Analist()
{
    oldSetPoint = 0;
    for (int i = 0; i < 3; i++){
        porcInital[i] = i*5/100.0;
        porcFinal[i] = (100 - i*5)/100.0;
        reachedTr[i] = false;
        qDebug() << "P[" << i << "]: " << porcFinal[i];
    }
    initialLevel = -1;
}

void Analist::calc(double nivel, double setPoint, double timeStamp, int tsOpt)
{
    //qDebug() << "SP: " << setPoint << "OSP: " << oldSetPoint;
    reset(setPoint);
    calcMpTp(nivel, setPoint);
    calcTr(nivel, setPoint);
    calcTs( nivel,  setPoint,  timeStamp, tsOpt);
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

double Analist::getTr(int i)
{
    return tr[i];
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
        //qDebug() << "reset";
        qDebug() << "Entrou Reset";
        oldSetPoint = setPoint;
        ts = -100;
        for(int i = 0; i < 3; i++) {
            trOldTime[i] = 0;
            tr[i] = 0;
            reachedTr[i] = false;
        }
        initialLevel = -1;
        mp = -100;
        tp = 0;
        tsOldTime = 0;
    }
}

double Analist::calcTs(double nivel, double setPoint, double timeStamp, int tsOpt)
{

    if(ts == -100) {
        ts=0;
        InitialLevelTs=nivel;
        tsInitialTime=QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    }

    double currentTimeTs=QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    double nivelDiff=qFabs(setPoint-InitialLevelTs);

    //qDebug() << currentTimeTs << tsInitialTime << currentTimeTs-tsInitialTime;
    if (qFabs((setPoint-InitialLevelTs)*0.02)<qFabs(nivel-setPoint))
        tsAux[0] = currentTimeTs - tsInitialTime;

    if (qFabs((setPoint-InitialLevelTs)*0.05)<qFabs(nivel-setPoint))
        tsAux[1] = currentTimeTs - tsInitialTime;

    if (qFabs((setPoint-InitialLevelTs)*0.07)<qFabs(nivel-setPoint))
        tsAux[2] = currentTimeTs - tsInitialTime;

    if (qFabs((setPoint-InitialLevelTs)*0.1)<qFabs(nivel-setPoint))
        tsAux[3] = currentTimeTs - tsInitialTime;

    //qDebug() << qFabs(setPoint-InitialLevelTs)*0.1 << qFabs(nivel-setPoint);

    if (tsOpt == 2) ts = tsAux[0];
    else if (tsOpt == 5) ts = tsAux[1];
    else if (tsOpt == 7) ts = tsAux[2];
    else ts = tsAux[3];

    return ts;
}

double Analist::calcMpTp(double nivel, double setPoint)
{
    //qDebug() << nivel << setPoint << mp;
    if(mp == -100) {
        if(nivel > setPoint) direction = false; // Esta descendo
        else direction = true;
        mp = 0;
        //qDebug() << "Entrou MP";
        mpInitialTime = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    }
    if((direction && nivel-setPoint>mp) || (!direction && nivel-setPoint<mp)){
        mp=nivel-setPoint;
        //qDebug() << mp;
        tp=QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - mpInitialTime;
    }

}



void Analist::calcTr(double nivel, double setPoint)
{
    double timeStampTr = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    //if (tr[0] == 0) trOldTime[0] = timeStamp; //botar no reset
    qDebug() <<  "SP: " << setPoint << " - IL: " << initialLevel <<  " - L: " << nivel <<" - tr: " << tr[0] << " - trOld: " << trOldTime[0] << " - TS: " << timeStampTr << "- Flag: " << reachedTr;
    qDebug() << "condInic: " << (nivel <= (setPoint - initialLevel)*porcInital[0] + initialLevel) << "condFim: " << (nivel <= (setPoint - initialLevel)*porcFinal[0] + initialLevel);
    if (initialLevel == -1) {
        initialLevel = nivel;
        //qDebug() << "PI: " << porcInital[0] << " - PF: " << porcFinal[0];
    }
    for (int i = 0; i < 3; i++){
        if (reachedTr[i]);
            //reachedTr = true;
            //qDebug() << "Entrou" << reachedTr;
        else if (direction){
            if (nivel <= (setPoint - initialLevel)*porcInital[i] + initialLevel){
                trOldTime[i] = timeStampTr;
                //qDebug() << i << " - if nivel menor" ;
            }
            else if (nivel <= (setPoint - initialLevel)*porcFinal[i] + initialLevel){
                tr[i] = timeStampTr - trOldTime[i];
                //qDebug() << i << " - else nivel menor";
            }
            else
                reachedTr[i] = true;
                //qDebug() << "Entrou uma vez so" << reachedTr;
        }
        else {
            if (nivel >= initialLevel - (initialLevel-setPoint)*porcInital[i]){
                trOldTime[i] = timeStampTr;
                //qDebug() << i << " - if nivel maior";
            }
            else if (nivel >= initialLevel - (initialLevel-setPoint)*porcFinal[i]){
                tr[i] = timeStampTr - trOldTime[i];
                //qDebug() << i << " - else nivel maior";
            }
            else
                reachedTr[i] = true;
        }
    }
}

