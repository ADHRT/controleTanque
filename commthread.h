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
    void setParameters(double, double, double, double, double, int, bool, int, int, double, double, double, bool, bool, double, int tank);
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
    volatile double sinalCalculado, lastTimeStamp, timeToNextRandomNumber, lastSinalCalculado;
    //variavel aux para anti-windup
    volatile double diferencaSaida;
    volatile double lastLoopTimeStamp;
    volatile int wave;
    Control control, lastControl;
    volatile double kp, ki, kd, taw, lastI, lastD, period;
    volatile bool malha, windup, conditionalIntegration;
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
