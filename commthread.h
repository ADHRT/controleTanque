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
    double sinalCalculado, lastTimeStamp, timeToNextRandomNumber;
    double lockSignal(double sinalCalculado, double nivelTanque1);
    double lastLoopTimeStamp;

public:
    explicit commThread(QObject *parent =0);
    void run();
    void setParameters(double, double, double, double, double, int);

signals:
    void plotValues(double,double,double,double,double,double,double);



};
#endif // COMMTHREAD_H
