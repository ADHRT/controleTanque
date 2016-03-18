#ifndef ANALIST_H
#define ANALIST_H

#include <QTimer>
#include <QDateTime>
#include <QDebug>

class Analist
{
private:
    double ts[4], mp, tp, tr[3], tsOldTime, trOldTime[3], tsOpt, trOpt, mpInitialTime, oldSetPoint, initialLevel;
    bool direction;
    double calcTs(double nivel, double setPoint, double timeStamp);
    double calcMpTp(double nivel, double setPoint);
    double calcTp(double nivel, double setPoint);
    double calcTr(double nivel, double setPoint, double timeStamp);
public:
    Analist();
    void calc(double nivel, double setPoint, double timeStamp);
    double getTs(void);
    double getMp(void);
    double getTp(void);
    double getTr(void);
    void setTsOpt(int);
    void setTrOpt(int);
    void reset(double setPoint);
};

#endif // ANALIST_H
