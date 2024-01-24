#include "result.h"
#include "ui_result.h"

result::result(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::result)
{
    ui->setupUi(this);
}

result::~result()
{
    delete ui;
}


void result::updateRet(std::vector<QStringList> ret)
{
    ui->tableWidget->setRowCount(ret.size());
    ui->tableWidget->setColumnCount(7);
    QStringList header;
    header <<"Class"<<"Images"<<"Instances"<<"P"<<"R"<<"mAP50"<<"mAP50-95";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    for(int i=0;i<ret.size();i++)
    {
        for(int j=0;j<ret[i].size();j++)
        {
            ui->tableWidget->setItem(i,j,new QTableWidgetItem(ret[i][j]));

        }
    }
}
