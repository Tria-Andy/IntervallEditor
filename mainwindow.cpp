#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSortFilterProxyModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    stdWorkout = new standardWorkouts();
    workoutModel = new QStandardItemModel;
    rootItem = workoutModel->invisibleRootItem();
    listModel = new QStandardItemModel;
    plotModel = new QStandardItemModel;
    valueModel = new QStandardItemModel;
    valueMapper = new QDataWidgetMapper;
    valueMapper->setModel(valueModel);

    metaProxy = new QSortFilterProxyModel;
    metaProxy->setSourceModel(stdWorkout->workouts_meta);
    stepProxy = new QSortFilterProxyModel;
    stepProxy->setSourceModel(stdWorkout->workouts_steps);

    modelHeader << "Phase" << "Level" << "Threshold %" << "Value" << "Time" << "TSS" << "Distance" << "Repeats";

    groupList << "Group" << "Series";
    ui->listWidget_group->addItems(groupList);
    phaseList << "Warmup" << "Permanent" << "Interval" << "Break" << "Cooldown";
    ui->listWidget_phases->addItems(phaseList);
    levelList << "RECOM" << "END" << "TEMP" << "LT" << "VO2" << "ANAERO" << "NEURO";
    ui->comboBox_sport->addItem("Swim");
    ui->comboBox_sport->addItem("Bike");
    ui->comboBox_sport->addItem("Run");
    ui->comboBox_sport->addItem("Strength");
    ui->comboBox_sport->addItem("Alternativ");

    workoutModel->setHorizontalHeaderLabels(modelHeader);
    ui->treeView_intervall->setModel(workoutModel);
    ui->treeView_intervall->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView_intervall->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeView_intervall->setAcceptDrops(true);
    ui->treeView_intervall->setDragEnabled(true);
    ui->treeView_intervall->setDragDropMode(QAbstractItemView::DragDrop);
    treeSelection = ui->treeView_intervall->selectionModel();
    connect(treeSelection,SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(set_selectData(QModelIndex)));
    connect(workoutModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(set_newItemData(QModelIndex)));

    edit_del.phaseList = phaseList;
    edit_del.levelList = levelList;
    isSeries = "Series";
    isGroup = "Group";

    for(int i = 1; i < 10; ++i)
    {
        ui->comboBox_reps->addItem(QString::number(i));
    }

    ui->frame_edit->setVisible(false);
    valueLayout = new QHBoxLayout(ui->frame_editValues);
    valuePhase = new QComboBox;
    valuePhase->addItems(phaseList);
    valueLayout->addWidget(valuePhase);
    //valueName = new QLineEdit;
    //valueLayout->addWidget(valueName);
    valueMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    //valueMapper->setItemDelegate(&edit_del);
    valueMapper->addMapping(valuePhase,0);
    //ui->listView_values->setModel(valueModel);
    //ui->listView_values->setItemDelegate(&edit_del);
    ui->label_head->setText("Add Phase");
}

