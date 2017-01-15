#ifndef STANDARDWORKOUTS_H
#define STANDARDWORKOUTS_H

#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QtXml>
#include <QFile>

class standardWorkouts
{
public:
    standardWorkouts();
    QStandardItemModel *standard_workouts,*workouts_meta, *workouts_steps;

    void set_saveFlag(bool isSave) {save_workouts = isSave;}
    void save_stdWorkouts() {write_standard_workouts();}
    void delete_stdWorkout(QString,bool);
    QStringList get_workoutIds() {return workoutIDs;}

private:
    QStringList meta_tags,step_tags,workoutIDs;
    QString workoutPath;
    bool save_workouts;
    void check_workoutFiles();
    void read_standard_workouts();
    void write_standard_workouts();
    void set_workoutIds();
};

#endif // STANDARDWORKOUTS_H
