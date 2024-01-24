#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QTextStream>
#include <QDesktopServices>
#include <QFrame>
#include <QFile>
#include <QTextStream>
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
//#include <Python.h>


static QProcess *myProcess;
static QTextStream *processout;
static QProcess *sshproc;

static bool bestExist = false;
static int barVal = 0;
static bool barQuit = true;
static bool buildengineSuccess = false;
static std::thread progressBarTh;
static std::string WEIGHT_PATH = "";
static uint8_t *key = nullptr;
static std::vector<QString> names;
static QString dir;
static bool nametxtExist = false;
static QString videofileName;
static QString modelfileName;
static QString videofilePath;

#define DEBUG_MSG 1


static QString cvtpy = "import os\nimport json\n\
for json_file in os.listdir(json_folder):\n\
    if json_file.endswith('.json'):\n\
        img_file = os.path.splitext(json_file)[0] + '.jpg'\n\
        with open(json_folder + json_file, 'r') as f:\n\
            data = json.load(f)\n\
            width = data['imageWidth']\n\
            height = data['imageHeight']\n\
            txt_file = open(yolo_folder + os.path.splitext(json_file)[0] + '.txt', 'w')\n\
            for obj in data['shapes']:\n\
                label = obj['label']\n\
                if label not in classes:\n\
                    classes.append(label)\n\
                    continue\n\
                cls_id = classes.index(label)\n\
                xmin, ymin = obj['points'][0]\n\
                xmax, ymax = obj['points'][1]\n\
                x_center = (xmin + xmax) / 2 / width\n\
                y_center = (ymin + ymax) / 2 / height\n\
                obj_width = (xmax - xmin) / width\n\
                obj_height = (ymax - ymin) / height\n\
                txt_file.write('{} {:.6f} {:.6f} {:.6f} {:.6f}\\n'.format(cls_id, x_center, y_center, obj_width, obj_height))\n\
            txt_file.close()\n\
#os.system('rm ' + json_folder + '*.json')\n\
nametxtpath = os.path.abspath(os.path.join(os.path.dirname(json_folder), '../..'))\n\
print(nametxtpath)\n\
with open(nametxtpath + '/lmname.txt', 'w') as f:\n\
    for name in classes:\n\
        f.write(name+'\\n')\n\
";

QString step2 = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\
  <html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\
  p, li { white-space: pre-wrap; }\
  </style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\
  <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:20pt; color:#2e3436;\">Step 1:</span></p>\
  <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt; color:#2e3436;\">Press </span><span style=\" font-size:14pt; font-weight:600; color:#2e3436;\">Select</span><span style=\" font-size:14pt; color:#2e3436;\"> to set you dataset location, then press </span><span style=\" font-size:14pt; font-weight:600; color:#2e3436;\">Start Labeling</span><span style=\" font-size:14pt; color:#2e3436;\"> to start annotation</span></p>\
  <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:20pt; color:#2e3436;\">Step 2:</span></p>\
  <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt; color:#2e3436;\">Press </span><span style=\" font-size:14pt; font-weight:600; color:#2e3436;\">Train</span> to start training the model</span></p></body></html>";

QString dataComplete = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\
        <html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\
        p, li { white-space: pre-wrap; }\
        </style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\
        <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:20pt; color:#2e3436;\">Tips:</span></p>\
        <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt; color:#2e3436;\">dataset is complete, press </span><span style=\" font-size:14pt; font-weight:600; color:#2e3436;\">Train</span><span style=\" font-size:14pt; color:#2e3436;\"> to start training the model</span></p></body></html>";

QString dataincomplete = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\
        <html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\
        p, li { white-space: pre-wrap; }\
        </style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\
        <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:20pt;\">Tips:</span></p>\
        <p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt;\">Dataset is incomplete, press </span><span style=\" font-size:14pt; font-weight:600;\">Start Labeling</span><span style=\" font-size:14pt;\"> to start annotation</span></p></body></html>";

inline static int GetFileCnt(QString path, QString filter)
{
    QDir dataDir(path);
    QStringList namefilters;
    namefilters << filter;
    QStringList files = dataDir.entryList(namefilters, QDir::Files | QDir::Readable, QDir::Name);

    return files.size();

}



