#include "commthread.h"
#include <QtCore>

commThread::commThread(QObject *parent):
    QThread(parent)
{


}

void commThread::run(){

    for(int i=0;i<1000;i++){
        emit waterLevelRead(i*0.25,i*0.5); //Envia sinal do nível dos tanques para ser lido por um slot do supervisório
        this->msleep(100);
    }
}

void commThread::OnOutputVoltageChanged(double voltageTank1, double voltageTank2){
    //Recebe o sinal do supervisório nível de tensão a ser enviado para os tanques

}
