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
    void setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave, bool malha, int channel, int *control, double *kp, double *ki, double *kd, bool *windup, bool *conditionalIntegration, double *taw, int tank, bool cascade, bool observador, double *lOb);
    void setNullParameters(void);
    void setSimulationMode(bool on);
    void disconnect(void);
    void terminate(void);
    void calculoDeControle(Controlador*, double,double,double);
    int start();
    void getPoles(double *l, complex<double> *pole);
    void getL(complex<double> *pole, double *l);

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
    int channel, tank;
    int simulationNivelTanque1;
    double waveTime, waveTimeStamp;
    Quanser* q;

    // Entrada do usuario ou calculado pelo auto-valores
    complex<double> polesOb[2];

    // Matrizes para o calculo do observador
    mat G;
    mat H;
    mat C;
    mat Wo;
    mat L;

    //Valores estimados
    mat xEst;
    mat erroEst;
    mat yEst;

    // Travas
    double lockSignal(double sinalCalculado, double nivelTanque1, double nivelTanque2);

    //Variáveis para observador de estados
    double L1_dot_const1, L1_dot_const2, L2_dot_const1, L2_dot_const2;

    void calcObs();
    void calcPoles();
    void calcEstimated(double nivelTanque1, double nivelTanque2, double sinalSaturado);

signals:
    void plotValues(double, double, double, double, double, double, double, double, double, double, double, double);



};
#endif // COMMTHREAD_H
