#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSortFilterProxyModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    stdWorkout = new standardWorkouts();
    listModel = new QStandardItemModel;
    plotModel = new QStandardItemModel;
    valueModel = new QStandardItemModel;

    metaProxy = new QSortFilterProxyModel;
    metaProxy->setSourceModel(stdWorkout->workouts_meta);
    stepProxy = new QSortFilterProxyModel;
    stepProxy->setSourceModel(stdWorkout->workouts_steps);

    modelHeader << "Phase" << "Level" << "Threshold %" << "Value" << "Time" << "TSS" << "Distance" << "Repeats";

    groupList << "Group" << "Series";
    ui->listWidget_group->addItems(groupList);
    phaseList << "Warmup" << "Permanent" << "Intervall" << "Break" << "Cooldown";
    ui->listWidget_phases->addItems(phaseList);
    levelList << "RECOM" << "END" << "TEMP" << "LT" << "VO2" << "ANAERO" << "NEURO";
    ui->comboBox_sport->addItem("Swim");
    ui->comboBox_sport->addItem("Bike");
    ui->comboBox_sport->addItem("Run");
    ui->comboBox_sport->addItem("Strength");
    ui->comboBox_sport->addItem("Alternativ");
    ui->treeWidget_intervall->setHeaderLabels(modelHeader);
    ui->treeWidget_intervall->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidget_intervall->setAcceptDrops(true);
    ui->treeWidget_intervall->setDragEnabled(true);
    ui->treeWidget_intervall->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeWidget_intervall->setEnabled(true);
    edit_del.phaseList = phaseList;
    edit_del.levelList = levelList;
    isSeries = "Series";
    isGroup = "Group";

    ui->frame_edit->setVisible(false);
    ui->listView_values->setModel(valueModel);
    ui->listView_values->setItemDelegate(&edit_del);
    ui->label_head->setText("Add Phase");
}

MainWindow::~MainWindow()
{
    delete stdWorkout;
    delete plotModel;
    delete valueModel;
    delete ui;
}

void MainWindow::on_treeWidget_intervall_itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if(currentItem != item)
    {
        if(item->data(0,Qt::DisplayRole) == isGroup || item->data(0,Qt::DisplayRole) == isSeries)
        {
            this->set_defaultData(item,false);
        }
        else
        {
            this->set_defaultData(item,true);
        }
    }

}
void MainWindow::set_itemData(QTreeWidgetItem *item)
{
    QString itemIdent = item->data(0,Qt::DisplayRole).toString();

    if(itemIdent.contains(isGroup) || itemIdent.contains(isSeries))
    {
        item->setData(0,Qt::EditRole,valueModel->data(valueModel->index(0,0)));
        item->setData(7,Qt::EditRole,valueModel->data(valueModel->index(1,0)));
    }
    else
    {
        for(int i = 0; i < valueModel->rowCount();++i)
        {
            item->setData(i,Qt::EditRole,valueModel->data(valueModel->index(i,0)));
        }
    }
}

void MainWindow::set_defaultData(QTreeWidgetItem *item,bool hasValues)
{
    currentItem = item;
    if(hasValues)
    {
        item->setData(1,Qt::EditRole,levelList.at(1));
        item->setData(2,Qt::EditRole,50);
        item->setData(3,Qt::EditRole,"200");
        item->setData(4,Qt::EditRole,"05:00");
        item->setData(5,Qt::EditRole,"10");
        item->setData(6,Qt::EditRole,"10.0");
        item->setData(7,Qt::EditRole,"");
    }
    else
    {
        item->setData(7,Qt::EditRole,2);
    }
    ui->treeWidget_intervall->expandAll();
    this->set_plotModel();
}

