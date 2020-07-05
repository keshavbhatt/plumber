#include "screenshot.h"
#include "ui_screenshot.h"
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

Screenshot::Screenshot(QWidget *parent,QString image_location) :
    QWidget(parent),
    ui(new Ui::Screenshot)
{
    ui->setupUi(this);
    this->image_location = image_location;
    QPixmap pix(image_location);
    int dotPos = image_location.lastIndexOf(".",-1);
    QString fileName = QUrl(image_location.left(dotPos)).fileName();
    QString ext = image_location.split(fileName).last();
    ui->save->setEnabled(false);
    if(pix.isNull()==false){
//        ui->screenshotView->setPixmap(pix);
        ui->screenshotView->setPixmap(pix.scaled(ui->screenshotView->minimumSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        ui->filename->setText(fileName);
        ui->ext->setText(ext);
    }else{
        ui->screenshotView->setAlignment(Qt::AlignCenter);
        ui->screenshotView->setText("Invalid Image");
    }

    filenameComplete = ui->filename->text().trimmed().simplified()+ui->ext->text();
    ext = ui->ext->text();
}

Screenshot::~Screenshot()
{
    delete ui;
}

void Screenshot::on_filename_textChanged(const QString &arg1)
{
    ui->save->setEnabled(!arg1.trimmed().isEmpty());
}

void Screenshot::on_save_clicked()
{
    QString path = settings.value("sc_location",
                   QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                   .toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image File"), path+"/"+filenameComplete, QString("ImageFile (*"+ext+")"));
    if (!fileName.isEmpty())
    {
       settings.setValue("sc_location",QFileInfo(fileName).path());
       QPixmap pix(image_location);
       if(pix.save(fileName,QString(ui->ext->text().split(".").last()).toUpper().toUpper().toLocal8Bit().data())){
           savedSc = true;
           emit savedScreenshot(fileName);
           this->close();
       }else{
           QMessageBox::warning(this, tr("Screenshot save error"),
                                tr("Save operation failed."));
       }
    }else {
        QMessageBox::warning(this, tr("Application"),
                             tr("Save operation cancelled."));
    }
}

void Screenshot::closeEvent(QCloseEvent *event)
{
    if(!savedSc){
        emit failedToSaveSc();
    }
    QWidget::closeEvent(event);
}
