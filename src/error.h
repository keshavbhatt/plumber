#ifndef ERROR_H
#define ERROR_H

#include <QWidget>

namespace Ui {
class Error;
}

class Error : public QWidget
{
    Q_OBJECT

public:
    explicit Error(QWidget *parent = nullptr);
    ~Error();

public slots:
    void setError(QString shortMessage, QString detail);
private slots:
    void on_ok_clicked();

private:
    Ui::Error *ui;
};

#endif // ERROR_H