MainWindow::~MainWindow()
{
    delete stdWorkout;
    delete plotModel;
    delete valueModel;
    delete ui;
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
    //this->set_plotModel();
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


void MainWindow::set_newItemData(QModelIndex index)
{
    qDebug() << "Insert" << index.row();
    qDebug() << workoutModel->rowCount();
}



void MainWindow::open_stdWorkout(QString workID)
{
    if(workoutModel->rowCount() > 0) workoutModel->clear();
    workoutModel->setHorizontalHeaderLabels(modelHeader);
    rootItem = workoutModel->invisibleRootItem();
    QStandardItem *parentItem;
    QString phaseName,parentName;

    stepProxy->setFilterFixedString(workID);
    stepProxy->setFilterKeyColumn(0);

    for(int i = 0; i < stepProxy->rowCount();++i)
    {
        phaseName = stepProxy->data(stepProxy->index(i,2)).toString();

        if(phaseName.contains(isGroup) || phaseName.contains(isSeries))
        {
            parentName = stepProxy->data(stepProxy->index(i,8)).toString();

            if(parentName.contains(isGroup))
            {
                parentItem->appendRow(this->read_itemData(i,true));
                parentItem = parentItem->child(parentItem->rowCount()-1,0);
            }
            else
            {
                rootItem->appendRow(this->read_itemData(i,true));
                parentItem = workoutModel->item(workoutModel->rowCount()-1);
            }
        }
        else
        {
            parentName = stepProxy->data(stepProxy->index(i,8)).toString();

            if(parentName.contains(isSeries))
            {
                parentItem->appendRow(this->read_itemData(i,false));
            }
            else
            {
                rootItem->appendRow(this->read_itemData(i,false));
            }
        }
    }
    ui->treeView_intervall->expandAll();
    this->set_plotModel();
}

QList<QStandardItem *> MainWindow::read_itemData(int stepRow,bool isParent)
{
    QList<QStandardItem*> workItems;
    QString thresValue,stepTime,stressValue;

    if(isParent)
    {
        stepTime = thresValue = stressValue ="";
    }
    else
    {
        stepTime = stepProxy->data(stepProxy->index(stepRow,5)).toString();
        thresValue = "-";
        stressValue = QString::number(this->estimate_stress(ui->comboBox_sport->currentText(),thresValue,this->get_timesec(stepTime)));

    }

    workItems << new QStandardItem(stepProxy->data(stepProxy->index(stepRow,2)).toString());
    workItems << new QStandardItem(stepProxy->data(stepProxy->index(stepRow,3)).toString());
    workItems << new QStandardItem(stepProxy->data(stepProxy->index(stepRow,4)).toString());
    workItems << new QStandardItem(thresValue);
    workItems << new QStandardItem(stepTime);
    workItems << new QStandardItem(stressValue);
    workItems << new QStandardItem(stepProxy->data(stepProxy->index(stepRow,6)).toString());
    workItems << new QStandardItem(stepProxy->data(stepProxy->index(stepRow,7)).toString());

    return workItems;
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

void MainWindow::set_plotModel()
{
    plotModel->clear();
    plotModel->setColumnCount(6);

    QStandardItem *currItem,*subItem;
    QList<QStandardItem*> itemList;

    for(int row = 0; row < workoutModel->rowCount(); ++row)
    {
        currItem = workoutModel->item(row,0);

        if(currItem->hasChildren())
        {
            int groupReps = workoutModel->data(workoutModel->index(row,7)).toInt();

            for(int reps = 0; reps < groupReps; ++reps)
            {
                for(int childRow = 0; childRow < currItem->rowCount(); ++childRow)
                {
                    subItem = currItem->child(childRow,0);

                    if(subItem->hasChildren())
                    {
                        int seriesReps = currItem->child(childRow,7)->data(Qt::DisplayRole).toInt();

                        for(int reps = 0; reps < seriesReps; ++reps)
                        {
                            for(int subChild = 0; subChild < subItem->rowCount(); ++subChild)
                            {
                                //qDebug() << subItem->child(subChild,0)->data(Qt::DisplayRole).toString();
                                itemList.append(subItem->child(subChild,2));
                                itemList.append(subItem->child(subChild,4));
                                itemList.append(subItem->child(subChild,5));
                                itemList.append(subItem->child(subChild,6));
                                this->add_itemPlot(subItem->child(subChild,0),&itemList);
                                itemList.clear();
                            }
                        }
                    }
                    else
                    {
                        itemList.append(currItem->child(childRow,2));
                        itemList.append(currItem->child(childRow,4));
                        itemList.append(currItem->child(childRow,5));
                        itemList.append(currItem->child(childRow,6));
                        this->add_itemPlot(subItem,&itemList);
                        itemList.clear();
                    }
                }
            }
        }
        else
        {
            itemList.append(workoutModel->item(row,2));
            itemList.append(workoutModel->item(row,4));
            itemList.append(workoutModel->item(row,5));
            itemList.append(workoutModel->item(row,6));
            this->add_itemPlot(currItem,&itemList);
            itemList.clear();
        }

    }
    this->set_plotGraphic(plotModel->rowCount());
}

void MainWindow::add_itemPlot(QStandardItem *item, QList<QStandardItem*> *itemList)
{
    double time_sum = 0.01;
    double dist_sum = 0;
    double stress_sum = 0.0;
    int row = plotModel->rowCount();

    int currentRow = item->row();

    if(row != 0)
    {
        time_sum = plotModel->data(plotModel->index(row-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(row-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(row-1,3,QModelIndex())).toDouble();
    }
    else
    {
        plotModel->setData(plotModel->index(0,0,QModelIndex()),itemList->at(0)->data(Qt::DisplayRole).toDouble());
    }

    plotModel->insertRows(row,2,QModelIndex());
    plotModel->setData(plotModel->index(row,0,QModelIndex()),itemList->at(0)->data(Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row,1,QModelIndex()),time_sum);
    plotModel->setData(plotModel->index(row,2,QModelIndex()),dist_sum);
    plotModel->setData(plotModel->index(row,3,QModelIndex()),0);

    plotModel->setData(plotModel->index(row+1,0,QModelIndex()),itemList->at(0)->data(Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row+1,1,QModelIndex()),time_sum + (this->get_timesec(itemList->at(1)->data(Qt::DisplayRole).toString()) / 60.0));
    plotModel->setData(plotModel->index(row+1,2,QModelIndex()),dist_sum + itemList->at(3)->data(Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row+1,3,QModelIndex()),stress_sum + itemList->at(2)->data(Qt::DisplayRole).toDouble());


    if(item->parent() == nullptr)
    {
        plotModel->setData(plotModel->index(row,4,QModelIndex()),item->data(Qt::DisplayRole).toString()+":"+QString::number(currentRow));
        plotModel->setData(plotModel->index(row+1,4,QModelIndex()),item->data(Qt::DisplayRole).toString()+":"+QString::number(currentRow));
    }
    else
    {
        plotModel->setData(plotModel->index(row,4,QModelIndex()),item->parent()->data(Qt::DisplayRole).toString()+"#"+item->data(Qt::DisplayRole).toString()+":"+QString::number(currentRow));
        plotModel->setData(plotModel->index(row+1,4,QModelIndex()),item->parent()->data(Qt::DisplayRole).toString()+"#"+item->data(Qt::DisplayRole).toString()+":"+QString::number(currentRow));
    }

    plotModel->setData(plotModel->index(row,5,QModelIndex()),row);
    plotModel->setData(plotModel->index(row+1,5,QModelIndex()),row+1);
}

void MainWindow::set_plotGraphic(int c_ints)
{
    ui->widget_plot->clearPlottables();
    int int_loops = c_ints;
    //int offset = 1;
    QVector<double> x_time(int_loops),x_dist(int_loops),y_thres(int_loops);
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
        workout_line->setLineStyle((QCPGraph::lsLine));
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
             //qDebug() << data << x_time[data] << y_thres[data] << plotModel->data(plotModel->index(data,4)).toString();
        }
        //qDebug() << "*****************************";
        time_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,3,QModelIndex())).toDouble();
        workout_line->setData(x_time,y_thres,true);

        //ui->widget_plot->graph()->rescaleAxes(true);
        ui->widget_plot->xAxis->setRange(0,time_sum+((time_sum/2)*0.1));
        ui->widget_plot->xAxis2->setRange(0,dist_sum+((dist_sum/2)*0.1));
        ui->widget_plot->xAxis2->setVisible(true);
        ui->widget_plot->yAxis->setRange(0,thres_high+20.0);
    }

    //ui->widget_plot->xAxis->setTicks(true);
    //ui->widget_plot->yAxis->setTicks(true);
    ui->widget_plot->xAxis->setLabel("Time - Minutes");
    ui->widget_plot->xAxis2->setLabel("Distance - KM");
    ui->widget_plot->yAxis->setLabel("Threshold %");
    //ui->widget_plot->xAxis->setTickLabels(true);
    //ui->widget_plot->yAxis->setTickLabels(true);
    ui->widget_plot->replot();

}

