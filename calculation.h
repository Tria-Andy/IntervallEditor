#ifndef CALCULATION_H
#define CALCULATION_H

#include <QtCore>

class calculation
{
public:
    calculation();
    double calc_totalWork(double,double,double);
    QString set_time(int);
    QString get_workout_pace(double, QTime, QString,bool);
    QString get_speed(QTime,int,QString,bool);
    double estimate_stress(QString,QString,int);
    int get_hfvalue(QString);
    int get_timesec(QString time);
    double set_doubleValue(double,bool);
};

#endif // CALCULATION_H