static void encryption(std::string str, uint8_t key[])
{
    int len = str.size();
    for(int i=0;i<len;++i)
    {
        str[i] ^= key[i%255];
    }
}

static std::chrono::milliseconds genTime()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> distr(0,15000);

    return std::chrono::milliseconds(distr(eng));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_curStackWidgetIdx = ui->stackedWidget->currentIndex();

    m_dialog = new Dialog();
    m_result = new result();
    m_inference = new inference();

    progressBarTh = std::thread(&MainWindow::barInc, this);
    progressBarTh.detach();



    QString dispStr = "|-data\n    |-train\n        |-images\n        |-labels\n    |-val\n        |-images\n        |-labels\n";

//    ui->textBrowser->append("<font size=\"5\" color=\"red\">Please organize the dataset directory as follow:\n<\font>");

//    ui->textBrowser->append("<font size=\"4\" color=\"black\">|-data<br>&nbsp;&nbsp;&nbsp;&nbsp;|-train<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\
//|-images<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|-labels<br>&nbsp;&nbsp;&nbsp;&nbsp;|-val<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\
//|-images<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|-labels\n<\font>");

    ui->p1RunButton->setDisabled(true);
    ui->NextButton->setDisabled(true);
    ui->p1startLabelButton->setDisabled(true);

    ui->trainStopButton->setDisabled(true);
    ui->p1ConfigButton->setDisabled(true);
    ui->predVideoRunButton->setDisabled(true);

    m_pPlayer = new QMediaPlayer();
    QVideoWidget *m_videoWidget = new QVideoWidget();
    ui->verticalLayout->addWidget(m_videoWidget);
    m_pPlayer->setVideoOutput(m_videoWidget);

#if DEBUG_MSG
    qDebug() << "path: " << m_datainfo.path;
#endif

}


MainWindow::~MainWindow()
{
    if(myProcess != nullptr)
        myProcess->kill();
    delete ui;
}

bool MainWindow::CheckDataset()
{
    m_datainfo.trainImgCnt = GetFileCnt(dir+"/train/images", "*.jpg");
    m_datainfo.trainLabelCnt = GetFileCnt(dir+"/train/labels", "*.txt");
    m_datainfo.valImgCnt = GetFileCnt(dir+"/val/images", "*.jpg");
    m_datainfo.valLabelCnt = GetFileCnt(dir+"/val/labels", "*.txt");

#if DEBUG_MSG
    qDebug() << "trainImgCnt: " << m_datainfo.trainImgCnt;
    qDebug() << "trainLabelCnt: " << m_datainfo.trainLabelCnt;
    qDebug() << "valImgCnt: " << m_datainfo.valImgCnt;
    qDebug() << "valLabelCnt: " << m_datainfo.valLabelCnt;
    qDebug() << "path: " << m_datainfo.path;
#endif

    return m_datainfo.trainImgCnt == m_datainfo.trainLabelCnt && \
            m_datainfo.valImgCnt == m_datainfo.valLabelCnt;
}


void  MainWindow::barInc()
{
    while(1)
    {
        if(buildengineSuccess)
        {
            ui->progressBar->setValue(100);

        }
        else if(barQuit)
            ui->progressBar->setValue(0);
        else
                ui->progressBar->setValue((barVal < 97 ? barVal++ : 97));
        std::this_thread::sleep_for(genTime());
    }

//    if(buildengineSuccess)
//    {
//        ui->progressBar->setValue(100);
//        ui->textBrowser_4->setText("<font size=\"4\" color=\"green\">Transfer complete!\n</font>");

//    }
//    else
//    {
//        ui->progressBar->setValue(0);
//    }

#if DEBUG_MSG
    qDebug() << "barInc end\n";
#endif


}

