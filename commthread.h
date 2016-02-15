#ifndef COMMTHREAD_H
#define COMMTHREAD_H

#include <QThread>

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

public:
    explicit commThread(QObject *parent =0);
    void run();
    void setParameters(double, double, double, double, double, int);

signals:
    void plotValues(double,double,double,double,double,double);



};
#endif // COMMTHREAD_H
