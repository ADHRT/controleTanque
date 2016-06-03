#include "commthread.h"

using std::complex;
using arma::mat;
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
    //IP para o quanser
    string IPs = "10.13.99.69";
    char * IP3 = new char[IPs.size() + 1];
    copy(IPs.begin(), IPs.end(), IP3);
    IP3[IPs.size()] = '\0';
    q = new Quanser(IP3, 20081);
    period = 0.1;
    tank = 1; // tanque2

    contMestre.diferencaSaida = 0;
    contMestre.lastI = 0;
    contMestre.lastD = 0;
    contEscravo.diferencaSaida = 0;
    contEscravo.lastI = 0;
    contEscravo.lastD = 0;

    polesOb[0] = complex<double>(0.99876,0);
    polesOb[1] = complex<double>(0.0999273,0);

    // Matrizes do sistema
    G = mat("0.993458148011545 0; 0.006520407260849 0.993458148011545");
    H = mat("0.021196129223099; 0.000069482650830");
    C = mat("0 1");
    invWo = mat(" -152.3613646001405 153.3646534633584; 1 0");
    L = mat("0.8; 0.9");

    // Valores estimados
    xEst = mat(2, 1, arma::fill::zeros);
    erroEst = mat(2, 1, arma::fill::zeros);

    //Seguidor
    Ga = mat("0.993458148011545 0 0.021196129223099; 0.006520407260849 0.993458148011545 0.000069482650830; 0 0 0");
    Ha = mat("0; 0; 1");
    invWc = mat("0 0 1; 70.741834006738 -7188.169264136371 0; -23.718572056820 7235.502852861834 0");
    kMatAux = mat("-0.5010938890433 151.8619115450547 1.0; 0 -1.0 1.0; 47.0237705882343 46.8697910620831 0.3086342756075");
    k2 = mat("0 0");
    v = 0;
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

            //malha aberta
            if (malha)
            {
                contMestre.sinalCalculado = sinalDaOndaGerada;
                contEscravo.sinalCalculado = contMestre.sinalCalculado;
                contEscravo.sinalSaturado = lockSignal(contEscravo.sinalCalculado, nivelTanque1, nivelTanque2);
            }

            else {//malha fechada

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

                if(follower) {
                    calculoDeControleSeguidor(nivelTanque1, nivelTanque2, contMestre.erro);
                }
                else {
                    calculoDeControle(&contMestre, nivelTanque, nivelTanque1, nivelTanque2);
                    qDebug();
                    if(cascade){
                        contEscravo.setPoint = contMestre.sinalCalculado;
                        contEscravo.erro = contEscravo.setPoint - nivelTanque1;

                        calculoDeControle(&contEscravo, nivelTanque,nivelTanque1,nivelTanque2);
                    }
                    else{
                        contEscravo.sinalSaturado=contMestre.sinalSaturado;
                    }
                    if(observer) calcObs(nivelTanque1, nivelTanque2, contEscravo.sinalSaturado*3);
                }
            }
//            kf() << "SinalSaturado: " << contEscravo.sinalSaturado;
            // Escreve no canal selecionado
            if(!simulationMode) {
                //qDebug() << "sinalSaturado: " << sinalSaturado << "\n";
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
            emit plotValues(timeStamp, contMestre.sinalCalculado, contEscravo.sinalCalculado, contEscravo.sinalSaturado, nivelTanque1, nivelTanque2, contMestre.setPoint, contMestre.erro, contMestre.i, contEscravo.i, contMestre.d, contEscravo.d, xEst[0], xEst[1], erroEst[0], erroEst[1]);
        }
        //qDebug() << "Kp:" << contMestre.kp << "Windup" << contMestre.windup << "controlador:" <<  control[0] << "seguidor" << follower << "Observador" << observer;
    }

    if(!simulationMode) {
        q->writeDA(channel, 0);
        //delete q;
    }
}

void commThread::calcObs(double nivelTanque1, double nivelTanque2, double sinalSaturado)
{
    double x_temp[2] = {nivelTanque1, nivelTanque2};
    mat x(x_temp, 2, 1);

    xEst = G*xEst + L*(nivelTanque2 - xEst[1]) + H*sinalSaturado;

    erroEst = x - xEst;
}

void commThread::zerarObs()
{
    this->xEst = mat(2, 1, arma::fill::zeros);
    this->erroEst = mat(2, 1, arma::fill::zeros);
}

void commThread::getPoles(double *l, complex<double> *pole)
{
    double l_temp[2] = {l[0], l[1]};
    mat L_temp(l_temp, 2, 1);

    mat temp = G - L_temp*C;

    arma::cx_vec eigVal = eig_gen(temp);

    pole[0] = eigVal[0];
    pole[1] = eigVal[1];
}

