#ifndef COMMTHREAD_H
#define COMMTHREAD_H

#include <QThread>
#include "quanser.h"

class commThread : public QThread
{
    Q_OBJECT

private:
    typedef QThread super;
    double frequencia;
    double amplitude;
    double offset;
    double duracaoMax;
    double duracaoMin;
    double sinalCalculado, lastTimeStamp, timeToNextRandomNumber;
    double lastLoopTimeStamp;
    int wave;
    bool malha;
    bool simulationMode;
    bool connected;
    double lockSignal(double sinalCalculado, double nivelTanque1, double nivelTanque2);
    int channel;

public:
    explicit commThread(QObject *parent =0);
    void run();
    void setParameters(double, double, double, double, double, int, bool, int);
    void setNullParameters(void);
    void setSimulationMode(bool on);
    void disconnect(void);
    void terminate(void);
signals:
    void plotValues(double,double,double,double,double,double,double);



};
#endif // COMMTHREAD_H
