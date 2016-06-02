#ifndef COMMTHREAD_H
#define COMMTHREAD_H

#include <QThread>
#include "quanser.h"
#include <complex>
#include <QtCore>
#include <cmath>
#include <armadillo>

using std::complex;
using arma::mat;

struct Controlador {
  double p, i, d;
  double sinalCalculado, sinalSaturado;
  double setPoint, erro ,lastSinalCalculado;
  double kp, ki, kd, taw, lastI, lastD, diferencaSaida;
  bool windup, conditionalIntegration;
};

class commThread : public QThread
{
    Q_OBJECT
public:
    explicit commThread(QObject *parent =0);
    enum Control { P, PI, PD, PID, PI_D, SEM };
    void run();
    void setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave, bool malha, int channel, int *control, double *kp, double *ki, double *kd, bool *windup, bool *conditionalIntegration, double *taw, int tank, bool cascade, bool observer, double *lOb, bool follower, double *kSeg);
    void setNullParameters(void);
    void setSimulationMode(bool on);
    void disconnect(void);
    void terminate(void);
    void calculoDeControle(Controlador*, double,double,double);
    int start();
    //Observador
    void getPoles(double *l, complex<double> *pole);
    void getL(complex<double> *pole, double *l);
    void zerarObs();
    //Seguidor
    void getPolesSeg(double *kSeg, complex<double> *poleSeg);
    void getK(complex<double> *poleSeg, double *kSeg);

private:
    Controlador contMestre, contEscravo;
    typedef QThread super;
    double frequencia;
    double amplitude;
    double offset;
    double duracaoMax;
    double duracaoMin;
    double sinalDaOndaGerada, sinalSaturadoDaOndaGerada, lastTimeStamp, timeToNextRandomNumber, lastSinalCalculado;
    //variavel aux para anti-windup
    double lastLoopTimeStamp;
    int wave;
    Control control[2], lastControl[2];
    double period;
    bool malha, cascade;
    bool simulationMode;
    bool levelSimulationFinished;
    bool connected;
    //flag para ligar o observador
    bool observer, follower;
    int channel, tank;
    int simulationNivelTanque1;
    double waveTime, waveTimeStamp;
    Quanser* q;

    // Entrada do usuario ou calculado pelo auto-valores
    complex<double> polesOb[2];

    // Polos seguidor
    complex<double> polesSeg[3];

    // Matrizes para o calculo do observador
    mat G;
    mat H;
    mat C;
    mat invWo;
    mat L;

    //Valores estimados
    mat xEst;
    mat erroEst;

    //Matrizes para o calculo do seguidor
    mat Ga;
    mat Ha;
    mat invWc;
    mat K;
    mat Ka;
    double k1;
    mat k2;
    mat kMatAux; //Matriz auxiliar para o calculo de k1 e k2
    double v;

    // Travas
    double lockSignal(double sinalCalculado, double nivelTanque1, double nivelTanque2);

    // Observador
    void calcObs(double nivelTanque1, double nivelTanque2, double sinalSaturado);

    // Seguidor
    void calcSeg();
    void calculoDeControleSeguidor(double nivelTanque1, double nivelTanque2, double erro);

signals:
    void plotValues(double timeStamp, double sinalCalculadoMestre, double sinalCalculadoEscravo, double sinalSaturado, double nivelTanque1, double nivelTanque2, double setPoint, double erro, double iMestre, double iEscravo, double dMestre, double dEscravo, double nivelTanque1Est, double nivelTanque2Est, double erroEst1, double erroEst2);

};
#endif // COMMTHREAD_H
