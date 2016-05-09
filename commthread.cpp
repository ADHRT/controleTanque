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
    char ip[] = "10.13.99.69";
    q = new Quanser(ip, 20081);
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


    //Inicializando variáveis do observador de estados;
    //double A1 = 15.5179;//A2=A1
    //double L10 = 15;//L2=L1
    //double a1 = 0.17813919765;//a2=a1
    //double Km = 3.3;
    //double g = 9.8066;

    //Note que L1_dot=L1_dot_const1*L1+L1_dot_const1*Vp
    //L1_dot_const1 = -a1/A1*sqrt(g/(2*L10));
    //L1_dot_const2 = Km/A1;

    //Note que L2_dot=L2_dot_const1*L2+L2_dot_const2*L1
    //L2_dot_const1 = L1_dot_const1;
    //L2_dot_const2 = -L2_dot_const1;

    // Carregando valores das matrizes nos vetores temp para uso posterior no armadilo
    double g_temp[4] = {0.999343880971910, 0.000655903734910, 0, 0.999343880971910};
    //double g_temp[4] = {1.59, 6.481, -3.8894, 2};
    //double h_temp[2] = {0.021258786853757, 0.000006975673074};
    double h_temp[2] = {0.0269, 0.0000963};
    double c_temp[2] = {0, 1};
    //double c_temp[2] = {0, 1};
    double wo_temp[4] = {0, 0.000655903734910, 1, 0.999343880971910};
    //double wo_temp[4] = {0, 6.481, 1, 2};
    double l_temp[2] = {0.8, 0.9};
    //double l_temp[2] = {-3.7045, 2.59};

    // Matrizes do sistema
    G = mat(g_temp, 2, 2);
    H = mat(h_temp, 2, 1);
    C = mat(c_temp, 1, 2);
    Wo = mat(wo_temp, 2, 2);
    L = mat(l_temp, 2, 1);

    // Valores estimados
    xEst = mat(2, 1, arma::fill::zeros);
    erroEst = mat(2, 1, arma::fill::zeros);
    yEst = mat(1, 1, arma::fill::zeros);
}

void commThread::run(){

    //calcObs();
    //calcPoles();

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

                calculoDeControle(&contMestre, nivelTanque, nivelTanque1, nivelTanque2);

                if(cascade){
                    contEscravo.setPoint = contMestre.sinalCalculado;
                    contEscravo.erro = contEscravo.setPoint - nivelTanque1;

                    calculoDeControle(&contEscravo, nivelTanque,nivelTanque1,nivelTanque2);
                }
                else{
                    contEscravo.sinalSaturado=contMestre.sinalSaturado;
                }
                //calcObs();
                calcEstimated(nivelTanque1, nivelTanque2, contEscravo.sinalSaturado);
            }

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
            emit plotValues(timeStamp, contMestre.sinalCalculado, contEscravo.sinalCalculado, contEscravo.sinalSaturado, nivelTanque1, nivelTanque2, contMestre.setPoint, contMestre.erro, contMestre.i, contEscravo.i, contMestre.d, contEscravo.d);
        }
    }
    if(!simulationMode) {
        q->writeDA(channel, 0);
        //delete q;
    }
}

void commThread::calcObs(void){
    double coef1 =  -polesOb[0].real() - polesOb[1].real();
    complex <double>coef2 = polesOb[0] * polesOb[1];

    mat A = arma::eye<mat> (2,2);
    double l_aux[2] = {0, 1};
    mat l_array(l_aux, 2, 1);

    mat q = G*G + coef1*G + coef2.real()*A;

    L = q*inv(Wo)*l_array;

//    qDebug() << "coef1: " << coef1 << "coef2: " << coef2.real();
//    qDebug() << "G: " << G[0] << G[1] << G[2] << G[3];
//    qDebug() << "Q: " << q[0] << q[1] << q[2] << q[3];
//    qDebug() << "L: " << L[0] << L[1];
}

void commThread::calcPoles()
{
    mat temp = G - L*C;

    arma::cx_vec eigVal = eig_gen(temp);
    //qDebug() << "autovalores: " << eigVal[0].real() << eigVal[0].imag() << eigVal[1].real() << eigVal[1].imag();

    polesOb[0] = eigVal[0];
    polesOb[1] = eigVal[1];
}

void commThread::calcEstimated(double nivelTanque1, double nivelTanque2, double sinalSaturado)
{
    double x_temp[2] = {nivelTanque1, nivelTanque2};
    mat x(x_temp, 2, 1);

    //qDebug() << "Sinal Saturado: " << sinalSaturado;

    erroEst = x - xEst;
    erroEst = (G - L*C)*erroEst;

    mat xEst_temp = G*xEst + L*(nivelTanque2 - yEst) + H*sinalSaturado;
    yEst = C*xEst;
    xEst = xEst_temp;

    //qDebug() << "L: " << L[0] << L[1];
    //qDebug() << "Erro: " << erroEst[0] << erroEst[1];
    qDebug() << "Nivel 1: " << x[0] << xEst[0];
    qDebug() << "Nivel 2: " << x[1] << xEst[1];
    //qDebug() << "Sinal Saturado: " << sinalSaturado;
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

void commThread::setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave, bool malha, int channel, int *control, double *kp, double *ki, double *kd, bool *windup, bool *conditionalIntegration, double *taw, int tank, bool cascade, bool observer, double *lOb)
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

    //TAYNARA receber observador e lob
    this->L = mat(lOb, 2, 1);
    this->observer = observer;
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

void commThread::getPoles(double *l, complex<double> *pole)
{
    double l_temp[2] = {l[0], l[1]};
    mat L_temp(l_temp, 2, 1);

    mat temp = G - L_temp*C;

    arma::cx_vec eigVal = eig_gen(temp);
    //qDebug() << "autovalores: " << eigVal[0].real() << eigVal[0].imag() << eigVal[1].real() << eigVal[1].imag();

    pole[0] = eigVal[0];
    pole[1] = eigVal[1];
}

void commThread::getL(complex<double> *pole, double *l)
{
    double coef1 = -pole[0].real() - pole[1].real();
    //qDebug() << "Polos getL: " << pole[0].real() << pole[1].real();
    complex <double>coef2 = pole[0] * pole[1];

    mat A = arma::eye<mat> (2,2);
    double l_aux[2] = {0, 1};
    mat l_array(l_aux, 2, 1);

    mat q = G*G + coef1*G + coef2.real()*A;

    mat l_mat = q*inv(Wo)*l_array;

    l[0] = l_mat[0];
    l[1] = l_mat[1];
}