void MainWindow::get_workouts(QString sport)
{
    QString workID,workTitle,listString;
    workoutMap.clear();
    listModel->clear();
    metaProxy->setFilterFixedString(sport);
    metaProxy->setFilterKeyColumn(0);

    listModel->setColumnCount(2);
    listModel->setRowCount(metaProxy->rowCount());

    for(int i = 0; i < metaProxy->rowCount(); ++i)
    {
        workID = metaProxy->data(metaProxy->index(i,1)).toString();
        workTitle = metaProxy->data(metaProxy->index(i,3)).toString();
        listString = metaProxy->data(metaProxy->index(i,2)).toString() + " - " + workTitle;
        listModel->setData(listModel->index(i,0,QModelIndex()),listString);
        listModel->setData(listModel->index(i,1,QModelIndex()),workID);
    }
    listModel->sort(0);

    ui->listView_workouts->setModel(listModel);
    ui->listView_workouts->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::clearIntTree()
{
    QTreeWidgetItem *treeItem;
    while(ui->treeWidget_intervall->topLevelItemCount() > 0)
    {
        for(int c_item = 0; c_item < ui->treeWidget_intervall->topLevelItemCount(); ++c_item)
        {
        treeItem = ui->treeWidget_intervall->topLevelItem(c_item);

        if(treeItem->childCount() > 0)
        {
            for(int c_child = 0; c_child < treeItem->childCount(); ++c_child)
            {
                if(treeItem->child(c_child)->childCount() > 0)
                {
                    for(int subchild = 0; subchild < treeItem->child(c_child)->childCount(); ++subchild)
                    {
                        delete treeItem->child(c_child)->child(subchild);
                    }
                }
                delete treeItem->child(c_child);
            }
        }
        delete treeItem;
        }
    }
    plotModel->clear();
    this->set_plotGraphic(0);
}

void MainWindow::open_stdWorkout(QString workID)
{
    ui->treeWidget_intervall->clearFocus();
    if(ui->treeWidget_intervall->topLevelItemCount() > 0) ui->treeWidget_intervall->clear();

    QStringList valueList;
    QString parentItem,thresValue,stepTime;

    stepProxy->setFilterFixedString(workID);
    stepProxy->setFilterKeyColumn(0);

    for(int i = 0; i < stepProxy->rowCount();++i)
    {
        valueList.clear();

        //thresValue = this->calc_threshold(step_model->item(index.row(),4)->text().toDouble());
        thresValue = "-";
        stepTime = stepProxy->data(stepProxy->index(i,5)).toString();

        valueList << stepProxy->data(stepProxy->index(i,2)).toString()
                  << stepProxy->data(stepProxy->index(i,3)).toString()
                  << stepProxy->data(stepProxy->index(i,4)).toString()
                  << thresValue
                  << stepTime
                  << QString::number(this->estimate_stress(ui->comboBox_sport->currentText(),thresValue,this->get_timesec(stepTime)))
                  << stepProxy->data(stepProxy->index(i,6)).toString()
                  << stepProxy->data(stepProxy->index(i,7)).toString();

        QTreeWidgetItem *item = new QTreeWidgetItem(valueList);

        parentItem = stepProxy->data(stepProxy->index(i,8)).toString();

        if(parentItem.contains(isGroup) || parentItem.contains(isSeries))
        {
            QList<QTreeWidgetItem*>   pItem = ui->treeWidget_intervall->findItems(parentItem,Qt::MatchExactly | Qt::MatchRecursive,0);
            pItem.at(0)->addChild(item);
        }
        else
        {
            ui->treeWidget_intervall->insertTopLevelItem(ui->treeWidget_intervall->topLevelItemCount(),item);
        }

        ui->treeWidget_intervall->expandAll();
    }
    this->set_plotModel();
}

void MainWindow::set_plotModel_old()
{
    plotModel->clear();
    plotModel->setColumnCount(6);
    int parentReps = 0;
    int childReps = 0;
    int childCount = 0;
    int subchildCount = 0;
    int currIndex = 0;
    bool isload = false;
    QString phase,subphase;
    QTreeWidgetItem *topItem,*childItem,*subItem;

    for(int c_item = 0; c_item < ui->treeWidget_intervall->topLevelItemCount(); ++c_item)
    {
        //TopItems
        topItem = ui->treeWidget_intervall->topLevelItem(c_item);
        phase = topItem->data(0,Qt::DisplayRole).toString();
        childCount = topItem->childCount();
        ui->treeWidget_intervall->setCurrentItem(topItem);
        currIndex = ui->treeWidget_intervall->currentIndex().row();

        if(childCount > 0)
        {
            if(phase.contains(isGroup))
            {
                parentReps = topItem->data(7,Qt::DisplayRole).toInt();

                for(int c_parent = 0; c_parent < parentReps; ++c_parent)
                {
                    for(int c_childs = 0; c_childs < childCount ; ++c_childs)
                    {
                        childItem = ui->treeWidget_intervall->topLevelItem(c_item)->child(c_childs);
                        subchildCount = childItem->childCount();
                        subphase = childItem->data(0,Qt::DisplayRole).toString();
                        ui->treeWidget_intervall->setCurrentItem(childItem);
                        currIndex = ui->treeWidget_intervall->currentIndex().row();

                        if(subchildCount > 0)
                        {
                            if(subphase.contains(isSeries))
                            {
                                childReps = childItem->data(7,Qt::DisplayRole).toInt();
                                for(int c_child = 0; c_child < childReps; ++c_child)
                                {
                                    for(int c_subchilds = 0; c_subchilds < subchildCount; ++c_subchilds)
                                    {
                                        subItem = ui->treeWidget_intervall->topLevelItem(c_item)->child(c_childs)->child(c_subchilds);
                                        ui->treeWidget_intervall->setCurrentItem(subItem);
                                        currIndex = ui->treeWidget_intervall->currentIndex().row();
                                        this->add_to_plot(subItem,currIndex);
                                    }
                                }
                            }
                        }
                        else
                        {
                            this->add_to_plot(childItem,currIndex);
                        }
                    }
                }
            }
            else if(phase.contains(isSeries))
            {
                parentReps = topItem->data(7,Qt::DisplayRole).toInt();

                for(int c_parent = 0; c_parent < parentReps; ++c_parent)
                {
                    for(int c_childs = 0; c_childs < childCount ; ++c_childs)
                    {
                        childItem = ui->treeWidget_intervall->topLevelItem(c_item)->child(c_childs);
                        ui->treeWidget_intervall->setCurrentItem(childItem);
                        currIndex = ui->treeWidget_intervall->currentIndex().row();
                        this->add_to_plot(ui->treeWidget_intervall->topLevelItem(c_item)->child(c_childs),currIndex);
                    }
                }
            }
        }
        else
        {
            //Items without connection
            if(!phase.contains(isSeries) && !phase.contains(isGroup))
            {
                this->add_to_plot(topItem,currIndex);
            }
        }
        if(c_item == ui->treeWidget_intervall->topLevelItemCount()-1) isload = true;
    }
    if(isload) this->set_plotGraphic(plotModel->rowCount());
}

void MainWindow::set_plotModel()
{
    plotModel->clear();
    plotModel->setColumnCount(6);
    QTreeWidget *intTree = ui->treeWidget_intervall;
    QTreeWidgetItem *topItem,*childItem;//*subItem;
    int childCount = 0;
    int groupReps = 0;
    int seriesReps = 0;
    int currIndex = 0;
    QString phase,subphase;
    //qDebug() << "TopCount:"<<intTree->topLevelItemCount();

    for(int topLevel = 0; topLevel < intTree->topLevelItemCount(); ++topLevel)
    {
        topItem = intTree->topLevelItem(topLevel);
        childCount = topItem->childCount();
        phase = topItem->data(0,Qt::DisplayRole).toString();

        intTree->setCurrentItem(topItem);
        currIndex = intTree->currentIndex().row();
        //qDebug() << "TopLevelItem:"<<phase << childCount << currIndex;

        if(childCount > 0)
        {
            if(phase.contains(isGroup))
            {
                groupReps = topItem->data(7,Qt::DisplayRole).toInt();
                for(int repeat = 0; repeat < groupReps; ++repeat)
                {
                    for(int groupLevel = 0; groupLevel < childCount; ++groupLevel)
                    {
                        childItem = topItem->child(groupLevel);
                        subphase = childItem->data(0,Qt::DisplayRole).toString();
                        intTree->setCurrentItem(childItem);
                        currIndex = intTree->currentIndex().row();
                        //qDebug() << "GroupItem:"<<subphase << childItem->childCount() << currIndex;

                        this->add_to_plot(childItem,currIndex);
                    }
                }
            }
            else if(phase.contains(isSeries))
            {
                seriesReps = topItem->data(7,Qt::DisplayRole).toInt();
                for(int repeat = 0; repeat < seriesReps; ++repeat)
                {
                    for(int seriesLevel = 0; seriesLevel < childCount; ++seriesLevel)
                    {
                        childItem = topItem->child(seriesLevel);
                        subphase = childItem->data(0,Qt::DisplayRole).toString();
                        intTree->setCurrentItem(childItem);
                        currIndex = intTree->currentIndex().row();
                        //qDebug() << "SeriesItem:"<<subphase << childItem->childCount() << currIndex;

                        this->add_to_plot(childItem,currIndex);
                    }
                }
            }
            else
            {
                qDebug() << "NoAction";
            }
        }
        else
        {
            //Items without connection
            if(!phase.contains(isSeries) && !phase.contains(isGroup))
            {
                this->add_to_plot(topItem,currIndex);
            }
        }
    }
    this->set_plotGraphic(plotModel->rowCount());
}

void MainWindow::add_to_plot(QTreeWidgetItem *item,int currIndex)
{
    double time_sum = 0;
    double dist_sum = 0;
    double stress_sum = 0.0;

    if(currIndex == 0 && item->parent() == nullptr)
    {
        plotModel->insertRows(0,1,QModelIndex());
        plotModel->setData(plotModel->index(0,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
        plotModel->setData(plotModel->index(0,1,QModelIndex()),0.01);
        plotModel->setData(plotModel->index(0,2,QModelIndex()),0.0);
        plotModel->setData(plotModel->index(0,3,QModelIndex()),0);
        plotModel->setData(plotModel->index(0,4,QModelIndex()),"Start");
        plotModel->setData(plotModel->index(0,5,QModelIndex()),0);
    }

    int row = plotModel->rowCount();

    if(row != 0)
    {
        time_sum = plotModel->data(plotModel->index(row-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(row-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(row-1,3,QModelIndex())).toDouble();
    }

    plotModel->insertRows(row,1,QModelIndex());
    plotModel->setData(plotModel->index(row,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row,1,QModelIndex()),time_sum + (this->get_timesec(item->data(4,Qt::DisplayRole).toString()) / 60.0));
    plotModel->setData(plotModel->index(row,2,QModelIndex()),dist_sum + item->data(6,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row,3,QModelIndex()),stress_sum + item->data(5,Qt::DisplayRole).toDouble());

    if(item->parent() == nullptr)
    {
        plotModel->setData(plotModel->index(row,4,QModelIndex()),item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
    }
    else
    {
        plotModel->setData(plotModel->index(row,4,QModelIndex()),item->parent()->data(0,Qt::DisplayRole).toString()+"#"+item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
    }
    plotModel->setData(plotModel->index(row,5,QModelIndex()),row);

    for(int i = 0; i < plotModel->rowCount(); ++i)
    {
        qDebug() << i << "Tresh:"<<plotModel->data(plotModel->index(i,0)).toDouble()
                 << "Time:"<<plotModel->data(plotModel->index(i,1)).toDouble()
                 << "Label:"<<plotModel->data(plotModel->index(i,4)).toString()
                 << "Point"<<plotModel->data(plotModel->index(i,5)).toDouble();
    }

}

void MainWindow::set_plotGraphic(int c_ints)
{
    ui->widget_plot->clearPlottables();
    int int_loops = c_ints;
    int offset = 1;
    QVector<double> x_time(int_loops+offset),x_dist(int_loops+offset),y_thres(int_loops+offset);
    time_sum = 0.0;
    dist_sum = 0.0;
    stress_sum = 0.0;
    double thres_high = 0.0;

    ui->widget_plot->setInteractions(QCP::iSelectPlottables | QCP::iMultiSelect);
    QCPSelectionDecorator *lineDec = new QCPSelectionDecorator;
    lineDec->setPen(QPen(QColor(255,0,0)));
    lineDec->setBrush(QColor(255,0,0,50));
    QCPGraph *workout_line = ui->widget_plot->addGraph();
    workout_line->setSelectionDecorator(lineDec);

    if(c_ints != 0)
    {
        workout_line->setPen(QPen(Qt::blue));
        workout_line->setBrush(QColor(0,170,255,60));
        workout_line->setLineStyle((QCPGraph::lsStepRight));
        workout_line->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QPen(Qt::red),QBrush(Qt::white), 5));

        for (int data=0 ;  data < int_loops; ++data)
        {
             y_thres[data] = plotModel->data(plotModel->index(data,0,QModelIndex())).toDouble();
             x_time[data] = plotModel->data(plotModel->index(data,1,QModelIndex())).toDouble();
             x_dist[data] = plotModel->data(plotModel->index(data,2,QModelIndex())).toDouble();
             if(y_thres[data] > thres_high)
             {
                 thres_high = y_thres[data];
             }
             qDebug() << data << x_time[data] << y_thres[data];
        }
        time_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,3,QModelIndex())).toDouble();
        workout_line->setData(x_time,y_thres);
        ui->widget_plot->graph()->rescaleAxes(true);
        ui->widget_plot->xAxis->setRange(0,time_sum+((time_sum/2)*0.1));
        ui->widget_plot->xAxis2->setRange(0,dist_sum+((dist_sum/2)*0.1));
        ui->widget_plot->xAxis2->setVisible(true);
        ui->widget_plot->yAxis->setRange(0,thres_high+20.0);
    }

    ui->widget_plot->xAxis->setTicks(true);
    ui->widget_plot->yAxis->setTicks(true);
    ui->widget_plot->xAxis->setLabel("Time - Minutes");
    ui->widget_plot->xAxis2->setLabel("Distance - KM");
    ui->widget_plot->yAxis->setLabel("Threshold %");
    ui->widget_plot->xAxis->setTickLabels(true);
    ui->widget_plot->yAxis->setTickLabels(true);
    ui->widget_plot->replot();

}

void MainWindow::set_selectData(QTreeWidgetItem *item)
{
    QCPGraph *graph = ui->widget_plot->graph(0);
    int currIndex = ui->treeWidget_intervall->currentIndex().row();
    QString itemIdent;
    int dataCount = 0;
    bool groupRange = false;

    QSortFilterProxyModel *plotProxy = new QSortFilterProxyModel;
    plotProxy->setSourceModel(plotModel);

    if(item->parent() == nullptr)
    {
        graph->setSelectable(QCP::stDataRange);
        itemIdent = item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex);

        if(itemIdent.contains(isSeries))
        {
            itemIdent = itemIdent.split(":").first();
        }
        else if(itemIdent.contains(isGroup))
        {
            groupRange = true;
            int childCount,groupCount;
            dataCount = groupCount = item->data(7,Qt::DisplayRole).toInt();
            childCount = item->childCount();
            if(childCount > 0)
            {
                itemIdent = item->child(0)->data(0,Qt::DisplayRole).toString();
                for(int i = 0; i < childCount; ++i)
                {
                    if(item->child(i)->childCount() > 0)
                    {
                        int subCount = item->child(i)->childCount();
                        dataCount = dataCount*(subCount*item->child(i)->data(7,Qt::DisplayRole).toInt());
                    }
                    else
                    {
                        dataCount += groupCount;
                    }
                }
            }
        }
    }
    else
    {
        graph->setSelectable(QCP::stMultipleDataRanges);
        itemIdent = item->parent()->data(0,Qt::DisplayRole).toString()+"#"+item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex);
        if(item->data(0,Qt::DisplayRole).toString().contains(isSeries))
        {
            itemIdent = itemIdent.split("#").last();
            itemIdent = itemIdent.split(":").first();
        }
    }
    plotProxy->setFilterRegExp("^"+itemIdent);
    plotProxy->setFilterKeyColumn(4);
    plotProxy->sort(5);

    QCPDataSelection selection;

    if(graph->selectable() == QCP::stDataRange)
    {
        if(groupRange)
        {
            int dataStart = plotProxy->data(plotProxy->index(0,5)).toInt();
            selection.addDataRange(QCPDataRange(dataStart,dataStart+dataCount+1));
        }
        else
        {
            qDebug() << plotProxy->data(plotProxy->index(0,5)).toInt() <<"-"<< plotProxy->data(plotProxy->index(0,5)).toInt()+1;
            selection.addDataRange(QCPDataRange(plotProxy->data(plotProxy->index(0,5)).toInt(),plotProxy->data(plotProxy->index(plotProxy->rowCount()-1,5)).toInt()+1));
        }
    }

    if(graph->selectable() == QCP::stMultipleDataRanges)
    {
        for(int i = 0; i < plotProxy->rowCount(); ++i)
        {
            qDebug() << plotProxy->data(plotProxy->index(i,5)).toInt() <<"-"<< plotProxy->data(plotProxy->index(i,5)).toInt()+1;
            selection.addDataRange(QCPDataRange(plotProxy->data(plotProxy->index(i,5)).toInt(),plotProxy->data(plotProxy->index(i,5)).toInt()+1),false);
        }
        qDebug() << selection.dataRangeCount();
    }

    graph->setSelection(selection);
    ui->widget_plot->replot();
}

void MainWindow::on_treeWidget_intervall_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    currentItem = item;
    QString itemIdent = item->data(0,Qt::DisplayRole).toString();
    valueModel->clear();
    valueModel->setColumnCount(2);

    if(itemIdent.contains(isGroup) || itemIdent.contains(isSeries))
    {
        valueModel->setRowCount(2);
        edit_del.hasValue = false;
        valueModel->setData(valueModel->index(0,0,QModelIndex()),item->data(0,Qt::DisplayRole));
        valueModel->setData(valueModel->index(1,0,QModelIndex()),item->data(7,Qt::DisplayRole));
    }
    else
    {
        valueModel->setRowCount(7);
        for(int i = 0; i < item->columnCount();++i)
        {
            valueModel->setData(valueModel->index(i,0,QModelIndex()),item->data(i,Qt::DisplayRole));
            valueModel->setData(valueModel->index(i,1,QModelIndex()),0);
        }
        valueModel->setData(valueModel->index(2,1,QModelIndex()),levelList.indexOf(valueModel->data(valueModel->index(1,0)).toString()));
        edit_del.hasValue = true;
    }

    ui->frame_edit->setVisible(true);
    ui->frame_select->setVisible(false);
    ui->label_head->setText("Edit Phase");

    ui->widget_plot->graph(0)->setSelection(QCPDataSelection(QCPDataRange(0,0)));
    this->set_selectData(item);
}

