#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialog.h"
#include "result.h"
#include "inference.h"
#include <QProcess>
#include <QtMultimedia>
#include <QVideoWidget>


namespace Ui {
class MainWindow;
}


struct stDataSetInfo
{
    int trainImgCnt;
    int trainLabelCnt;
    int valImgCnt;
    int valLabelCnt;
    QString path;
    int clsnum;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void barInc();

signals:
    void barval(int);

private slots:
    void on_DirSelecButton_clicked();

    void on_readoutput();

    void on_runButton_clicked();

    void on_stopButton_clicked();

    void on_runLmButton_clicked();

    void on_testButton_clicked();

    void on_NextButton_clicked();

    void on_stackedWidget_currentChanged(int arg1);

    void on_backButton_clicked();

    void on_p2nextButton_clicked();

    void on_cfgButton_clicked();

    void on_getRetButton_clicked();

    void on_ConnectButton_clicked();

    void on_sshreadoutput();

    void on_TransButton_clicked();

    void trainFinished(int exitcode, QProcess::ExitStatus st);

    void on_p3BackButton_clicked();

    void on_p1JumpButton_clicked();

    void on_p1RunButton_clicked();

    void on_p1ConfigButton_clicked();

    void on_p1startLabelButton_clicked();

    void on_trainRunButton_clicked();

    void on_trainStopButton_clicked();

    void on_predVidelselectButton_clicked();

    void on_predVideoRunButton_clicked();

    void on_predModelselectButton_clicked();

private:

    bool CheckDataset();
    Ui::MainWindow *ui;
    int m_curStackWidgetIdx;
    Dialog *m_dialog;
    result *m_result;
    inference *m_inference;
    std::vector<QStringList> ret;

    stDataSetInfo m_datainfo;
    QMediaPlayer *m_pPlayer;

};

#endif // MAINWINDOW_H
