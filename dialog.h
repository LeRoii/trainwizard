#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
enum class enModel
{
    V8N,
    V8S,
    V8M,
    V8L,
    V8X,
};

struct stTrainCfg
{
    QString epoch;
    QString clsNum;
    QString model;
    QString pretrain;

};

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    stTrainCfg m_cfg;
public slots:
    void on_buttonBox_accepted();

private slots:
    void on_selectButton_clicked();

    void on_epochSpinBox_valueChanged(const QString &arg1);

    void on_modelComboBox_currentIndexChanged(const QString &arg1);

    void on_clsNumSpinBox_valueChanged(const QString &arg1);



private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