void commThread::getL(complex<double> *pole, double *l)
{
    double coef1 = -pole[0].real() - pole[1].real();
    complex <double>coef2 = pole[0] * pole[1];

    mat A = arma::eye<mat> (2,2);
    double l_aux[2] = {0, 1};
    mat l_array(l_aux, 2, 1);

    mat q = G*G + coef1*G + coef2.real()*A;

    mat l_mat = q*invWo*l_array;

    l[0] = l_mat[0];
    l[1] = l_mat[1];
}

void commThread::calculoDeControleSeguidor(double nivelTanque1, double nivelTanque2, double erro)
{
    v += erro;
    contEscravo.sinalCalculado = -(k2[0]*nivelTanque1 + k2[1]*nivelTanque2) + k1*v;
    contMestre.sinalCalculado = contEscravo.sinalCalculado;

    contEscravo.sinalSaturado = lockSignal(contEscravo.sinalCalculado, nivelTanque1, nivelTanque2);
}

void commThread::getPolesSeg(double *kSeg, complex<double> *poleSeg)
{
    bool flagPole[2];
    flagPole[0] = false; flagPole[1] = false;

    mat vec = ("0 0 1");
    double kTemp[3] = {kSeg[0], kSeg[1], kSeg[2]};
    mat kSegT(kTemp, 1, 3);

    mat ka = kSegT*inv(kMatAux) - vec;

    mat sist = Ga - Ha*ka;

    arma::cx_vec eigVal = eig_gen(sist);

    for (int i = 0; i < 3; i++){
        if (eigVal[i].imag() == 0 && !flagPole[1]){
            flagPole[1] = true;
            poleSeg[2] = eigVal[i];
        }
        else if (!flagPole[0]){
            poleSeg[0] = eigVal[i];
            flagPole[0] = true;
        }
        else {
            poleSeg[1] = eigVal[i];
        }
    }
}

void commThread::getK(complex<double> *poleSeg, double *kSeg)
{
    double a = poleSeg[0].real();
    double b = poleSeg[0].imag();
    double c = poleSeg[1].real();
    double d = poleSeg[2].real();

    double coef1, coef2, coef3;

    if(b != 0){
        coef1 = -2*a-d;
        coef2 = pow(a,2) + 2*a*d + pow(b,2);
        coef3 = -d*(pow(a,2) + pow(b,2));
    }
    else {
        coef1 = -(a + c + d);
        coef2 = a*c + d*(a + c);
        coef3 = -a*c*d;
    }

    mat I = arma::eye<mat> (3,3);
    mat vec = ("0 0 1");

    mat q = Ga*Ga*Ga + coef1*Ga*Ga + coef2*Ga + coef3*I;

    mat ka = vec*invWc*q;
    mat k = (ka + vec)*kMatAux;

    kSeg[0] = k[0];
    kSeg[1] = k[1];
    kSeg[2] = k[2];

//    qDebug() << q[0] << q[3] << q[6];
//    qDebug() << q[1] << q[4] << q[7];
//    qDebug() << q[2] << q[5] << q[8];
//    qDebug() << "Coef:" << coef1 << coef2 << coef3;
//    qDebug() << "K:" << kSeg[0] << kSeg[1] << kSeg[2];
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

void commThread::setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave, bool malha, int channel, int *control, double *kp, double *ki, double *kd, bool *windup, bool *conditionalIntegration, double *taw, int tank, bool cascade, bool observer, double *lOb, bool follower, double *kSeg)
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
    this->contMestre.erro = 0;

    this->contEscravo.kp = kp[1];
    this->contEscravo.ki = ki[1];
    this->contEscravo.kd = kd[1];
    this->contEscravo.taw = taw[1];
    this->contEscravo.windup = windup[1];
    this->contEscravo.conditionalIntegration = conditionalIntegration[1];
    this->contEscravo.erro = 0;

    this->tank = tank;
    this->cascade = cascade;

    //Observador
    this->L = mat(lOb, 2, 1);
    this->observer = observer;
    this->xEst = mat(2, 1, arma::fill::zeros);
    this->erroEst = mat(2, 1, arma::fill::zeros);

    //Seguidor
    this->follower = follower;
    this->k1 = kSeg[0];
    this->k2[0] = kSeg[1];
    this->k2[1] = kSeg[2];
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
    contMestre.p = 0;
    contMestre.i = 0;
    contMestre.d = 0;
    contMestre.lastI = 0;
    contMestre.lastD = 0;
    contMestre.diferencaSaida = 0;
    contMestre.sinalCalculado = 0;
    contMestre.erro = 0;

    contEscravo.kp = 2;
    contEscravo.ki = 0.05;
    contEscravo.kd = 0.005;
    contEscravo.p = 0;
    contEscravo.i = 0;
    contEscravo.d = 0;
    contEscravo.lastI = 0;
    contEscravo.lastD = 0;
    contEscravo.diferencaSaida = 0;
    contEscravo.sinalCalculado = 0;
    contEscravo.erro = 0;

    v = 0;


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
        c->p = 1, c->i = 0, c->d = 0;
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


