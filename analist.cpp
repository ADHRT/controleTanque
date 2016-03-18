#include "analist.h"

Analist::Analist()
{

}

void Analist::calc(double nivel, double setPoint, double timeStamp)
{

}

double Analist::getTs()
{
    return ts;
}

double Analist::getMp()
{
    return mp;
}

double Analist::getTp()
{
    return tp;
}

double Analist::getTr()
{
    return tr;
}

void Analist::setTsOpt(int)
{

}

void Analist::setTrOpt(int)
{

}

void Analist::reset()
{
    ts = 0;
    mp = 0;
    tp = 0;
    tr = 0;
    tsOldTime = 0;
    trOldTime = 0;
}

double Analist::calcTs(double nivel, double setPoint)
{

}

double Analist::calcMp(double nivel, double setPoint)
{

}

double Analist::calcTp(double nivel, double setPoint)
{

}

double Analist::calcTr(double nivel, double setPoint)
{

}

