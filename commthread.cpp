#include "commthread.h"
#include <QtCore>

commThread::commThread(QObject *parent):
    QThread(parent)
{


}

void commThread::run(){

    // Conecta
    Quanser* q = new Quanser("10.13.97.69", 20072);

    while(1) {

        // Le
        double nivelTanque1 = q->readAD(0) * 6.25;
        double nivelTanque2 = q->readAD(1) * 6.25;


        // Calcula

        // Get timeStamp
        double timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

        //Calculates new points
        //double nivelTanque1 = qSin(timeStamp)*5+5;
        //double nivelTanque2 = qCos(timeStamp)*5+5;

        switch(wave)
        {
        case 0://degrau:
            sinalCalculado = amplitude + offset;
            break;
        case 1://senoidal:
            sinalCalculado = qSin(timeStamp*3.14159265359*frequencia)*amplitude+offset;
            break;
        case 2://quadrada:
            sinalCalculado = qSin(timeStamp*3.14159265359*frequencia)*amplitude+offset;
            if(sinalCalculado>0)sinalCalculado = amplitude;
            else sinalCalculado = -amplitude;
            break;
        case 3://serra:
            sinalCalculado = (fmod((timeStamp*3.14159265359*frequencia), (2*3.14159265359))/(2*3.14159265359))*amplitude*2-amplitude+offset;
            break;
        case 4://aleatorio:
            if((timeStamp-lastTimeStamp)>timeToNextRandomNumber){
                sinalCalculado = (double)rand()/RAND_MAX * amplitude * 2 - amplitude + offset;
                lastTimeStamp=timeStamp;
                timeToNextRandomNumber= ((double)rand()/RAND_MAX) * (duracaoMax-duracaoMin) + duracaoMin;
                if (timeToNextRandomNumber>duracaoMax)timeToNextRandomNumber=duracaoMax;//Isso não deveria acontecer
            }
            break;
        default:
            qDebug() << "ERRO: Nenhuma onda selecionada!";
        }

        //Calculates other points
        double sinalSaturado = commThread::lockSignal(sinalCalculado,nivelTanque1);
        double setPoint = qSin(timeStamp*0.5+1);
        double erro = nivelTanque1-setPoint;

        // Dorme
        this->msleep(100);

        // Escreve no canal 0
        q->writeDA(0, sinalSaturado);

        // Envia valores para o supervisorio
        emit plotValues(timeStamp, sinalCalculado, sinalSaturado, nivelTanque1, nivelTanque2, setPoint, erro);
    }
    /*for(int i=0;i<1000;i++){
        emit waterLevelRead(i*0.25,i*0.5); //Envia sinal do nível dos tanques para ser lido por um slot do supervisório
        this->msleep(100);
    }*/
    /* Aqui deve ser lido o tempo a todo instante e, somente quando decorrido 0.1s, a comunicação acontece
     * Esse 'for loop' acima é apenas para demonstrar o funcionamento da comunicação entre thread e supervisório */
}

double commThread::lockSignal(double sinalCalculado, double nivelTanque1){

    double sinalSaturado=sinalCalculado;

    //Trava 1
    if(sinalCalculado>4) sinalSaturado=4;
    else if(sinalCalculado<-4) sinalSaturado=-4;

    //Trava 2
    if(nivelTanque1<3 && sinalCalculado<0) sinalSaturado=0;

    //Trava 3
    if(nivelTanque1>28 && sinalCalculado>3.25) sinalSaturado=3.25;

    //Trava 4
    if(nivelTanque1>29 && sinalCalculado>0) sinalSaturado=0;

    return sinalSaturado;
}

void commThread::setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave)
{
    this->frequencia = frequencia;
    this->amplitude = amplitude;
    this->offset = offset;
    this->duracaoMax = duracaoMax;
    this->duracaoMin = duracaoMin;
    this->wave = wave;
}

