#include "error.h"
#include "ui_error.h"

Error::Error(QWidget *parent) : QWidget(parent), ui(new Ui::Error) {
  ui->setupUi(this);
}

void Error::setError(QString shortMessage, QString detail) {
  ui->detail->setText(detail);
  ui->message->setText(shortMessage);
}

Error::~Error() { delete ui; }

void Error::on_ok_clicked() { this->close(); }