void MainWindow::on_comboBox_sport_currentTextChanged(const QString &value)
{
    this->get_workouts(value);
}

void MainWindow::on_listView_workouts_clicked(const QModelIndex &index)
{
    //QStringList workoutTitle = stdProxy->data(stdProxy->index(index.row(),0)).toString().split("-");
    QString workoutID = listModel->data(listModel->index(index.row(),1)).toString();
    //QString workCode = workoutTitle.first();
    //QString workTitle = workoutTitle.last();
    this->open_stdWorkout(workoutID);
}

void MainWindow::on_toolButton_update_clicked()
{
    ui->frame_edit->setVisible(false);
    ui->frame_select->setVisible(true);
    ui->label_head->setText("Add Phase");
    this->set_itemData(currentItem);
    this->set_plotModel();
}

void MainWindow::on_toolButton_remove_clicked()
{
    if(ui->treeWidget_intervall->topLevelItemCount() == 1)
    {
        ui->treeWidget_intervall->clear();
    }
    else
    {
        if(currentItem->childCount() > 0)
        {
            while(currentItem->childCount() > 0)
            {
                for(int i = 0; i < currentItem->childCount(); ++i)
                {
                    delete currentItem->child(i);
                }
            }
        }
        delete currentItem;
    }
    ui->frame_edit->setVisible(false);
    ui->frame_select->setVisible(true);
    ui->label_head->setText("Add Phase");
    this->set_plotModel();
}

void MainWindow::on_toolButton_clear_clicked()
{
    this->clearIntTree();
}
