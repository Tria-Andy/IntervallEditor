#ifndef DEL_EDITVALUES_H
#define DEL_EDITVALUES_H
#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QDebug>
#include "calculation.h"

class del_editvalues : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_editvalues(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    QStringList phaseList,levelList;
    bool hasValue;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        if(hasValue)
        {
            if(index.row() == 0 || index.row() == 1)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                return editor;
            }
            if(index.row() == 2)
            {
                const QAbstractItemModel *model = index.model();
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
                editor->setMinimum(set_thresMin(model->data(model->index(index.row(),1)).toInt()));
                editor->setMaximum(set_thresMax(model->data(model->index(index.row(),1)).toInt()));
                return editor;
            }
            if(index.row() ==  4)
            {
                QTimeEdit *editor = new QTimeEdit(parent);
                editor->setDisplayFormat("mm:ss");
                editor->setFrame(true);
                return editor;
            }
            if(index.row() ==  5)
            {
                QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                editor->setFrame(true);
                editor->setDecimals(2);
                editor->setMinimum(0.0);
                editor->setMaximum(500.);
                return editor;
            }
        }
        else
        {
            if(index.row() == 0)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                return editor;
            }
            if(index.row() == 1)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
                editor->setMinimum(2);
                editor->setMaximum(20);
                return editor;
            }
        }
        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if(hasValue)
        {
            if(index.row() == 0 || index.row() == 1)
            {
                QString value = index.data(Qt::DisplayRole).toString();
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                if(index.row() == 0)
                {
                    comboBox->addItems(phaseList);
                }
                else
                {
                    comboBox->addItems(levelList);
                }
                comboBox->setCurrentText(value);
            }
            if(index.row() == 2)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toInt());
            }
            if(index.row() == 4)
            {
                QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"mm:ss"));
            }
            if(index.row() == 5)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
            }
        }
        else
        {
            if(index.row() == 0)
            {
                QString value = index.data(Qt::DisplayRole).toString();
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                comboBox->addItem("Group");
                comboBox->addItem("Series");
                comboBox->setCurrentText(value);
            }
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toInt());
            }
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(hasValue)
        {
            if(index.row() == 0 || index.row() == 1)
            {
               QComboBox *comboBox = static_cast<QComboBox*>(editor);
               QString value = comboBox->currentText();
               model->setData(index,value, Qt::EditRole);
               if(index.row() == 1)
               {
                   model->setData(model->index(2,1),comboBox->currentIndex());
                   model->setData(model->index(2,0),set_thresMin(comboBox->currentIndex()));
               }
            }
            if(index.row() == 2)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value, Qt::EditRole);
                model->setData(model->index(3,0),200.0*(value/100.0));
            }
            if(index.row() == 4)
            {
                QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                QTime value = timeEdit->time();
                timeEdit->interpretText();
                model->setData(index,value.toString("mm:ss"), Qt::EditRole);
            }
            if(index.row() == 5)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->interpretText();
                double value = spinBox->value();
                model->setData(index, value, Qt::EditRole);
            }
        }
        else
        {
            if(index.row() == 0)
            {
               QComboBox *comboBox = static_cast<QComboBox*>(editor);
               QString value = comboBox->currentText();
               model->setData(index,value, Qt::EditRole);
            }
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value, Qt::EditRole);
            }
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

    int set_thresMin(int index) const
    {
        if(index == 0)
        {
            return 0;
        }
        if(index == 1)
        {
            return 80;
        }
        if(index == 2)
        {
            return 90;
        }
        if(index == 3)
        {
            return 100;
        }
        if(index == 4)
        {
            return 105;
        }
        if(index == 5)
        {
            return 110;
        }
        if(index == 6)
        {
            return 115;
        }
        return 0;
    }

    int set_thresMax(int index) const
    {
        if(index == 0)
        {
            return 80;
        }
        if(index == 1)
        {
            return 90;
        }
        if(index == 2)
        {
            return 100;
        }
        if(index == 3)
        {
            return 105;
        }
        if(index == 4)
        {
            return 110;
        }
        if(index == 5)
        {
            return 115;
        }
        if(index == 6)
        {
            return 150;
        }
        return 0;
    }
};
#endif // DEL_EDITVALUES_H
