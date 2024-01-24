#include "inference.h"
#include "ui_inference.h"
#include <QFileDialog>
#include <QDebug>
#include <QProcess>


static QString videofileName;
static QString videofilePath;
inference::inference(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::inference)
{
    ui->setupUi(this);

    m_pPlayer = new QMediaPlayer();

    QVideoWidget *m_videoWidget = new QVideoWidget();

    ui->verticalLayout->addWidget(m_videoWidget);


    m_pPlayer->setVideoOutput(m_videoWidget);
//    m_pPlayer->setMedia(QUrl::fromLocalFile(QFileInfo("/opt/tmp/car.mp4").absoluteFilePath()));
//    m_pPlayer->setMedia(QUrl::fromLocalFile("/opt/tmp/car.avi"));
//    m_pPlayer->setMedia(QUrl("/opt/tmp/car.mp4"));

}

inference::~inference()
{
    delete ui;
}

void inference::on_p1SelectButton_clicked()
{
    videofilePath = QFileDialog::getOpenFileName(this, tr("Open File"),"/home");
    ui->videoLineEdit->setText(videofilePath);
    QFileInfo fileinfo = QFileInfo(videofilePath);
    videofileName = fileinfo.baseName();
    qDebug() << videofilePath;
    qDebug() << videofileName;
}

void inference::on_p1RunButton_clicked()
{
//    m_pPlayer->setMedia(QUrl::fromLocalFile("/opt/tmp/car.avi"));
//    m_pPlayer->play();
//    return ;
    ui->p1RunButton->setDisabled(true);
    repaint();
    QFile pred("/opt/twizd/ultralytics-main/predict.py");
    if(!pred.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qDebug() << "open script failed";
    }
    QTextStream in(&pred);

    in << "from ultralytics import YOLO\n";
    in << "model = YOLO('/opt/tmp/weights/best.pt')\n";
    in << "model.predict('" + videofilePath + "', save=True)\n";
    pred.close();

    connect(m_pPlayer, &QMediaPlayer::mediaStatusChanged, [=]()
    {
        if(m_pPlayer->mediaStatus() == QMediaPlayer::EndOfMedia)
        {
            qDebug() << "enddddd";
            ui->p1RunButton->setDisabled(false);
        }
    });

    QString program = "python3";

    QStringList arguments;
    arguments << "/opt/twizd/ultralytics-main/predict.py";
//    arguments << "/home/u/ultralytics/train1.py";

    QProcess *myProcess = new QProcess(this);
    myProcess->setProcessChannelMode(QProcess::MergedChannels);
    myProcess->start(program, arguments);
//    myProcess->waitForReadyRead();
    myProcess->waitForFinished();
    QString tmp = QString::fromLocal8Bit(myProcess->readAllStandardOutput());
//    qDebug() <<tmp;
    if(tmp.contains("Results saved to "))
    {
        m_pPlayer->setMedia(QUrl::fromLocalFile("/opt/tmp/" + videofileName + ".avi"));
//        m_pPlayer->setMedia(QUrl::fromLocalFile("/opt/tmp/car.mp4"));
//        m_pPlayer->setMedia(QMediaContent("/opt/tmp/" + videofileName));
        m_pPlayer->play();
//        qDebug() <<"end";
    }
    delete myProcess;

}
