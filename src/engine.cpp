#include "engine.h"

Engine::Engine(QObject *parent) : QObject(parent)
{
    QTimer::singleShot(1000, [this]() {
        if(!checkEngine()){
            evoke_engine_check();
            return;
        }else{
            check_engine_updates();
        }
    });
}

bool Engine::checkEngine(){
    QString setting_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo checkFile(setting_path+"/core");
    bool present = false;
    if(checkFile.exists()&&checkFile.size()>0){
        emit engineStatus("Present");
        present = true;
    }else{
        emit engineStatus("Absent");
        present = false;
    }
    return present;
}

void Engine::download_engine_clicked()
{
//    settingsUi.download_engine->setEnabled(false);
    emit engineStatus("Downloading...");
    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir(addin_path);
    if (!dir.exists())
    dir.mkpath(addin_path);

    QString filename = "core";
    core_file =  new QFile(addin_path+"/"+filename ); //addin_path
    if(!core_file->open(QIODevice::ReadWrite | QIODevice::Truncate)){
        qDebug()<<"Could not open a file to write.";
    }

    QNetworkAccessManager *m_netwManager = new QNetworkAccessManager(this);
    connect(m_netwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished(QNetworkReply*)));
    QUrl url("http://yt-dl.org/downloads/latest/youtube-dl");
    QNetworkRequest request(url);
    m_netwManager->get(request);
}

void Engine::slot_netwManagerFinished(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError){
        // Get the http status code
        int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (v >= 200 && v < 300) // Success
        {
            if(reply->error() == QNetworkReply::NoError){
                core_file->write(reply->readAll());
                core_file->close();
                clearEngineCache(); //call clear engine cache after engine update
                get_engine_version_info();
                checkEngine();
                emit engineDownloadSucceeded();
            }else{
                core_file->remove();
            }

        }
        else if (v >= 300 && v < 400) // Redirection
        {
            // get the redirection url
            QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            // because the redirection url can be relative  we need to use the previous one to resolve it
            newUrl = reply->url().resolved(newUrl);
            QNetworkAccessManager *manager = new QNetworkAccessManager();
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(slot_netwManagerFinished(QNetworkReply*))); //keep requesting until reach final url
            manager->get(QNetworkRequest(newUrl));
        }
    }
    else //error
    {
        QString err = reply->errorString();
        if(err.contains("not")){ //to hide "Host yt-dl.org not found"
        emit engineStatus("Host not Found");}
        else if(err.contains("session")||err.contains("disabled")){
            emit engineStatus(err);
        }
        emit engineDownloadFailed(err);
        emit engineDownloadSucceeded(); //fake UI fixer
        reply->manager()->deleteLater();
    }
    reply->deleteLater();
}

//funtion used to clear engine cache, to prevent 403 and 429 issue.
void Engine::clearEngineCache(){
    QProcess *clear_engine_cache = new QProcess(this);
    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    connect(clear_engine_cache, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), [clear_engine_cache, this](int exitCode,QProcess::ExitStatus exitStatus) {
        if(checkEngine()==true){
            if(exitCode==0){
                emit engineCacheCleared();
            }
            else{
                emit errorMessage(exitStatus+" | "+clear_engine_cache->readAll());
            }
        }else{
            emit errorMessage("Engine not present download engine first");
            emit engineStatus("Absent");
            emit engineCacheCleared();//fake UI fixer
        }

    });
    clear_engine_cache->start("python",QStringList()<<addin_path+"/core"<<"--rm-cache-dir");
    if(clear_engine_cache->waitForStarted(1000)){
        clear_engine_cache->start("python3",QStringList()<<addin_path+"/core"<<"--rm-cache-dir");
    }
}

//writes core_version file with version info after core downloaded
void Engine::get_engine_version_info(){
    QNetworkAccessManager *m_netwManager = new QNetworkAccessManager(this);
    connect(m_netwManager,&QNetworkAccessManager::finished,[=](QNetworkReply* rep){
        if(rep->error() == QNetworkReply::NoError){
            QString addin_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            QDir dir(addin_path);
            if (!dir.exists())
            dir.mkpath(addin_path);

            QString filename = "core_version";
            QFile *core_version_file =  new QFile(addin_path+"/"+filename ); //addin_path
            if(!core_version_file->open(QIODevice::ReadWrite | QIODevice::Truncate)){
                qDebug()<<"Could not open a core_version_file to write.";
            }
            core_version_file->write(rep->readAll());
            core_version_file->close();
            core_version_file->deleteLater();
        }
        rep->deleteLater();
        m_netwManager->deleteLater();
    });
    QUrl url("https://rg3.github.io/youtube-dl/update/LATEST_VERSION");
    QNetworkRequest request(url);
    m_netwManager->get(request);
}

