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
    control[0] = P; control[1] = P;
    lastControl[0] = control[0]; lastControl[1] = control[1];
    q = new Quanser("10.13.99.69", 20081);
    period = 0.1;
    tank = 1; // tanque2

    contMestre.diferencaSaida = 0;
    contMestre.lastI = 0;
    contMestre.lastD = 0;
    contEscravo.diferencaSaida = 0;
    contEscravo.lastI = 0;
    contEscravo.lastD = 0;


}

void commThread::run(){

    double nivelTanque = 0, nivelTanque1 = 0, nivelTanque2 = 0, timeStamp;

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
            }


            // Passa o valor do tanque selecionado para PV
            nivelTanque = tank == 1?nivelTanque1:nivelTanque2;

            switch(wave)
            {
            case 0://degrau:
                //qDebug() << "Degrau";
                sinalDaOndaGerada = offset;
                break;
            case 1://senoidal:
                //qDebug() << "Senoidal";
                sinalDaOndaGerada = qSin((waveTime)*3.14159265359*frequencia)*amplitude+offset;
                break;
            case 2://quadrada:
                //qDebug() << "Quadrada";
                sinalDaOndaGerada = qSin(waveTime*3.14159265359*frequencia)*amplitude;
                if(sinalDaOndaGerada>0)sinalDaOndaGerada = amplitude+offset;
                else sinalDaOndaGerada = -amplitude+offset;
                break;
            case 3://serra:
                //qDebug() << "Serra";
                sinalDaOndaGerada = (fmod((waveTime*3.14159265359*frequencia), (2*3.14159265359))/(2*3.14159265359))*amplitude*2-amplitude+offset;
                break;
            case 4://aleatorio:
                sinalDaOndaGerada = lastSinalCalculado;
                if((timeStamp-lastTimeStamp)>timeToNextRandomNumber){
                    sinalDaOndaGerada = (double)rand()/RAND_MAX * amplitude * 2 - amplitude + offset;
                    lastSinalCalculado=sinalDaOndaGerada;
                    //qDebug() << "Aleatorio";
                    lastTimeStamp = timeStamp;
                    timeToNextRandomNumber = ((double)rand()/RAND_MAX) * (duracaoMax-duracaoMin) + duracaoMin;
                    if (timeToNextRandomNumber>duracaoMax)timeToNextRandomNumber=duracaoMax;//Isso não deveria acontecer
                }
                break;
            default:
                qDebug() << "ERRO: Nenhuma onda selecionada!";
            }

            //qDebug() <<sinalDaOndaGerada;
            //Calculates other points
            //sinalSaturadoDaOndaGerada = commThread::lockSignal(sinalDaOndaGerada, nivelTanque1, nivelTanque2);



            if(malha == false){//malha fechada

                //Se houver mudanca de controlador zera o lastI e lastD
                if(control[0] != lastControl[0]) {
                    contMestre.lastI = 0;
                    contMestre.lastD = 0;
                    lastControl[0] = control[0];
                }

                //Se houver mudanca de controlador zera o lastI e lastD
                if(control[1] != lastControl[1]) {
                    contEscravo.lastI = 0;
                    contEscravo.lastD = 0;
                    lastControl[1] = control[1];
                }

                contMestre.setPoint = sinalDaOndaGerada;
                contMestre.erro = contMestre.setPoint - nivelTanque;

                calculoDeControle(&contMestre, nivelTanque,nivelTanque1,nivelTanque2);

                contEscravo.setPoint = contMestre.sinalCalculado;
                contEscravo.erro = contEscravo.setPoint - nivelTanque;

                calculoDeControle(&contEscravo, nivelTanque,nivelTanque1,nivelTanque2);
            }

            // Escreve no canal selecionado
            if(!simulationMode) {
//                qDebug() << "sinalSaturado: " << sinalSaturado << "\n";
                q->writeDA(channel, contEscravo.sinalSaturado);
            } else { //Simulacao

                //Nivel Tanque 1
                nivelTanque1 = nivelTanque1+(contEscravo.sinalSaturado)/10-0.01*nivelTanque1;
                if(nivelTanque1>30){nivelTanque1=30;}
                else if(nivelTanque1<0){nivelTanque1=0;}

                //Nivel Tanque 2
                nivelTanque2 = nivelTanque2+(nivelTanque1-nivelTanque2)/50-0.1;
                if(nivelTanque2>30){nivelTanque2=30;}
                else if(nivelTanque2<0){nivelTanque2=0;}
           }

            // Envia valores para o supervisorio
            emit plotValues(timeStamp, contMestre.sinalCalculado, contMestre.sinalSaturado, nivelTanque1, nivelTanque2, contMestre.setPoint, contMestre.erro, contMestre.i, contMestre.d);

        }
    }
    if(!simulationMode) {
        q->writeDA(channel, 0);
        //delete q;
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

void commThread::setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave, bool malha, int channel, int *control, double *kp, double *ki, double *kd, bool *windup, bool *conditionalIntegration, double *taw, int tank, bool cascade)
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
    for (int i = 0; i < 2; i++){
        this->control[i] = static_cast<Control>(control[i]);
    }
    this->contMestre.kp = kp[0];
    this->contMestre.ki = ki[0];
    this->contMestre.kd = kd[0];
    this->contMestre.taw = taw[0];
    this->contMestre.windup = windup[0];
    this->contMestre.conditionalIntegration = conditionalIntegration[0];

    this->contEscravo.kp = kp[1];
    this->contEscravo.ki = ki[1];
    this->contEscravo.kd = kd[1];
    this->contEscravo.taw = taw[1];
    this->contEscravo.windup = windup[1];
    this->contEscravo.conditionalIntegration = conditionalIntegration[1];

    this->tank = tank;
    this->cascade = cascade;
}

