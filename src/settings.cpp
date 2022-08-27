#include "settings.h"
#include "error.h"
#include "ui_settings.h"
#include <QDesktopServices>
#include <QMovie>

Settings::Settings(QWidget *parent) : QWidget(parent), ui(new Ui::Settings) {
  ui->setupUi(this);
  ui->loading_movie->hide();
  readSettings();
  init_engine();
  ui->frameRate->setSuffix(" fps");
  ui->scale->setSuffix(" px");
  updateAbout();
}

void Settings::updateAbout() {
  QString aboutStr = R"(
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html><head><meta name="qrichtext" content="1" /><meta charset="utf-8" /><style type="text/css">
p, li { white-space: pre-wrap; }
hr { height: 1px; border-width: 0; }
</style></head><body style="text-align:center; font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;">
<p style="-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Ubuntu'; font-size:11pt;"><br /></p>
<p style="-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Ubuntu'; font-size:11pt;"><br /></p>
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><img src=":/icons/app/icon-64.png" /></p>
<p style="-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Ubuntu'; font-size:11pt;"><br /></p>
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style=" font-family:'Ubuntu'; font-size:11pt;">Designed and Developed</span></p>
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style=" font-family:'Ubuntu'; font-size:11pt;">by </span><span style=" font-family:'Ubuntu'; font-size:11pt; font-weight:600;">Keshav Bhatt</span><span style=" font-family:'Ubuntu'; font-size:11pt;"> &lt;keshavnrj@gmail.com&gt;</span></p>
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style=" font-family:'Ubuntu'; font-size:11pt;">Website: https://ktechpit.com</span></p>
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style=" font-family:'Ubuntu'; font-size:11pt;">Runtime: %1</span></p>
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style=" font-family:'Ubuntu'; font-size:11pt;">Version: %2</span></p>
<p><a style='color: silver' href='https://snapcraft.io/search?q=keshavnrj'>More Applications</a></p>
<p style="-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Ubuntu'; font-size:11pt;"><br /></p></body></html>
    )";

  ui->aboutTextBrowser->setHtml(aboutStr.arg(qVersion(),VERSIONSTR));
}

// public method to call clear engine cache.
void Settings::clearEngineCache() { engine->clearEngineCache(); }

void Settings::on_clear_engine_cache_clicked() {
  engine->clearEngineCache();
  QMovie *movie = new QMovie(":/icons/others/load.gif");
  ui->loading_movie->setMovie(movie);
  ui->loading_movie->setVisible(true);
  movie->start();
}

void Settings::on_download_engine_clicked() {
  engine->download_engine_clicked();
  ui->download_engine->setEnabled(false);
  QMovie *movie = new QMovie(":/icons/others/load.gif");
  ui->loading_movie->setMovie(movie);
  ui->loading_movie->setVisible(true);
  movie->start();
}

void Settings::downloadEngine() { on_download_engine_clicked(); }

bool Settings::engineReady() {
  if (engine != nullptr)
    return engine->engineReady();
  else
    return false;
}

void Settings::init_engine() {
  engine = new Engine(this);

  connect(engine, &Engine::errorMessage, [=](QString errorMessage) {
    Error *_error = new Error(this);
    _error->setAttribute(Qt::WA_DeleteOnClose);
    _error->setWindowTitle(QApplication::applicationName() + " | Error dialog");
    _error->setWindowFlag(Qt::Dialog);
    _error->setWindowModality(Qt::NonModal);
    _error->setError("An Error ocurred while processing your request!",
                     errorMessage);
    _error->show();
  });

  connect(engine, &Engine::engineCacheCleared, [=]() {
    if (ui->loading_movie->movie() != nullptr) {
      ui->loading_movie->movie()->stop();
    }
    ui->loading_movie->setVisible(false);
  });

  connect(engine, &Engine::engineDownloadFailed, [=](QString errorMessage) {
    Error *_error = new Error(this);
    _error->setAttribute(Qt::WA_DeleteOnClose);
    _error->setWindowTitle(QApplication::applicationName() + " | Error dialog");
    _error->setWindowFlag(Qt::Dialog);
    _error->setWindowModality(Qt::NonModal);
    _error->setError("An Error ocurred while processing your request!",
                     errorMessage);
    _error->show();
  });

  connect(engine, &Engine::engineDownloadSucceeded, [=]() {
    if (ui->loading_movie->movie() != nullptr) {
      ui->loading_movie->movie()->stop();
    }
    ui->loading_movie->setVisible(false);
    ui->download_engine->setEnabled(true);
  });

  connect(engine, &Engine::engineStatus,
          [=](QString status) { ui->engine_status->setText(status); });

  connect(engine, &Engine::openSettingsAndClickDownload,
          [=]() { emit openSettingsAndClickDownload(); });
}

Settings::~Settings() { delete ui; }

void Settings::readSettings() {
  // theme setting
  ui->dark->setChecked(settings.value("theme", "dark").toString() == "dark");
  ui->light->setChecked(settings.value("theme").toString() == "light");

  // encoder
  if (settings.value("codec", "mpeg")
          .toString()
          .contains("copy", Qt::CaseInsensitive)) {
    ui->copy->setChecked(true);
  } else {
    ui->mpeg->setChecked(true);
  }

  // screenshot
  if (settings.value("sc_format", "png")
          .toString()
          .contains("png", Qt::CaseInsensitive)) {
    ui->png->setChecked(true);
  } else {
    ui->jpg->setChecked(true);
  }

  // gif
  QString fps = settings.value("framerate", "10").toString();
  QString scale = settings.value("scale", "320").toString();
  ui->frameRate->setValue(fps.toInt());
  ui->scale->setValue(scale.toInt());
}

void Settings::on_github_clicked() {
  QDesktopServices::openUrl(QUrl("https://github.com/keshavbhatt/plumber"));
}

void Settings::on_rate_clicked() {
  QDesktopServices::openUrl(QUrl("snap://plumber"));
}

void Settings::on_donate_clicked() {
  QDesktopServices::openUrl(QUrl("https://paypal.me/keshavnrj/5"));
}

void Settings::on_dark_toggled(bool checked) {
  settings.setValue("theme", checked ? "dark" : "light");
  emit themeToggled();
}

void Settings::on_light_toggled(bool checked) {
  settings.setValue("theme", checked ? "light" : "dark");
  emit themeToggled();
}

void Settings::on_copy_toggled(bool checked) {
  settings.setValue("codec", checked ? "copy" : "mpeg");
}

void Settings::on_mpeg_toggled(bool checked) {
  settings.setValue("codec", checked ? "mpeg" : "copy");
}

void Settings::on_png_toggled(bool checked) {
  settings.setValue("sc_format", checked ? "png" : "jpg");
}

void Settings::on_jpg_toggled(bool checked) {
  settings.setValue("sc_format", checked ? "jpg" : "png");
}

void Settings::on_frameRate_valueChanged(const QString &arg1) {
  settings.setValue("framerate", arg1.split(" ").first());
}

void Settings::on_scale_valueChanged(const QString &arg1) {
  settings.setValue("scale", arg1.split(" ").first());
}

void Settings::on_restore_clicked() {
  ui->dark->setChecked(true);
  ui->frameRate->setValue(10);
  ui->scale->setValue(320);
  ui->mpeg->setChecked(true);
}

void Settings::on_aboutTextBrowser_anchorClicked(const QUrl &arg1) {
  QDesktopServices::openUrl(QUrl(arg1));
}