void MainWindow::on_treeView_intervall_clicked(const QModelIndex &index)
{
    ui->widget_plot->graph(0)->setSelection(QCPDataSelection(QCPDataRange(0,0)));
}

void MainWindow::set_selectData(QModelIndex selectIndex)
{
    QCPGraph *graph = ui->widget_plot->graph(0);
    int indexRow = selectIndex.row();
    QString itemIdent,selectItem;
    int dataCount = 0;
    bool groupRange = false;

    treeSelection->select(selectIndex,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    QStandardItem *item = workoutModel->itemFromIndex(selectIndex);

    QSortFilterProxyModel *plotProxy = new QSortFilterProxyModel;
    plotProxy->setSourceModel(plotModel);

    selectItem = treeSelection->selectedRows(0).at(0).data().toString();

    if(item->parent() == nullptr)
    {
        qDebug() << "ParentIndex Not Valid";

        graph->setSelectable(QCP::stDataRange);
        itemIdent = selectItem+":"+QString::number(indexRow);

        if(itemIdent.contains(isSeries))
        {
            itemIdent = itemIdent.split(":").first();
        }
        else if(itemIdent.contains(isGroup))
        {
            groupRange = true;
            int childCount,groupCount;
            dataCount = groupCount = treeSelection->selectedRows(7).at(0).data().toInt()*2;
            childCount = workoutModel->itemFromIndex(selectIndex)->rowCount();

            if(childCount > 0)
            {
                itemIdent = item->child(0)->data(Qt::DisplayRole).toString();
                for(int i = 0; i < childCount; ++i)
                {
                    if(item->child(i)->rowCount() > 0)
                    {
                        int subCount = item->child(i)->rowCount();
                        dataCount = dataCount*(subCount*item->child(i,7)->data(Qt::DisplayRole).toInt());
                        qDebug() << dataCount;
                    }
                    else
                    {
                        qDebug() << dataCount << groupCount;
                        dataCount += groupCount;
                        qDebug() << dataCount;
                    }
                }
            }
        }
    }
    else
    {

        qDebug() << "ParentIndex Valid";

        graph->setSelectable(QCP::stMultipleDataRanges);
        itemIdent = item->parent()->data(Qt::DisplayRole).toString()+"#"+item->data(Qt::DisplayRole).toString()+":"+QString::number(indexRow);
        qDebug() << itemIdent;
        if(selectItem.contains(isSeries))
        {
            itemIdent = itemIdent.split("#").last();
            itemIdent = itemIdent.split(":").first();
        }
        qDebug() << itemIdent;
    }
    plotProxy->setFilterRegExp("^"+itemIdent);
    plotProxy->setFilterKeyColumn(4);
    plotProxy->sort(5);

    for(int i = 0; i < plotProxy->rowCount(); ++i)
    {
        qDebug() << plotProxy->data(plotProxy->index(i,4)).toString()
                 << plotProxy->data(plotProxy->index(i,5)).toInt();
    }

    QCPDataSelection selection;

    if(graph->selectable() == QCP::stDataRange)
    {
        if(groupRange)
        {
            int dataStart = plotProxy->data(plotProxy->index(0,5)).toInt();
            selection.addDataRange(QCPDataRange(dataStart,dataStart+dataCount));
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
    }

    graph->setSelection(selection);
    ui->widget_plot->replot();
}

void MainWindow::on_toolButton_update_clicked()
{

}

void MainWindow::on_toolButton_remove_clicked()
{

}

void MainWindow::on_toolButton_clear_clicked()
{

}


/*

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


void MainWindow::set_plotModel()
{
    plotModel->clear();
    plotModel->setColumnCount(6);

    QTreeWidget *intTree = ui->treeWidget_intervall;
    QTreeWidgetItem *topItem,*childItem,*subItem;
    int childCount = 0;
    int subchildCount = 0;
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
                        subchildCount = childItem->childCount();
                        intTree->setCurrentItem(childItem);
                        currIndex = intTree->currentIndex().row();
                        //qDebug() << "GroupItem:"<<subphase << childItem->childCount() << currIndex;

                        if(subchildCount > 0)
                        {
                            if(subphase.contains(isSeries))
                            {
                                seriesReps = childItem->data(7,Qt::DisplayRole).toInt();
                                for(int repeat = 0; repeat < seriesReps; ++repeat)
                                {
                                    for(int subchild = 0; subchild < subchildCount; ++subchild)
                                    {
                                        subItem = childItem->child(subchild);
                                        intTree->setCurrentItem(subItem);
                                        currIndex = intTree->currentIndex().row();
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
                seriesReps = topItem->data(7,Qt::DisplayRole).toInt();
                for(int repeat = 0; repeat < seriesReps; ++repeat)
                {
                    for(int seriesLevel = 0; seriesLevel < childCount; ++seriesLevel)
                    {
                        childItem = topItem->child(seriesLevel);
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
    double time_sum = 0.01;
    double dist_sum = 0;
    double stress_sum = 0.0;
    int row = plotModel->rowCount();

    if(currIndex == 0 && item->parent() == nullptr)
    {
        plotModel->setData(plotModel->index(0,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    }

    if(row != 0)
    {
        time_sum = plotModel->data(plotModel->index(row-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(row-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(row-1,3,QModelIndex())).toDouble();
    }

    plotModel->insertRows(row,2,QModelIndex());
    plotModel->setData(plotModel->index(row,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row,1,QModelIndex()),time_sum);
    plotModel->setData(plotModel->index(row,2,QModelIndex()),dist_sum);
    plotModel->setData(plotModel->index(row,3,QModelIndex()),0);

    plotModel->setData(plotModel->index(row+1,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row+1,1,QModelIndex()),time_sum + (this->get_timesec(item->data(4,Qt::DisplayRole).toString()) / 60.0));
    plotModel->setData(plotModel->index(row+1,2,QModelIndex()),dist_sum + item->data(6,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row+1,3,QModelIndex()),stress_sum + item->data(5,Qt::DisplayRole).toDouble());

    if(item->parent() == nullptr)
    {
        plotModel->setData(plotModel->index(row,4,QModelIndex()),item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
        plotModel->setData(plotModel->index(row+1,4,QModelIndex()),item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
    }
    else
    {
        plotModel->setData(plotModel->index(row,4,QModelIndex()),item->parent()->data(0,Qt::DisplayRole).toString()+"#"+item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
        plotModel->setData(plotModel->index(row+1,4,QModelIndex()),item->parent()->data(0,Qt::DisplayRole).toString()+"#"+item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
    }
    plotModel->setData(plotModel->index(row,5,QModelIndex()),row);
    plotModel->setData(plotModel->index(row+1,5,QModelIndex()),row+1);
}

void MainWindow::set_plotGraphic(int c_ints)
{
    ui->widget_plot->clearPlottables();
    int int_loops = c_ints;
    int offset = 1;
    QVector<double> x_time(int_loops),x_dist(int_loops),y_thres(int_loops);
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
        workout_line->setLineStyle((QCPGraph::lsLine));
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
             qDebug() << data << x_time[data] << y_thres[data] << plotModel->data(plotModel->index(data,4)).toString();
        }
        qDebug() << "*****************************";
        time_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,3,QModelIndex())).toDouble();
        workout_line->setData(x_time,y_thres,true);

        //ui->widget_plot->graph()->rescaleAxes(true);
        ui->widget_plot->xAxis->setRange(0,time_sum+((time_sum/2)*0.1));
        ui->widget_plot->xAxis2->setRange(0,dist_sum+((dist_sum/2)*0.1));
        ui->widget_plot->xAxis2->setVisible(true);
        ui->widget_plot->yAxis->setRange(0,thres_high+20.0);
    }

    //ui->widget_plot->xAxis->setTicks(true);
    //ui->widget_plot->yAxis->setTicks(true);
    ui->widget_plot->xAxis->setLabel("Time - Minutes");
    ui->widget_plot->xAxis2->setLabel("Distance - KM");
    ui->widget_plot->yAxis->setLabel("Threshold %");
    //ui->widget_plot->xAxis->setTickLabels(true);
    //ui->widget_plot->yAxis->setTickLabels(true);
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
            dataCount = groupCount = item->data(7,Qt::DisplayRole).toInt()*2;
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
                        qDebug() << dataCount;
                    }
                    else
                    {
                        qDebug() << dataCount << groupCount;
                        dataCount += groupCount;
                        qDebug() << dataCount;
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

    for(int i = 0; i < plotProxy->rowCount(); ++i)
    {
        qDebug() << plotProxy->data(plotProxy->index(i,4)).toString()
                 << plotProxy->data(plotProxy->index(i,5)).toInt();
    }

    QCPDataSelection selection;

    if(graph->selectable() == QCP::stDataRange)
    {
        if(groupRange)
        {
            int dataStart = plotProxy->data(plotProxy->index(0,5)).toInt();
            selection.addDataRange(QCPDataRange(dataStart,dataStart+dataCount));
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
    valueMapper->toFirst();
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
    valueMapper->submit();
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
*/

