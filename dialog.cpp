#include "dialog.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    m_cfg.clsNum = "10";

    m_cfg.pretrain = "/opt/twizd/ultralytics-main/pretrain-s.pt";
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_selectButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/home");
    qDebug() << fileName;
    ui->pretrainWLineEdit->setText(fileName);
    m_cfg.pretrain = fileName;
}

void Dialog::on_epochSpinBox_valueChanged(const QString &arg1)
{
    m_cfg.epoch = arg1;
}

void Dialog::on_modelComboBox_currentIndexChanged(const QString &arg1)
{
    m_cfg.model = arg1;
}

void Dialog::on_clsNumSpinBox_valueChanged(const QString &arg1)
{
    m_cfg.clsNum = arg1;
}

void Dialog::on_buttonBox_accepted()
{
    QFile trainpy("/opt/twizd/ultralytics-main/train.py");
    trainpy.open(QIODevice::ReadWrite | QIODevice::Truncate);
    if(!trainpy.isOpen())
    {
        qDebug() << "open failed\n";
        return;
    }

    QString modelstr = "yolov8s";
    if(this->m_cfg.model == "XS")
    {
        modelstr = "yolov8n";
    }
    else if(this->m_cfg.model == "S")
    {
        modelstr = "yolov8s";
    }
    else if(this->m_cfg.model == "M")
    {
        modelstr = "yolov8m";
    }
    else if(this->m_cfg.model == "L")
    {
        modelstr = "yolov8l";
    }

    m_cfg.epoch = "1";


    QTextStream in(&trainpy);
    in << "from ultralytics import YOLO\n";
    in << "model = YOLO('" + modelstr + ".yaml').load('" + this->m_cfg.pretrain + "')\n";
    in << "results = model.train(data='/opt/config/data.yaml', device=[0],epochs=" + this->m_cfg.epoch + ", imgsz=416)\n";
    trainpy.close();
}
