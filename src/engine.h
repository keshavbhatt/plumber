#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QStandardPaths>
#include <QSettings>
#include <QMessageBox>
#include <QtNetwork>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QPushButton>
#include <QProcess>

class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = nullptr);

signals:
    void  engineStatus(QString status);

    void  engineCacheCleared();

    void  errorMessage(QString errorMessage);

    void  openSettingsAndClickDownload();

    void  engineDownloadFailed(QString errorMessage);

    void  engineDownloadSucceeded();

public slots:
    void clearEngineCache();

    void download_engine_clicked();

private slots:
    bool checkEngine();

    void slot_netwManagerFinished(QNetworkReply *reply);

    void get_engine_version_info();

    void check_engine_updates();

    void compare_versions(QString date, QString n_date);

    void evoke_engine_check();

private:
    QFile *core_file = nullptr;

    QString core_local_date,core_remote_date;
};

#endif // ENGINE_H
