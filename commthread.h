#ifndef COMMTHREAD_H
#define COMMTHREAD_H

#include <QThread>
#include "quanser.h"

class commThread : public QThread
{
    Q_OBJECT

private:
    double frequencia;
    double amplitude;
    double offset;
    double duracaoMax;
    double duracaoMin;
    int wave;
    bool malha;
    double sinalCalculado, lastTimeStamp, timeToNextRandomNumber;
    double lockSignal(double sinalCalculado, double nivelTanque1, double nivelTanque2);
    double lastLoopTimeStamp;
    bool simulationMode;
    int channel;

public:
    explicit commThread(QObject *parent =0);
    void run();
    void setParameters(double, double, double, double, double, int, bool, int);
    void setNullParameters(void);
    void setSimulationMode(bool on);

signals:
    void plotValues(double,double,double,double,double,double,double);



};
#endif // COMMTHREAD_H