// Zera todos os valores
void commThread::setNullParameters()
{
    //frequencia = 0;
    //control = P;
    //tank = 1;
    amplitude = 0;
    offset = 0;
    duracaoMax = 0;
    duracaoMin = 0;

    contMestre.kp = 2;
    contMestre.ki = 0.05;
    contMestre.kd = 0.005;
    contMestre.lastI = 0;
    contMestre.lastD = 0;
    contMestre.diferencaSaida = 0;
    contMestre.sinalCalculado = 0;

    contEscravo.kp = 2;
    contEscravo.ki = 0.05;
    contEscravo.kd = 0.005;
    contEscravo.lastI = 0;
    contEscravo.lastD = 0;
    contEscravo.diferencaSaida = 0;
    contEscravo.sinalCalculado = 0;

    qDebug() << "setNullParametres()\n";
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

void commThread::calculoDeControle(Controlador *c, double nivelTanque, double nivelTanque1, double nivelTanque2)
{
    //Calculo do p
    c->p = c->kp*c->erro;

    //Calculo do i
    c->i = c->lastI + (c->ki*period*c->erro);

    //Calculo do d
    c->d = c->kd*(c->erro - c->lastD)/period;


    switch (control[0]) {
    case SEM:
        c->p = 0, c->i = 0, c->d = 0;
        break;
    case P:
        c->i = 0, c->d = 0;
        break;
    case PI:
        c->d = 0;
        break;
    case PD:
        c->i = 0;
        break;
    case PID:
        break;
    case PI_D:
        c->d = c->kd*(nivelTanque - c->lastD)/period;
        break;
    default:
        qDebug() << "Nenhum sinal de controle selecionado";
    }


    c->sinalCalculado = (c->p + c->i + c->d);
    c->sinalSaturado = commThread::lockSignal(c->sinalCalculado, nivelTanque1, nivelTanque2);
    c->diferencaSaida = c->sinalSaturado - c->sinalCalculado;

    // WINDUP FIX
    if(c->windup) {
        // Anti-windup
        c->i += (c->kp/c->taw)*period*c->diferencaSaida;
    } else if (c->conditionalIntegration && c->sinalSaturado != c->sinalCalculado) {
        // Integral Condicional
        c->i = c->lastI;
    }

    if (c->windup || c->conditionalIntegration) {
        c->sinalCalculado = (c->p + c->i + c->d);
        c->sinalSaturado = commThread::lockSignal(c->sinalCalculado, nivelTanque1, nivelTanque2);
        c->diferencaSaida = c->sinalSaturado - c->sinalCalculado;
    }

    //qDebug() << "(p,i,d) = (" << p << "," << i << "," << d << ")" << " taw:" << taw << " dif: " << diferencaSaida;
    c->lastD = c->d;
    c->lastI = c->i;

    c->diferencaSaida = c->sinalSaturado - c->sinalCalculado;


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


