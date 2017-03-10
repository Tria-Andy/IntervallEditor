#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QHBoxLayout>
#include <QDataWidgetMapper>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include "del_editvalues.h"
#include "standardworkouts.h"
#include "calculation.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public calculation
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_toolButton_update_clicked();
    void on_toolButton_remove_clicked();
    void on_comboBox_sport_currentTextChanged(const QString &arg1);
    void on_listView_workouts_clicked(const QModelIndex &index);
    void on_toolButton_clear_clicked();
    void on_treeView_intervall_clicked(const QModelIndex &index);
    void set_selectData(QModelIndex);
    void set_newItemData(QModelIndex);

private:
    Ui::MainWindow *ui;
    QString isSeries,isGroup,current_sport,current_workID;
    QStandardItemModel *workoutModel,*plotModel,*valueModel,*listModel;
    QStandardItem *rootItem;
    QItemSelectionModel *treeSelection;
    QDataWidgetMapper *valueMapper;
    QHBoxLayout *valueLayout;
    QSortFilterProxyModel *metaProxy,*stepProxy;
    QMap<QString,QString> workoutMap;
    QStringList modelHeader,phaseList,groupList,levelList;
    double time_sum,dist_sum,stress_sum;
    standardWorkouts *stdWorkout;
    del_editvalues edit_del;
    QVector<bool> editRow;

    QLineEdit *valueName;
    QComboBox *valuePhase;

    QList<QStandardItem *> read_itemData(int,bool);
    void set_itemData(QTreeWidgetItem *item);
    void set_defaultData(QTreeWidgetItem *item,bool);

    void show_editItem(QTreeWidgetItem *item);
    void get_workouts(QString);
    void clearIntTree();
    void open_stdWorkout(QString);
    void set_plotModel();
    void testPlot(int);
    void set_plotModel_old();
    void add_itemPlot(QStandardItem*,QList<QStandardItem*> *itemList);
    void set_plotGraphic(int);

};

#endif // MAINWINDOW_H
