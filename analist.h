#ifndef ANALIST_H
#define ANALIST_H

#include <QTimer>

class Analist
{
private:
    double ts[4], mp, tp, tr[3], tsOldTime, trOldTime[3], tsOpt, trOpt, initialLevel;
    double calcTs(double nivel, double setPoint, double timeStamp);
    double calcMp(double nivel, double setPoint);
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
    void reset(double nivel);
};

#endif // ANALIST_H