void Engine::check_engine_updates(){

    //read version from local core_version file
    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFile *core_version_file =  new QFile(addin_path+"/"+"core_version" );
    if (!core_version_file->open(QIODevice::ReadOnly | QIODevice::Text)){
        core_local_date = "2019.01.01";
        core_remote_date = QDate::currentDate().toString(Qt::ISODate);
        compare_versions(core_local_date,core_remote_date);
        return;
    }
    core_local_date  = core_version_file->readAll().trimmed();

    //read version from remote
    QNetworkAccessManager *m_netwManager = new QNetworkAccessManager(this);
    connect(m_netwManager,&QNetworkAccessManager::finished,[=](QNetworkReply* rep){
        if(rep->error() == QNetworkReply::NoError){
             core_remote_date = rep->readAll().trimmed();
             if(!core_local_date.isNull() && !core_remote_date.isNull()){
                compare_versions(core_local_date,core_remote_date);
             }
        }
        rep->deleteLater();
        m_netwManager->deleteLater();
    });
    QUrl url("https://rg3.github.io/youtube-dl/update/LATEST_VERSION");
    QNetworkRequest request(url);
    m_netwManager->get(request);
}

void Engine::compare_versions(QString date,QString n_date){

    int year,month,day,n_year,n_month,n_day;

    year = QDate::fromString(date,Qt::ISODate).year();
    month = QDate::fromString(date,Qt::ISODate).month();
    day = QDate::fromString(date,Qt::ISODate).day();

    n_year = QDate::fromString(n_date,Qt::ISODate).year();
    n_month = QDate::fromString(n_date,Qt::ISODate).month();
    n_day = QDate::fromString(n_date,Qt::ISODate).day();

    bool update= false;

    if(n_year>year || n_month>month || n_day>day ){
       update=true;
    }

    if(update){
        QMessageBox msgBox;
          msgBox.setText(""+QApplication::applicationName()+" requires an updated version of its video download engine. Would you like to install the updated version now?");
          msgBox.setIconPixmap(QPixmap(":/icons/sidebar/info.png").scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));
          msgBox.setInformativeText("Choose Install to download and install the modified version of youtube-dl (1.4MB). Cancel will proceed with the existing engine version (" + date + "), which may not work successfully.");
          msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
          msgBox.setDefaultButton(QMessageBox::Ok);
          QPushButton *p = new QPushButton("Quit",nullptr);
          msgBox.addButton(p,QMessageBox::NoRole);
          msgBox.button(QDialogButtonBox::Ok).setText("Install"); // this might not work
          int ret = msgBox.exec();
          switch (ret) {
            case QMessageBox::Ok:
                  emit openSettingsAndClickDownload();
              break;
            case  QMessageBox::Cancel:
                  check_engine_updates();
              break;
            default:
              qApp->quit();
            break;
          }
    }
}

void Engine::evoke_engine_check(){
    if(checkEngine()==false){
        QMessageBox msgBox;
          msgBox.setText(""+QApplication::applicationName()+" requires an updated version of its video download engine. Would you like to install the updated version now?");
          msgBox.setIconPixmap(QPixmap(":/icons/sidebar/info.png").scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));
          msgBox.setInformativeText("Choose Install to download and install the modified version of youtube-dl (1.4MB). Cancel will proceed with the existing engine version, which may not work successfully.");
          msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
          QPushButton *p = new QPushButton("Quit",nullptr);
          msgBox.addButton(p,QMessageBox::NoRole);
          msgBox.setDefaultButton(QMessageBox::Ok);
          msgBox.button(QDialogButtonBox::Ok).setText("Install"); // this might not work

          int ret = msgBox.exec();
          switch (ret) {
            case QMessageBox::Ok:
                  emit openSettingsAndClickDownload();
              break;
            case  QMessageBox::Cancel:
                  evoke_engine_check();
              break;
            default:
                qApp->quit();
            break;
          }
    }
}

