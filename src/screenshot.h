#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class Screenshot;
}

class Screenshot : public QWidget
{
    Q_OBJECT

public:
    explicit Screenshot(QWidget *parent = nullptr, QString image_location="");
    ~Screenshot();
    bool savedSc = false;
signals:
    void savedScreenshot(QString fileLocation);
    void failedToSaveSc();

protected slots:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_filename_textChanged(const QString &arg1);

    void on_save_clicked();

private:
    Ui::Screenshot *ui;
    QSettings settings;
    QString image_location,filenameComplete,ext;
};

#endif // SCREENSHOT_H