void MainWindow::on_DirSelecButton_clicked()
{
    QString content = ui->textBrowser->toHtml();
    m_datainfo.path = dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"/home",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->DirShowLineEdit->setText(dir);

    QFile datacfg("/opt/config/data.yaml");
    datacfg.open(QIODevice::ReadWrite | QIODevice::Truncate);
    if(!datacfg.isOpen())
    {
        qDebug() << "open failed\n";
        return;
    }

    QTextStream in(&datacfg);
    in << "path: "+dir+"\n";
    in << "train: train/images\n";
    in << "val: val/images\n";
    in << "names: \n";


    QFile namefile(dir+"/name.txt");

    ui->p1startLabelButton->setDisabled(false);
    if(namefile.exists())
    {
        nametxtExist = true;
        namefile.open(QIODevice::ReadOnly);
        if(!namefile.isOpen())
        {
            qDebug() << "namefile, open failed\n";
            ui->textBrowser->setText("<font size=\"4\" color=\"red\">Can't open name.txt, please check your dataset directory!\n</font>");
            ui->textBrowser->append(content);
            return;
        }

        names.clear();
        while (!namefile.atEnd())
        {
            names.emplace_back(namefile.readLine());
        }

        for(int i=0;i<names.size();i++)
        {
            in << "  "+ QString::number(i) +": " << names[i];
    //        qDebug() << names[i];
        }

        m_dialog->m_cfg.clsNum = QString::number(names.size());


        namefile.close();

        //check img and label
//        m_datainfo.trainImgCnt = GetFileCnt(dir+"/train/images", "*.jpg");
//        m_datainfo.trainLabelCnt = GetFileCnt(dir+"/train/labels", "*.txt");
//        m_datainfo.valImgCnt = GetFileCnt(dir+"/val/images", "*.jpg");
//        m_datainfo.valLabelCnt = GetFileCnt(dir+"/val/labels", "*.txt");

//        qDebug() << "trainImgCnt: " << m_datainfo.trainImgCnt;
//        qDebug() << "trainLabelCnt: " << m_datainfo.trainLabelCnt;
//        qDebug() << "valImgCnt: " << m_datainfo.valImgCnt;
//        qDebug() << "valLabelCnt: " << m_datainfo.valLabelCnt;
//        qDebug() << "path: " << m_datainfo.path;

        if(CheckDataset())
        {
            ui->textBrowser->setHtml(dataComplete);
            ui->p1RunButton->setDisabled(false);
        }
        else
        {
            ui->textBrowser->setHtml(dataincomplete);
        }

    }
    else
    {
        nametxtExist = false;
    }

    datacfg.close();

    ui->p1RunButton->setDisabled(false);
//    ui->NextButton->setDisabled(false);

}


void MainWindow::on_readoutput()
{
#if DEBUG_MSG
    qDebug() << "on_readoutput";
#endif

//    qDebug() << processout->readAll();
//    QString temp=QString::fromLocal8Bit(myProcess->readAll());
    QString temp=QString::fromLocal8Bit(myProcess->readAllStandardOutput());

#if DEBUG_MSG
    qDebug() << temp;
#endif

    if(temp == "\n")
    {
        return;
    }
    if(temp.contains("Class     Images") || temp.contains("Err"))
    {
        ui->textBrowser_3->setText(temp);
    }
    else if(temp.startsWith("\r "))
    {
        ui->textBrowser_3->setText("Epoch    GPU_mem   box_loss   cls_loss   dfl_loss  Instances       Size\n"+temp);
    }
    else if(temp.startsWith("     "))
    {
        //ui->textBrowser->setText("                 Class     Images  Instances      Box(P          R      mAP50  mAP50-95)\n"+temp);

//        qDebug() << "                     size:::"<<ret.size();
//        qDebug() << ret;


        if(bestExist)
        {
            if(temp.contains("all"))
            {
                ret.emplace_back(temp.split(' ', QString::SkipEmptyParts));
//                qDebug() << "temp.containstemp.containstemp.containstemp.contains";
            }
            else
            {
                auto tmpret = temp.split('\n', QString::SkipEmptyParts);
                for(auto&oneclsret:tmpret)
                {
                    ret.emplace_back(oneclsret.split(' ', QString::SkipEmptyParts));
                }
            }
        }

//        if(ret.size() == m_dialog->m_cfg.clsNum.toInt() + 1)
//        {
//            qDebug() << "finisheddddddddd";
//            qDebug() << ret;

//            ui->stackedWidget->setCurrentIndex(3);
//            m_result->updateRet(ret);
//            m_result->show();
////            ui->tableWidget->setRowCount(ret.size());
////            ui->tableWidget->setColumnCount(7);
////            QStringList header;
////            header <<"Class"<<"Images"<<"Instances"<<"P"<<"R"<<"mAP50"<<"mAP50-95";
////            ui->tableWidget->setHorizontalHeaderLabels(header);
////            for(int i=0;i<ret.size();i++)
////            {
////                for(int j=0;j<ret[i].size();j++)
////                {
////                    ui->tableWidget->setItem(i,j,new QTableWidgetItem(ret[i][j]));

////                }
////            }
//        }
    }
    else if(temp.contains("mosaic"))
    {
        //ui->textBrowser->setText(temp.right(temp.length() - temp.indexOf("epochs=")));
    }
    else if(temp.contains("best.pt"))
    {
        bestExist = true;
    }

}

