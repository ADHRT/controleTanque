#include "commthread.h"
#include <QtCore>
#include<cmath>

commThread::commThread(QObject *parent):
    QThread(parent)
{
    //Quadrada
    wave = 0;
    simulationMode = false;
    channel = 0;
    waveTime = 0;
    kp = 2;
    ki = 0.05;
    kd = 0.005;
    control = P;
    lastControl = control;
    q = new Quanser("10.13.99.69", 20081);
    lastI = 0;
    lastD = 0;
    period = 0.1;
    windup = true;
    diferencaSaida = 0;
}

void commThread::run(){

    double nivelTanque1 = 0, nivelTanque2 = 0, timeStamp, setPoint = 0, sinalSaturado, erro = 0, i, d, p;

    //Inicia a contagem de tempo
    lastLoopTimeStamp=QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    waveTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    //Mantem o loop ate a funcao disconnect ser chamada
    connected = true;
    while(connected) {

        //Reads Time
        waveTime = timeStamp - waveTimeStamp;
        timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

        if(timeStamp-lastLoopTimeStamp > period || !connected){
            lastLoopTimeStamp=timeStamp;

            if(!simulationMode) {
                // Le
                nivelTanque1 = q->readAD(0) * 6.25;
                if (nivelTanque1 < 0) nivelTanque1 = 0;
                nivelTanque2 = q->readAD(1) * 6.25;
                if (nivelTanque2 < 0) nivelTanque2 = 0;
            } else {
                //Calculates new points
                nivelTanque1 = qSin(timeStamp)*5+5;
                nivelTanque2 = qCos(timeStamp)*5+5;
            }

            switch(wave)
            {
            case 0://degrau:
                sinalCalculado = offset;
                break;
            case 1://senoidal:
                sinalCalculado = qSin((waveTime)*3.14159265359*frequencia)*amplitude+offset;
                break;
            case 2://quadrada:
                sinalCalculado = qSin(waveTime*3.14159265359*frequencia)*amplitude;
                if(sinalCalculado>0)sinalCalculado = amplitude+offset;
                else sinalCalculado = -amplitude+offset;
                break;
            case 3://serra:
                sinalCalculado = (fmod((waveTime*3.14159265359*frequencia), (2*3.14159265359))/(2*3.14159265359))*amplitude*2-amplitude+offset;
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
            sinalSaturado = commThread::lockSignal(sinalCalculado, nivelTanque1, nivelTanque2);



            if(malha == false){//malha fechada
                setPoint = sinalCalculado;
                enum Control { P, PI, PD, PID, PI_D, SEM };
                erro = setPoint - nivelTanque1;
                //p = 0, i = 0, d = 0;
                //kp=2 ki=0.05 kd=0.005
                //Se houver mudanca de controlador zera o lastI e lastD
                if(control != lastControl) {
                    lastI = 0;
                    lastD = 0;
                    lastControl = control;
                }
                //p
                p = kp*erro;
                //i
                i = lastI + (ki*period*erro);
                //i = lastI + (ki*period*erro) + (diferencaSaida)/sqrt(kd/ki));
                //i = lastI + ((ki+(diferencaSaida)/sqrt(kd/ki))*period*erro);
                //i = lastI + ((ki*erro) + (diferencaSaida)/sqrt(kd/ki))*period;

                if(windup && abs(erro)<2) {
                    //qDebug() << "SS: " << sinalSaturado << "erro: " << erro << "| difout: " << diferencaSaida;
                    //qDebug() << "ss = " << (sinalSaturado-erro) << " | kd/ki = " << sqrt(kd/ki) << "| wd = " << (sinalSaturado - sinalCalculado)/sqrt(kd/ki);
                    i += ((diferencaSaida)/sqrt(kd/ki))*period;
                    //qDebug() << "i: " << i << "LastI: " << lastI    ;
                    qDebug() << "antes: " << lastI + (ki*period*erro)  << "depois: " << ((diferencaSaida)/sqrt(kd/ki))*period   ;
                } else if (conditionalIntegration && sinalSaturado != erro) {
                    i = lastI;

                }
                lastI = i;
                //d
                d = kd*(erro - lastD)/period;

                switch (control) {
                case SEM:
                    p = 0, i = 0, d = 0;
                    break;
                case P:
                    i = 0, d = 0;
                    break;
                case PI:
                    d = 0;
                    break;
                case PD:
                    lastD = erro;
                    i = 0;
                    break;
                case PID:
                    lastD = erro;
                    break;
                case PI_D:
                    d = kd*(nivelTanque1 - lastD)/period;
                    lastD = nivelTanque1;
                    break;
                default:
                    qDebug() << "Nenhum sinal de controle selecionado";
                }
                erro = p + i + d;
                sinalCalculado = erro;
                sinalSaturado = commThread::lockSignal(erro, nivelTanque1, nivelTanque2);
            }


            // Escreve no canal selecionado
            if(!simulationMode)
                q->writeDA(channel, sinalSaturado);

            diferencaSaida = sinalSaturado - sinalCalculado;
            // Envia valores para o supervisorio
            emit plotValues(timeStamp, sinalCalculado, sinalSaturado, nivelTanque1, nivelTanque2, setPoint, setPoint - nivelTanque1, i, d);
        }
    }
    if(!simulationMode) {
        q->writeDA(channel, 0);
        delete q;
    }
}

double commThread::lockSignal(double sinalCalculado, double nivelTanque1, double nivelTanque2){

    double sinalSaturado=sinalCalculado;


    //Trava 1
    if(sinalCalculado>4) sinalSaturado=4;
    else if(sinalCalculado<-4) sinalSaturado=-4;

    //Trava 2
    if(nivelTanque1<8 && sinalCalculado<0) sinalSaturado=0;

    //Trava 3
    //if(nivelTanque1>28 && sinalCalculado>3.25) sinalSaturado=3.25;
    // valor impiricamente calculuado  = 2.97
    if(nivelTanque1>28 && sinalCalculado>2.97) sinalSaturado=2.97;

    //Trava 4
    if(nivelTanque1>29 && sinalCalculado>0) sinalSaturado=0;

    //Trava 5
    if(nivelTanque2 > 26 && nivelTanque1 > 8) sinalSaturado = -4;
    else if (nivelTanque2 > 26) sinalSaturado = 0;


    return sinalSaturado;
}

void commThread::setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave, bool malha, int channel, int control, double kp, double ki, double kd, bool windup, bool conditionalIntegration)
{
    this->frequencia = frequencia;
    this->amplitude = amplitude;
    this->offset = offset;
    this->duracaoMax = duracaoMax;
    this->duracaoMin = duracaoMin;
    if (wave != this->wave)
    {
        waveTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        waveTime = 0;
    }
    this->wave = wave;
    this->malha = malha;
    this->channel = channel;
    this->control = static_cast<Control>(control);
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
    this->windup = windup;
    this->conditionalIntegration = conditionalIntegration;
}

// Zera todos os valores
void commThread::setNullParameters()
{
    frequencia = 0;
    amplitude = 0;
    offset = 0;
    duracaoMax = 0;
    duracaoMin = 0;
    sinalCalculado = 0;
    kp = 0;
    ki = 0;
    kd = 0;
}

void commThread::setSimulationMode(bool on)
{
    simulationMode = on;
}

void commThread::disconnect(void)
{
    connected = false;
}

void commThread::terminate(void)
{
    //setNullParameters();
    disconnect();
    QThread::msleep(100);
    super::terminate();
    //while(!super::isFinished());
}

int commThread::start(void)
{
    // Conecta com os tanques
    int erro = 0;
    if(!simulationMode) {
        erro = q->connectServer();
    }
    if(!erro) super::start();
    return erro;
}


