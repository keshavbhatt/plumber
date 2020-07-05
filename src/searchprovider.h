#ifndef SEARCHPROVIDER_H
#define SEARCHPROVIDER_H

#include <QWidget>
#include <QDebug>

#include "request.h"
#include "waitingspinnerwidget.h"
#include "error.h"
#include "ui_track.h"

namespace Ui {
class SearchProvider;
}

class SearchProvider : public QWidget
{
    Q_OBJECT

public:
    explicit SearchProvider(QWidget *parent = nullptr);
    ~SearchProvider();

signals:
    void loadVideo(QString videoId);

private slots:
    void on_term_textChanged(const QString &arg1);

    void on_search_clicked();

    void showError(QString message);

    void processResult(QString reply);

    void on_term_returnPressed();

private:
    Ui::SearchProvider *ui;
    Ui::track track_ui;


    Request * _request = nullptr;
    WaitingSpinnerWidget *_loader = nullptr;
    Error * _error = nullptr;



};

#endif // SEARCHPROVIDER_H
