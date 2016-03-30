#ifndef ANALIST_H
#define ANALIST_H

#include <QTimer>
#include <QDateTime>
#include <QDebug>

class Analist
{
private:
    const double porcTs[4] = {0.02, 0.05, 0.07, 0.1};
    double ts[4], mp, mpPerc, tp, tr[3], tsOldTime, trOldTime[3], mpInitialTime, oldSetPoint, initialLevel, porcInital[3], porcFinal[3], InitialLevelTs, tsInitialTime;
    bool direction, reachedTr[3];
    void calcTs(double nivel, double setPoint, double timeStamp);
    void calcMpTp(double nivel, double setPoint, double timeStamp);
    double calcTp(double nivel, double setPoint);
    void calcTr(double nivel, double setPoint, double timeStamp);
public:
    Analist();
    //bool reachedTr;
    void calc(double nivel, double setPoint, double timeStamp);
    double getMp(void);
    double getMpPerc(void);
    double getTp(void);
    double getTs(int);
    double getTr(int);
    void reset(double setPoint);
};

#endif // ANALIST_H
