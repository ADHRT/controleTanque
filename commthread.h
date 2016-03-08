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
    void setParameters(double, double, double, double, double, int, bool, int, int, double, double, double, bool, bool);
    void setNullParameters(void);
    void setSimulationMode(bool on);
    void disconnect(void);
    void terminate(void);
    int start();

private:
    typedef QThread super;
    double frequencia;
    double amplitude;
    double offset;
    double duracaoMax;
    double duracaoMin;
    double sinalCalculado, lastTimeStamp, timeToNextRandomNumber;
    //variavel aux para anti-windup
    double diferencaSaida;
    double lastLoopTimeStamp;
    int wave;
    Control control, lastControl;
    double kp, ki, kd, lastI, lastD, period;
    bool malha, windup, conditionalIntegration;
    bool simulationMode;
    bool connected;
    int channel;
    double waveTime, waveTimeStamp;
    Quanser* q;
    double lockSignal(double sinalCalculado, double nivelTanque1, double nivelTanque2);

signals:
    void plotValues(double,double,double,double,double,double,double,double,double);



};
#endif // COMMTHREAD_H
