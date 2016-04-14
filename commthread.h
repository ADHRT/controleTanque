#ifndef COMMTHREAD_H
#define COMMTHREAD_H

#include <QThread>
#include "quanser.h"

class commThread : public QThread
{
    Q_OBJECT
public:
    explicit commThread(QObject *parent =0);
    enum Control { P, PI, PD, PID, PI_D, SEM };
    void run();
    void setParameters(double frequencia, double amplitude, double offset , double duracaoMax, double duracaoMin, int wave, bool malha, int channel, int *control, double *kp, double *ki, double *kd, bool *windup, bool *conditionalIntegration, double *taw, int tank, bool cascade);
    void setNullParameters(void);
    void setSimulationMode(bool on);
    void disconnect(void);
    void terminate(void);
    int start();

private:
    typedef QThread super;
    volatile double frequencia;
    volatile double amplitude;
    volatile double offset;
    volatile double duracaoMax;
    volatile double duracaoMin;
    volatile double sinalCalculado[2], lastTimeStamp, timeToNextRandomNumber, lastSinalCalculado;
    //variavel aux para anti-windup
    volatile double diferencaSaida[2];
    volatile double lastLoopTimeStamp;
    volatile int wave;
    Control control[2], lastControl[2];
    volatile double kp[2], ki[2], kd[2], taw[2], lastI[2], lastD[2], period;
    volatile bool malha, windup[2], conditionalIntegration[2], cascade;
    volatile bool simulationMode;
    volatile bool levelSimulationFinished;
    volatile bool connected;
    volatile int channel, tank;
    volatile int simulationNivelTanque1;
    volatile double waveTime, waveTimeStamp;
    Quanser* q;
    double lockSignal(double sinalCalculado, double nivelTanque1, double nivelTanque2);

signals:
    void plotValues(double,double,double,double,double,double,double,double,double);



};
#endif // COMMTHREAD_H