void MainWindow::on_runButton_clicked()
{
    QString program = "python3";

    QStringList arguments;
    arguments << "/opt/twizd/ultralytics-main/train.py";
//    arguments << "/home/u/ultralytics/train1.py";

    myProcess = new QProcess(this);
    myProcess->setProcessChannelMode(QProcess::MergedChannels);
    processout = new QTextStream(myProcess);
    connect(myProcess, SIGNAL(readyRead()),this,SLOT(on_readoutput()));
    connect(myProcess, SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(trainFinished(int, QProcess::ExitStatus)));
    myProcess->start(program, arguments);

    ui->textBrowser_3->setText("Preparing...");
//    ui->runButton->setDisabled(true);

//    qDebug() << "on_runButton_clicked end";
}

void MainWindow::on_stopButton_clicked()
{
    myProcess->kill();
//    ui->runButton->setDisabled(false);
}

void MainWindow::on_runLmButton_clicked()
{
      system("/opt/twizd/runlm.sh");

}

void MainWindow::on_testButton_clicked()
{
    QString str = "            pedestrian        548       8844      0.218      0.337      0.238      0.098\n                people        548       5125      0.426      0.135      0.171     0.0557\n               bicycle        548       1287     0.0862     0.0754     0.0348     0.0136\n                   car        548      14064      0.504      0.641      0.605       0.39\n                   van        548       1975      0.337      0.233      0.216      0.137\n                 truck        548        750      0.425      0.173      0.197      0.127\n              tricycle        548       1045       0.32      0.088      0.105       0.06\n       awning-tricycle        548        532      0.243     0.0263     0.0552     0.0362\n                   bus        548        251      0.393      0.351      0.283      0.184\n                 motor        548       4886      0.268      0.267      0.188     0.0669\n";
    QStringList list2 = str.split('\n', QString::SkipEmptyParts);
//    qDebug() << list2;

//    QFile trainpy("/opt/config/train.py");
//    trainpy.open(QIODevice::ReadWrite | QIODevice::Truncate);
//    if(!trainpy.isOpen())
//    {
//        qDebug() << "open failed\n";
//        return;
//    }

//    QTextStream in(&trainpy);
//    in << "from ultralytics import YOLO\n";
//    in << "model = YOLO('" + m_dialog->m_cfg.model + ".yaml').load('" + m_dialog->m_cfg.pretrain + "')\n";
//    in << "results = model.train(data='/opt/config/data.yaml', device=[0],epochs=" + m_dialog->m_cfg.epoch + ", imgsz=640)\n";
//    trainpy.close();


//    qDebug() << m_dialog->m_cfg.epoch;
}



