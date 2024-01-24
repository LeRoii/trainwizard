#ifndef RESULT_H
#define RESULT_H

#include <QWidget>

namespace Ui {
class result;
}

class result : public QWidget
{
    Q_OBJECT

public:
    explicit result(QWidget *parent = nullptr);
    ~result();
    void updateRet(std::vector<QStringList> ret);

private:
    Ui::result *ui;
};

#endif // RESULT_H
