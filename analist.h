#ifndef ANALIST_H
#define ANALIST_H

#include <QTimer>

class Analist
{
private:
    double ts, mp, tp, tr, tsOldTime, trOldTime;
    double calcTs(double nivel, double setPoint);
    double calcMp(double nivel, double setPoint);
    double calcTp(double nivel, double setPoint);
    double calcTr(double nivel, double setPoint);
public:
    Analist();
    void calc(double nivel, double setPoint, double timeStamp);
    double getTs(void);
    double getMp(void);
    double getTp(void);
    double getTr(void);
    void setTsOpt(int);
    void setTrOpt(int);
    void reset(void);
};

#endif // ANALIST_H