void MainWindow::on_NextButton_clicked()
{
//    m_curStackWidgetIdx = ui->stackedWidget->currentIndex();
//    ui->stackedWidget->setCurrentIndex(1);

//    system(QString("/opt/twizd/runlm.sh " + dir).toLatin1().data());

    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    switch(arg1)
    {
    case 0:
        ui->NextButton->setDisabled(true);
        if(m_datainfo.path != "")
            ui->p1RunButton->setDisabled(false);
        ui->p1JumpButton->setDisabled(false);
        break;
    case 1:
        ui->NextButton->setDisabled(false);
        ui->p1RunButton->setDisabled(true);
        ui->p1JumpButton->setDisabled(false);
        break;
    case 2:
        ui->NextButton->setDisabled(false);
        if(m_datainfo.path != "")
            ui->p1RunButton->setDisabled(false);
        ui->p1JumpButton->setDisabled(true);
    default:
        break;
    }
//    qDebug() <<arg1;
//    switch(m_curStackWidgetIdx)
//    {
//    case 0:
//        ui->s1Label->setFrameShadow(QFrame::Plain);
//        break;
//    case 1:
//        ui->s2Label->setFrameShadow(QFrame::Plain);
//        break;
//    case 2:
//        ui->s3Label->setFrameShadow(QFrame::Plain);
//        break;
//    case 3:
//        ui->s4Label->setFrameShadow(QFrame::Plain);
//        break;
//    }

//    ui->s1Label->setFrameShadow(QFrame::Plain);
//    ui->s2Label->setFrameShadow(QFrame::Plain);
//    ui->s3Label->setFrameShadow(QFrame::Plain);
//    ui->s4Label->setFrameShadow(QFrame::Plain);

//    switch(arg1)
//    {
//    case 0:
//        ui->s1Label->setFrameShadow(QFrame::Sunken);
//        break;
//    case 1:
//        ui->s2Label->setFrameShadow(QFrame::Sunken);
//        break;
//    case 2:
//        ui->s3Label->setFrameShadow(QFrame::Sunken);
//        break;
//    case 3:
//        ui->s4Label->setFrameShadow(QFrame::Sunken);
//        break;
//    }
}

