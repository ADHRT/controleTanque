#ifndef ANALIST_H
#define ANALIST_H

#include <QTimer>
#include <QDateTime>
#include <QDebug>

class Analist
{
private:
    double ts, mp, tp, tr[3], tsOldTime, trOldTime[3], tsOpt, trOpt, mpInitialTime, oldSetPoint, initialLevel, porcInital[3], porcFinal[3];
    bool direction;
    double calcTs(double nivel, double setPoint, double timeStamp);
    double calcMpTp(double nivel, double setPoint);
    double calcTp(double nivel, double setPoint);
    void calcTr(double nivel, double setPoint, double timeStamp);
public:
    Analist();
    void calc(double nivel, double setPoint, double timeStamp);
    double getTs(void);
    double getMp(void);
    double getTp(void);
    double getTr(int);
    void setTsOpt(int);
    void setTrOpt(int);
    void reset(double setPoint);
};

#endif // ANALIST_H
