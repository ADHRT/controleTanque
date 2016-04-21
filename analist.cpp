#include "analist.h"
#include <qmath.h>

int cont = 0;

Analist::Analist()
{
    oldSetPoint = 0; //sem isso o reset nao funciona
    for (int i = 0; i < 3; i++){
        porcInital[i] = i*5/100.0;
        porcFinal[i] = (100 - i*5)/100.0;
        //qDebug() << "P[" << i << "]: " << porcFinal[i];
    }

    reset(-1);
}

void Analist::calc(double nivel, double setPoint, double timeStamp)
{
    //qDebug() << "SP: " << setPoint << "OSP: " << oldSetPoint;
    reset(setPoint);
    calcMpTp(nivel, setPoint, timeStamp);
    calcTs(nivel,  setPoint, timeStamp);
    calcTr(nivel, setPoint, timeStamp);
    cont++;
}

double Analist::getMp()
{
    return mp;
}

double Analist::getMpPerc()
{
    return mpPerc;
}

double Analist::getTp()
{
    return tp;
}

double Analist::getTs(int i)
{
    return ts[i];
}

double Analist::getTr(int i)
{
    return tr[i];
}

void Analist::reset(double setPoint)
{
    // detecta mudanca de setPoint para iniciar o calculo das variaveis
    if(setPoint != oldSetPoint) {
        //qDebug() << "reset";
        //qDebug() << "Entrou Reset";
        oldSetPoint = setPoint;
        initialLevel = -1;
        InitialLevelTs = -1;
        mp = -100;
        tp = 0;
        tsOldTime = 0;
        tsInitialTime = -1;
    }
}


void Analist::calcMpTp(double nivel, double setPoint, double timeStamp)
{
    //qDebug() << nivel << setPoint << mp;
    if(mp == -100) {
        if(nivel > setPoint) direction = false; // Esta descendo
        else direction = true;
        mp = 0;
        mpPerc = 0;
        //qDebug() << "Entrou MP";
        //mpInitialTime = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        mpInitialTime = timeStamp;
    }
    if((direction && nivel-setPoint > mp) || (!direction && nivel-setPoint < mp)){
        mp = abs(nivel-setPoint);
        mpPerc = mp/(setPoint - initialLevel)*100;
        //qDebug() << mp;
        //tp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - mpInitialTime;
        tp = timeStamp - mpInitialTime;
    }
}

void Analist::calcTs(double nivel, double setPoint, double timeStamp)
{
    if(InitialLevelTs == -1) {
        InitialLevelTs = nivel;
        //tsInitialTime = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        //tsInitialTime = timeStamp;
        for(int i = 0; i < 4; i++)
            ts[i] = 0;
    }

    else if (nivel != InitialLevelTs && tsInitialTime == -1){
        tsInitialTime = timeStamp;
    }

    //double currentTimeTs=QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

    for (int i = 0; i < 4; i++){
        //qDebug() << currentTimeTs << tsInitialTime << currentTimeTs-tsInitialTime;
        if (qFabs((setPoint-InitialLevelTs)*porcTs[i]) < qFabs(nivel-setPoint))
            ts[i] = timeStamp - tsInitialTime;
            //qDebug() << "ts: 108 - " << cont;
    }
}

void Analist::calcTr(double nivel, double setPoint, double timeStamp)
{
    //double timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    //if (tr[0] == 0) trOldTime[0] = timeStamp; //botar no reset
    //qDebug() <<  "SP: " << setPoint << " - IL: " << initialLevel <<  " - L: " << nivel <<" - tr: " << tr[0] << " - trOld: " << trOldTime[0] << " - TS: " << timeStamp << "- Flag: " << reachedTr;
    //qDebug() << "condInic: " << (nivel <= (setPoint - initialLevel)*porcInital[0] + initialLevel) << "condFim: " << (nivel <= (setPoint - initialLevel)*porcFinal[0] + initialLevel);
    if (initialLevel == -1) {
        initialLevel = nivel;
        for (int i = 0; i < 3; i++) {
            trOldTime[i] = 0;
            tr[i] = 0;
            reachedTr[i] = false;
        }

        //qDebug() << "PI: " << porcInital[0] << " - PF: " << porcFinal[0];
    }
    for (int i = 0; i < 3; i++){
        if (reachedTr[i]);
            //reachedTr = true;
            //qDebug() << "Entrou" << reachedTr;
        else if (direction){
            if (nivel <= (setPoint - initialLevel)*porcInital[i] + initialLevel){
                trOldTime[i] = timeStamp;
                //qDebug() << "tr[" << i << "] 135: " << cont;
            }
            else if (nivel <= (setPoint - initialLevel)*porcFinal[i] + initialLevel){
                tr[i] = timeStamp - trOldTime[i];
                //qDebug() << "tr[" << i << "] 139: " << cont;
            }
            else {
                reachedTr[i] = true;
                //qDebug() << "tr[" << i << "] 143: " << cont;
            }
        }
        else {
            if (nivel >= initialLevel - (initialLevel-setPoint)*porcInital[i]){
                trOldTime[i] = timeStamp;
                //qDebug() << "tr[" << i << "] 148: " << cont;
            }
            else if (nivel >= initialLevel - (initialLevel-setPoint)*porcFinal[i]){
                tr[i] = timeStamp - trOldTime[i];
                //qDebug() << "tr[" << i << "] 152: " << cont;
            }
            else
                reachedTr[i] = true;
        }
        //qDebug() << "tr: " << tr[i];
    }
}