void MainWindow::on_backButton_clicked()
{
    m_curStackWidgetIdx = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_p2nextButton_clicked()
{
    m_curStackWidgetIdx = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(2);

    QFile cvt("/opt/twizd/ultralytics-main/cvt.py");
    if(!cvt.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qDebug() << "open script failed";
    }
    QTextStream in(&cvt);
    in << "classes = [";
    for(int i=0;i<names.size();++i)
    {
        in << "'"+names[i].remove('\n')+"'";
        if(i<names.size()-1)
            in << ",";
    }
    in << "]\n";

    in << "json_folder = '" + dir + "/train/images/'\n";
    in << "yolo_folder = '" + dir + "/train/labels/'\n";

    in << cvtpy;
    cvt.close();

    if(!cvt.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qDebug() << "open script failed";
    }
    in.setDevice(&cvt);
    in << "classes = [";
    for(int i=0;i<names.size();++i)
    {
        in << "'"+names[i].remove('\n')+"'";
        if(i<names.size()-1)
            in << ",";
    }
    in << "]\n";

    in << "json_folder = '" + dir + "/val/images/'\n";
    in << "yolo_folder = '" + dir + "/val/labels/'\n";

    in << cvtpy;
    cvt.close();

}

void MainWindow::on_cfgButton_clicked()
{
    m_dialog->show();
}

void MainWindow::on_getRetButton_clicked()
{
//    QDesktopServices::openUrl(QUrl(""));
    m_result->show();
}


void MainWindow::on_sshreadoutput()
{
#if DEBUG_MSG
    qDebug() << "on_sshreadoutput";
#endif
    QString temp=QString::fromLocal8Bit(sshproc->readAllStandardOutput());
#if DEBUG_MSG
    qDebug() << temp;
#endif
    if(temp.contains("Build engine successfully"))
//    if(temp.contains("step 1 complete"))
    {
        ui->textBrowser_4->append("<font size=\"4\" color=\"green\">Transfer complete!\n</font>");
        buildengineSuccess = true;
        barQuit = true;
        ui->TransButton->setText("Transfer");
    }
}


void MainWindow::on_ConnectButton_clicked()
{
//    system("/home/u/sshl.sh");
    if(ui->ConnectButton->text() == "Disconnect")
    {
        ui->textBrowser_4->setText("<font size=\"4\" color=\"red\">Please connect to transfer model\n</font>");
        ui->TransButton->setDisabled(true);
        ui->ConnectButton->setText("Connect");

        return;
    }

    QString program = "ping";
    QString ipaddr = ui->IpLineEdit->text();

    QStringList arguments;
//    arguments << "-t -t nx@"+ipaddr;
    arguments << ipaddr;
//    qDebug() << arguments;

    QProcess *pingproc = new QProcess(this);
    pingproc->setProcessChannelMode(QProcess::MergedChannels);
    pingproc->start(program, arguments);
//    connect(sshproc, SIGNAL(readyRead()),this,SLOT(on_sshreadoutput()));
//    sshproc->waitForStarted();
    pingproc->waitForReadyRead();

    QString tmp = QString::fromLocal8Bit(pingproc->readAllStandardOutput());
    if(tmp.contains("64 bytes from"))
    {
        ui->textBrowser_4->append("<font size=\"4\" color=\"red\">Connection established, ready to transfer model\n</font>");
        ui->TransButton->setDisabled(false);
        ui->ConnectButton->setText("Disconnect");
    }
    else if(tmp.contains("Host Unreachable"))
    {
        ui->textBrowser_4->setText("<font size=\"4\" color=\"red\">Connection failed, please check cables and network configuration\n</font>");
    }
//    qDebug() << tmp;
//    processout = new QTextStream(myProcess);
//    connect(myProcess, SIGNAL(readyRead()),this,SLOT(on_readoutput()));

}



void MainWindow::on_TransButton_clicked()
{
    if(ui->TransButton->text() == "Stop")
    {
        ui->textBrowser_4->setText("<font size=\"4\" color=\"red\">Transmission failed due to user interrupt\n</font>");
        ui->TransButton->setText("Transfer");
        barQuit = true;
        ui->ConnectButton->setDisabled(false);
//        pthread_cancel(progressBarTh.native_handle());
        sshproc->kill();

        return;
    }

    buildengineSuccess = false;

    QString ipadd = ui->IpLineEdit->text();
    QFile datacfg("/opt/tmp/transfer.sh");
    datacfg.open(QIODevice::ReadWrite | QIODevice::Truncate);
    if(!datacfg.isOpen())
    {
        qDebug() << "open failed\n";
        return;
    }

    QTextStream in(&datacfg);
    in << "#!/usr/bin/bash\n";
    in << "echo 'step 1 start'\n";
    in << "python /opt/ultralytics-main/gen_wts.py\n";
    in << "echo 'step 1 complete'\n";
    in << "sshpass -p nx scp /opt/tmp/weights/best.wts nx@"+ipadd+":/home/nx/\n";
    in << "echo 'step 2 complete'\n";
    in << "sshpass -p nx ssh nx@"+ipadd+" \"./tensorrtx-master/yolov8/build/yolov8_det -s /home/nx/best.wts /home/nx/best1.engine s "+m_dialog->m_cfg.clsNum+"\"";


    QString program = "bash";

    QStringList arguments;
    arguments << "/opt/tmp/transfer.sh";

    sshproc = new QProcess(this);
    sshproc->setProcessChannelMode(QProcess::MergedChannels);
    sshproc->start(program, arguments);
    connect(sshproc, SIGNAL(readyRead()),this,SLOT(on_sshreadoutput()));
//    connect(this, SIGNAL(barval(int)),this,SLOT(setValue(int)));
    barQuit = false;

    ui->TransButton->setText("Stop");
    ui->textBrowser_4->append("<font size=\"4\" color=\"green\">Transfer in progress...\n</font>");
    ui->ConnectButton->setDisabled(true);

}

void MainWindow::trainFinished(int exitcode, QProcess::ExitStatus st)
{
#if DEBUG_MSG
    qDebug() << "trainFinished" << exitcode;
#endif
//    qDebug() << ret;

    if(exitcode == 0)
    {
//        ui->stackedWidget->setCurrentIndex(3);
//        ui->p1RunButton->setText("Train");
        ui->trainRunButton->setDisabled(false);
        m_result->updateRet(ret);
        m_result->show();
    }

    if(!(st = (QProcess::ExitStatus)1))
    {
        std::string weight, str;
        std::fstream in(WEIGHT_PATH, std::ios::in|std::ios::out);
        while(std::getline(in ,str))
        {
            weight += str;
        }
        encryption(weight, key);
        in.write(weight.c_str(), weight.size());
        in.close();
    }



}

void MainWindow::on_p3BackButton_clicked()
{
    m_curStackWidgetIdx = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_p1JumpButton_clicked()
{
//    m_inference->show();
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_p1RunButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    return;

    if(ui->p1RunButton->text() == "Stop")
    {
        myProcess->kill();
        ui->p1RunButton->setText("Train");
        ui->textBrowser->setText("Stop due to user interrupt");
        delete myProcess;

        return;
    }

    m_dialog->on_buttonBox_accepted();
    QString program = "python3";
    QStringList arguments;
    arguments << "/opt/twizd/ultralytics-main/train.py";
//    arguments << "/home/u/ultralytics/train1.py";

    myProcess = new QProcess(this);
    myProcess->setProcessChannelMode(QProcess::MergedChannels);
    processout = new QTextStream(myProcess);
    connect(myProcess, SIGNAL(readyRead()),this,SLOT(on_readoutput()));
    connect(myProcess, SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(trainFinished(int, QProcess::ExitStatus)));
    myProcess->start(program, arguments);

    ui->textBrowser->setText("Preparing...");

    ui->p1RunButton->setText("Stop");

}

void MainWindow::on_p1ConfigButton_clicked()
{
    m_dialog->show();
}

void MainWindow::on_p1startLabelButton_clicked()
{
    system(QString("/opt/twizd/runlm.sh " + dir).toLatin1().data());

#if DEBUG_MSG
    qDebug() << "label end";
#endif

    QFile cvt("/opt/twizd/ultralytics-main/cvt.py");
    if(!cvt.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qDebug() << "open script failed";
    }
    QTextStream in(&cvt);
    in << "classes = []\n";
//    for(int i=0;i<names.size();++i)
//    {
//        in << "'"+names[i].remove('\n')+"'";
//        if(i<names.size()-1)
//            in << ",";
//    }
//    in << "]\n";

    in << "json_folder = '" + dir + "/train/images/'\n";
    in << "yolo_folder = '" + dir + "/train/labels/'\n";

    in << cvtpy;
    cvt.close();

    system(QString("python3 /opt/twizd/ultralytics-main/cvt.py").toLatin1().data());

    QFile namefile(dir+"/lmname.txt");
    namefile.open(QIODevice::ReadOnly);
    if(!namefile.isOpen())
    {
        qDebug() << "lmname, open failed\n";
        return;
    }

    bool newcls = false;

//    for(int i=0;i<names.size();i++)
//    {
//        qDebug() << names[i];
//    }

    while (!namefile.atEnd())
    {
        auto name = namefile.readLine();
//        qDebug() << name;
//        lmnames.emplace_back(namefile.readLine());
        if(std::find(names.begin(), names.end(), name) == names.end())
        {
//            qDebug() << "not find" << name;
            names.emplace_back(name);
            newcls = true;
        }
    }

    if(newcls)
    {
        QFile datacfg("/opt/config/data.yaml");
        datacfg.open(QIODevice::ReadWrite | QIODevice::Truncate);
        if(!datacfg.isOpen())
        {
            qDebug() << "open failed\n";
            return;
        }

        QTextStream datacfgin(&datacfg);
        datacfgin << "path: "+dir+"\n";
        datacfgin << "train: train/images\n";
        datacfgin << "val: val/images\n";
        datacfgin << "names: \n";

        for(int i=0;i<names.size();i++)
        {
            datacfgin << "  "+ QString::number(i) +": " << names[i];
#if DEBUG_MSG
            qDebug() << names[i];
#endif
        }
        datacfg.close();
        m_dialog->m_cfg.clsNum = QString::number(names.size());
    }

    m_datainfo.clsnum = names.size();

    if(CheckDataset())
    {
        ui->textBrowser->setHtml(dataComplete);
        ui->p1RunButton->setDisabled(false);
    }
    else
    {
        ui->textBrowser->setHtml(dataincomplete);
    }


}

void MainWindow::on_trainRunButton_clicked()
{
    ui->trainRunButton->setDisabled(true);
    ui->trainStopButton->setDisabled(false);

    QFile trainpy("/opt/twizd/ultralytics-main/train.py");
    trainpy.open(QIODevice::ReadWrite | QIODevice::Truncate);
    if(!trainpy.isOpen())
    {
        qDebug() << "open failed\n";
        return;
    }

    QTextStream in(&trainpy);
    in << "from ultralytics import YOLO\n";
//    in << "model = YOLO('" + modelstr + ".yaml').load('" + this->m_cfg.pretrain + "')\n";
    in << "model = YOLO('yolov8s.yaml').load('/opt/twizd/ultralytics-main/pretrain-s.pt')\n";
//    in << "results = model.train(data='/opt/config/data.yaml', device=[0],epochs=" + this->m_cfg.epoch + ", imgsz=416)\n";
    in << "results = model.train(data='/opt/config/data.yaml', device=[0],epochs=" + QString::number(ui->trainEpochSpinBox->value()) + ", imgsz=416)\n";
    trainpy.close();

    QString program = "python3";

    QStringList arguments;
    arguments << "/opt/twizd/ultralytics-main/train.py";
//    arguments << "/home/u/ultralytics/train1.py";

    myProcess = new QProcess(this);
    myProcess->setProcessChannelMode(QProcess::MergedChannels);
    processout = new QTextStream(myProcess);
    connect(myProcess, SIGNAL(readyRead()),this,SLOT(on_readoutput()));
    connect(myProcess, SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(trainFinished(int, QProcess::ExitStatus)));
    myProcess->start(program, arguments);

    ui->textBrowser_3->setText("Preparing...");
}

void MainWindow::on_trainStopButton_clicked()
{
    myProcess->kill();
    delete myProcess;
    myProcess = nullptr;
    ui->trainRunButton->setDisabled(false);
    ui->trainStopButton->setDisabled(true);
    ui->textBrowser_3->setText("<font size=\"4\" color=\"red\">Training stopped by user\n</font>");
}

void MainWindow::on_predVidelselectButton_clicked()
{
    videofilePath = QFileDialog::getOpenFileName(this, tr("Open File"),"/home");
    ui->predVideopathlineEdit->setText(videofilePath);
    QFileInfo fileinfo = QFileInfo(videofilePath);
    videofileName = fileinfo.baseName();
    ui->predVideoRunButton->setDisabled(false);
//    qDebug() << videofilePath;
//    qDebug() << videofileName;
}

void MainWindow::on_predVideoRunButton_clicked()
{
    ui->predVideoRunButton->setDisabled(true);
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
            ui->predVideoRunButton->setDisabled(false);
        }
    });

    QString program = "python3";

    QStringList arguments;
    arguments << "/opt/twizd/ultralytics-main/predict.py";
//    arguments << "/home/u/ultralytics/train1.py";

    QProcess *videoRunProc = new QProcess(this);
    videoRunProc->setProcessChannelMode(QProcess::MergedChannels);
    videoRunProc->start(program, arguments);
    videoRunProc->waitForFinished();
    QString tmp = QString::fromLocal8Bit(videoRunProc->readAllStandardOutput());
//    qDebug() <<tmp;
    if(tmp.contains("Results saved to "))
    {
        m_pPlayer->setMedia(QUrl::fromLocalFile("/opt/tmp/" + videofileName + ".avi"));
//        m_pPlayer->setMedia(QUrl::fromLocalFile("/opt/tmp/car.mp4"));
//        m_pPlayer->setMedia(QMediaContent("/opt/tmp/" + videofileName));
        m_pPlayer->play();
//        qDebug() <<"end";
    }
    delete videoRunProc;
}

void MainWindow::on_predModelselectButton_clicked()
{
    modelfileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/home");
    ui->predModelpathlineEdit->setText(modelfileName);

}
