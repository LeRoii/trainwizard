#ifndef INFERENCE_H
#define INFERENCE_H

#include <QWidget>
#include <QtMultimedia>
#include <QVideoWidget>

namespace Ui {
class inference;
}

class inference : public QWidget
{
    Q_OBJECT

public:
    explicit inference(QWidget *parent = nullptr);
    ~inference();

private slots:
    void on_p1SelectButton_clicked();

    void on_p1RunButton_clicked();

private:
    Ui::inference *ui;
    QMediaPlayer *m_pPlayer;
};

#endif // INFERENCE_H
