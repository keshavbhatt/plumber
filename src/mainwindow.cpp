#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QPropertyAnimation>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  this->setWindowTitle(QApplication::applicationName() + " v" +
                       QApplication::applicationVersion());
  this->setWindowIcon(QIcon(":/icons/app/icon-256.png"));
  setStyle(":/qbreeze/" + settings.value("theme", "dark").toString() + ".qss");

  ui->start->setEnabled(false);
  ui->url->addAction(QIcon("://icons/links-line.png"),
                     QLineEdit::LeadingPosition);
  ui->cancel->hide();
  ui->video->setEnabled(false);
  ui->gif->setEnabled(false);
  if (settings.value("mode").isValid() == false) {
    settings.setValue("mode", "video");
  }

  QString path =
      settings
          .value("destination", QStandardPaths::writableLocation(
                                    QStandardPaths::DownloadLocation) +
                                    "/" + QApplication::applicationName())
          .toString();
  QFileInfo pathInfo(path);
  if (pathInfo.exists() == false) {
    QDir dir;
    if (dir.mkpath(path)) {
      ui->location->setText(path);
    }
  } else {
    ui->location->setText(path);
  }

  init_player();

  rsH = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles,
                        nullptr);

  connect(rsH, &RangeSlider::lowerValueChanged, [=](int lValue) {
    lValue = lValue / 1000;
    int seconds = (lValue) % 60;
    int minutes = (lValue / 60) % 60;
    int hours = (lValue / 3600) % 24;
    QTime time(hours, minutes, seconds);
    ui->startDur->setText(time.toString());
    updateDuration();
  });

  connect(rsH, &RangeSlider::upperValueChanged, [=](int uValue) {
    uValue = uValue / 1000;
    int seconds = (uValue) % 60;
    int minutes = (uValue / 60) % 60;
    int hours = (uValue / 3600) % 24;
    QTime time(hours, minutes, seconds);
    ui->endDur->setText(time.toString());
    updateDuration();
  });

  rsH->SetRange(0, 100);

  ui->rangeLayout->addWidget(rsH);

  ui->url->setText(
      settings.value("lastUrl", "https://www.youtube.com/watch?v=NtXMOJGkous")
          .toString()
          .trimmed()
          .simplified());

  init_settings();

  consoleButton = ui->consoleButton;
  consoleButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  consoleButton->setMouseTracking(true);
  consoleButton->setToolTip("Hide console");
  consoleButton->setIconSize(QSize(10, 10));
  consoleButton->setStyleSheet("border:none");
  connect(consoleButton, &controlButton::clicked, [=]() {
    if (consoleHidden_() == true) {
      showConsole();
    } else {
      hideConsole();
    }
  });

  connect(
      this, &MainWindow::consoleVisibilityChanged, [=](const bool isVisible) {
        consoleButton->setIcon(
            isVisible ? QIcon(":/icons/others/arrow-down-s-line-cropped.png")
                      : QIcon(":/icons/others/arrow-up-s-line-cropped.png"));
        consoleButton->setToolTip(isVisible ? "Show console" : "Hide console");
      });

  setConsoleHidden(false);
  showStatus("<html><head/><body style='color:lightgray'><p "
             "align='center'><br>Welcome to " +
             utils::toCamelCase(QApplication::applicationName()) +
             "</p><p align='center'>version : " +
             QApplication::applicationVersion() +
             "</p><p align='center'>Developed by- Keshav Bhatt</p><p "
             "align='center'><a style='color: lightgray' "
             "href='mailto:keshavnrj@gmail.com?subject=" +
             utils::toCamelCase(QApplication::applicationName()) +
             "'>keshavnrj@gmail.com</a></p>"
             "<p align='center'><a style='color: lightgray' "
             "href='https://snapcraft.io/search?q=keshavnrj'>More "
             "Applications</a></p></body></html>");

  if (settings.value("geometry").isValid()) {
    restoreGeometry(settings.value("geometry").toByteArray());
    if (settings.value("windowState").isValid()) {
      restoreState(settings.value("windowState").toByteArray());
    } else {
      QScreen *pScreen =
          QGuiApplication::screenAt(this->mapToGlobal({this->width() / 2, 0}));
      QRect availableScreenSize = pScreen->availableGeometry();
      this->move(availableScreenSize.center() - this->rect().center());
    }
  }
}

void MainWindow::init_player() {
  // init media player
  player = new QMediaPlayer(this);

  QVideoWidget *videoWidget = new QVideoWidget(this);
  videoWidget->setObjectName("videoWidget");
  videoWidget->setMinimumSize(210, 140);
  videoWidget->setMouseTracking(false);
  videoWidget->setAttribute(Qt::WA_Hover, false);
  videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  videoWidget->setStyleSheet("background-color:black");
  player->setVideoOutput(videoWidget);
  ui->playerLayout->addWidget(videoWidget);
  videoWidget->show();

  // init console Ui
  consoleWidget = new QWidget(ui->playerWidget);
  consoleUi.setupUi(consoleWidget);
  connect(
      consoleUi.textBrowser, &QTextBrowser::anchorClicked,
      [=](const QUrl link) {
        qWarning() << "clicked" << link;
        if (link.toString().contains("open://settings", Qt::CaseInsensitive)) {
          on_settingsButton_clicked();
          return;
        }

        if (link.toString().contains("do://update", Qt::CaseInsensitive)) {
          emit settingsWidget->openSettingsAndClickDownload();
          return;
        }

        QProcess *xdg_open = new QProcess(this);
        xdg_open->start("xdg-open", QStringList() << link.toString());
        if (xdg_open->waitForStarted(1000) == false) {
          // try using QdesktopServices
          bool opened = QDesktopServices::openUrl(link);
          if (opened == false) {
            consoleUi.textBrowser->append(
                "<br><i style='color:red'>Failed to open '" + link.toString() +
                "'</i>");
            qWarning() << "failed to open url" << link;
          }
        }
        connect(
            xdg_open,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                &QProcess::finished),
            [this, xdg_open](int exitCode, QProcess::ExitStatus exitStatus) {
              Q_UNUSED(exitCode);
              Q_UNUSED(exitStatus);
              xdg_open->close();
              xdg_open->deleteLater();
            });
      });

  QTimer::singleShot(100, this, SLOT(resizeFix()));

  // loader is the child of wall_view
  _loader = new WaitingSpinnerWidget(ui->loader, true, false);
  _loader->setRoundness(70.0);
  _loader->setMinimumTrailOpacity(15.0);
  _loader->setTrailFadePercentage(70.0);
  _loader->setNumberOfLines(10);
  _loader->setLineLength(8);
  _loader->setLineWidth(2);
  _loader->setInnerRadius(2);
  _loader->setRevolutionsPerSecond(3);
  _loader->setColor(QColor("#1e90ff"));

  connect(player, &QMediaPlayer::volumeChanged, [=](int volume) {
    ui->vl->setText((QString::number(volume).length() < 2 ? "0" : "") +
                    QString::number(volume));
    ui->volume->setValue(volume);
    settings.setValue("volume", volume);
  });

  player->setVolume(settings.value("volume", 50).toInt());

  connect(ui->playerSeekSlider, &seekSlider::setPosition, [=](QPoint localPos) {
    ui->playerSeekSlider->blockSignals(true);
    int pos =
        ui->playerSeekSlider->minimum() +
        ((ui->playerSeekSlider->maximum() - ui->playerSeekSlider->minimum()) *
         localPos.x()) /
            ui->playerSeekSlider->width();
    QPropertyAnimation *a =
        new QPropertyAnimation(ui->playerSeekSlider, "value");
    a->setDuration(150);
    a->setStartValue(ui->playerSeekSlider->value());
    a->setEndValue(pos);
    a->setEasingCurve(QEasingCurve::Linear);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    player->setPosition(pos * 1000);
    ui->playerSeekSlider->blockSignals(false);
  });

  connect(ui->playerSeekSlider, &seekSlider::showToolTip, [=](QPoint localPos) {
    int pos =
        ui->playerSeekSlider->minimum() +
        ((ui->playerSeekSlider->maximum() - ui->playerSeekSlider->minimum()) *
         localPos.x()) /
            ui->playerSeekSlider->width();
    int seconds = (pos) % 60;
    int minutes = (pos / 60) % 60;
    int hours = (pos / 3600) % 24;
    QTime time(hours, minutes, seconds);
    QToolTip::showText(ui->playerSeekSlider->mapToGlobal(localPos),
                       "Seek: " + time.toString());
  });

  connect(player, &QMediaPlayer::durationChanged, [=](qint64 dur) {
    dur = dur / 1000;
    int seconds = (dur) % 60;
    int minutes = (dur / 60) % 60;
    int hours = (dur / 3600) % 24;
    QTime time(hours, minutes, seconds);
    ui->duration->setText(time.toString());
    ui->playerSeekSlider->setMaximum(dur);
    rsH->setMaximum(dur * 1000);
  });

  connect(player, &QMediaPlayer::positionChanged, [=](qint64 pos) {
    pos = pos / 1000;
    int seconds = (pos) % 60;
    int minutes = (pos / 60) % 60;
    int hours = (pos / 3600) % 24;
    QTime time(hours, minutes, seconds);
    ui->position->setText(time.toString());
    ui->playerSeekSlider->setValue(pos);
    if (isPlayingPreview == true) {
      ui->preview->setIcon(QIcon(":/icons/pause-line.png"));
      if (pos == rsH->GetUpperValue() / 1000) {
        player->blockSignals(true);
        player->pause();
        ui->play->setIcon(QIcon(":/icons/play-line.png"));
        ui->preview->setIcon(QIcon(":/icons/play-line.png"));
        isPlayingPreview = false;
        player->blockSignals(false);
      }
    }
  });

  connect(player, &QMediaPlayer::stateChanged, [=](QMediaPlayer::State state) {
    if (state == QMediaPlayer::PlayingState ||
        state == QMediaPlayer::StoppedState)
      _loader->stop();
    ui->play->setIcon(QIcon(
        state == (QMediaPlayer::PlayingState || QMediaPlayer::BufferedMedia)
            ? ":/icons/pause-line.png"
            : ":/icons/play-line.png"));
    if (isPlayingPreview)
      ui->preview->setIcon(QIcon(
          state == (QMediaPlayer::PlayingState || QMediaPlayer::BufferedMedia)
              ? ":/icons/pause-line.png"
              : ":/icons/play-line.png"));
  });

  connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
          [=](QMediaPlayer::Error error) {
            Q_UNUSED(error);
            QString error_string = player->errorString();
            if (error_string.contains("forbidden", Qt::CaseInsensitive)) {
              forbiddenRetryCount = forbiddenRetryCount + 1;
              if (forbiddenRetryCount >
                  2) { // prevent the loop if media is actaully forbidden
                showError(error_string);
              } else {
                settingsWidget->clearEngineCache();
                showError(error_string + "\nThis issue is caused with youtube "
                                         "videos due to obsolete engine "
                                         "cache.\nAuto clearing engine cache "
                                         "and trying again...");
                ui->start->click();
              }
            } else {
              showError(error_string);
            }
#ifndef QT_NO_CURSOR
            QApplication::restoreOverrideCursor();
#endif
          });

  connect(player, &QMediaPlayer::mediaStatusChanged,
          [=](QMediaPlayer::MediaStatus mediastate) {
            //        qDebug()<<mediastate;
            (mediastate == QMediaPlayer::BufferingMedia ||
             mediastate == QMediaPlayer::LoadingMedia ||
             mediastate == QMediaPlayer::StalledMedia ||
             mediastate == QMediaPlayer::LoadingMedia)
                ? _loader->start()
                : _loader->stop();
          });
}

void MainWindow::hideConsole() {
  qDebug() << "hideConsolecalled" << sender();
  if (consoleHidden_())
    return;
  QPropertyAnimation *animation = new QPropertyAnimation(consoleWidget, "pos");
  animation->setDuration(500);
  animation->setEasingCurve(QEasingCurve::InCurve);
  animation->setStartValue(QPoint(consoleWidget->x(), consoleWidget->y()));
  animation->setEndValue(QPoint(
      consoleWidget->x(), (consoleWidget->y()) - consoleWidget->height()));
  animation->start(QPropertyAnimation::DeleteWhenStopped);
  consoleButton->setEnabled(animation->state() != QPropertyAnimation::Running);
  connect(animation, &QPropertyAnimation::finished, [=]() {
    consoleButton->setEnabled(animation->state() !=
                              QPropertyAnimation::Running);
    setConsoleHidden(true);
  });
  qDebug() << "2hideConsolecalled" << sender();
}

void MainWindow::showConsole() {
  if (!consoleHidden_())
    return;
  QPropertyAnimation *animation = new QPropertyAnimation(consoleWidget, "pos");
  animation->setDuration(500);
  animation->setEasingCurve(QEasingCurve::InCurve);
  animation->setStartValue(QPoint(consoleWidget->x(), consoleWidget->y()));
  animation->setEndValue(QPoint(
      consoleWidget->x(), (consoleWidget->y()) + consoleWidget->height()));
  animation->start(QPropertyAnimation::DeleteWhenStopped);
  consoleButton->setEnabled(animation->state() != QPropertyAnimation::Running);
  connect(animation, &QPropertyAnimation::finished, [=]() {
    consoleButton->setEnabled(animation->state() !=
                              QPropertyAnimation::Running);
    setConsoleHidden(false);
  });
  qDebug() << "showconsolecalled" << sender();
}

void MainWindow::resizeFix() {
  this->resize(this->width() + 1, this->height());
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  // clear locatemp dir
  QDir(utils::returnPath("localTemp")).removeRecursively();
  QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  if (consoleHidden_() == false) {
    consoleWidget->setGeometry(ui->playerWidget->rect());
  } else {
    consoleWidget->resize(ui->playerWidget->rect().size());
    consoleWidget->move(QPoint(ui->playerWidget->rect().x(),
                               ui->playerWidget->rect().y() -
                                   ui->playerWidget->rect().height()));
  }
  QMainWindow::resizeEvent(event);
}

// read
bool MainWindow::consoleHidden_() const { return consoleHidden; }

void MainWindow::updateDuration() {
  int dur = rsH->GetUpperValue() - rsH->GetLowerValue();
  dur = dur / 1000;
  if (dur > 0) {
    int seconds = (dur) % 60;
    int minutes = (dur / 60) % 60;
    int hours = (dur / 3600) % 24;
    QTime time(hours, minutes, seconds);
    ui->clip_duration->setText(time.toString());
    ui->preview->setEnabled(true);
    ui->clip->setEnabled(true);
  } else {
    ui->clip_duration->setText("invalid");
    ui->preview->setEnabled(false);
    ui->clip->setEnabled(false);
  }
}

void MainWindow::showError(QString message) {
  // init error
  _error = new Error(this);
  _error->setAttribute(Qt::WA_DeleteOnClose);
  _error->setWindowTitle(QApplication::applicationName() + " | Error dialog");
  _error->setWindowFlag(Qt::Dialog);
  _error->setWindowModality(Qt::NonModal);
  _error->setError("An Error ocurred while processing your request!", message);
  _error->show();
}

void MainWindow::setStyle(QString fname) {
  QFile styleSheet(fname);
  if (!styleSheet.open(QIODevice::ReadOnly)) {
    qWarning("Unable to open file");
    return;
  }
  qApp->setStyleSheet(styleSheet.readAll());
  styleSheet.close();
}

MainWindow::~MainWindow() {
  if (engineProcess != nullptr) {
    engineProcess->blockSignals(true);
    engineProcess->close();
    engineProcess->disconnect();
    engineProcess->deleteLater();
  }

  if (ffmpegProcess != nullptr) {
    ffmpegProcess->blockSignals(true);
    ffmpegProcess->close();
    ffmpegProcess->disconnect();
    ffmpegProcess->deleteLater();
  }
  delete ui;
}

void MainWindow::setConsoleHidden(bool hidden) {
  consoleHidden = hidden;
  emit consoleVisibilityChanged(hidden);
}

void MainWindow::on_url_textChanged(const QString &arg1) {
  ui->start->setEnabled(!arg1.trimmed().isEmpty());
  if (arg1.isEmpty() == false) {
    settings.setValue("lastUrl", arg1);
  }
}

void MainWindow::on_start_clicked() {
  if (clipOptionChecker())
    return;

  ui->video->setEnabled(false);
  ui->gif->setEnabled(false);

  isPlayingPreview = false;
  currentFileName = "";
  ui->clipname->clear();
  rsH->SetRange(0, 100);
  player->stop();
  if (engineProcess != nullptr) {
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    consoleUi.textBrowser->append(
        "<br><i style='color:red'>Cancelling current process...</i><br>\n");
    engineProcess->blockSignals(true);
    engineProcess->close();
    engineProcess->disconnect();
    engineProcess->deleteLater();
    engineProcess->blockSignals(false);
    engineProcess = nullptr;
    consoleUi.textBrowser->append(
        "<br><i style='color:red'>Process cancelled.</i><br>\n");
    QTimer::singleShot(800, [=]() {
      consoleUi.textBrowser->clear();
      on_start_clicked();
    });
    return;
  }

  QString resouceUrl = ui->url->text().trimmed().simplified();
  consoleUi.textBrowser->clear();
  // handle local files
  if (resouceUrl.at(0) == "/") {
    resouceUrl.prepend("file://");
  }

  if (resouceUrl.contains("file://")) {
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

    QString ext = QFileInfo(resouceUrl).completeSuffix();

    // clear localTemp
    QDir(utils::returnPath("localTemp")).removeRecursively();
    // copy file to temp location with name reanamed
    QString tempPath = utils::returnPath("localTemp");
    QString temFileName = utils::generateRandomId(10) + "." + ext;
    QString srcFileName =
        QString(ui->url->text().trimmed().simplified()).remove("file://");
    QString destFileName = tempPath + temFileName;
    if (QFileInfo(srcFileName).exists()) {
      if (QFile::copy(srcFileName, destFileName)) {
#ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
#endif
        int dotPos2 = destFileName.lastIndexOf(".", -1);
        // change resource file to temp resouce.
        resouceUrl = "file://" + destFileName;
        currentFileName = QUrl(destFileName.left(dotPos2)).fileName();
      }
    } else {
#ifndef QT_NO_CURSOR
      QApplication::restoreOverrideCursor();
#endif
    }

    hideConsole();
    if (!currentFileName.isEmpty()) {
      ui->clipname->setText(currentFileName);
    } else {
      ui->clipname->setText(utils::generateRandomId(10));
    }
    ui->video->setEnabled(true);
    ui->gif->setEnabled(true);
    QString mode = settings.value("mode", "video").toString();

    ui->video->toggle();
    ui->gif->toggle();

    ui->video->setChecked(mode == "video");
    ui->gif->setChecked(mode == "gif");
    playMedia(resouceUrl);
    return;
  }
  // handle remote urls with engine
  if (settingsWidget->engineReady() == false) {
    consoleUi.textBrowser->setText(
        "<br><i style='color:red'>Media probe engine not present or not "
        "updated.<br><a style='color:skyblue' href='do://update'>Click to "
        "Update engine</a> or <a style='color:skyblue' "
        "href='open://settings'>Click to open Settings.</a></i><br>\n");
    return;
  }
  QString addin_path =
      QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  QStringList args;
  args << addin_path + "/core"
       << "-f"
       << "best"
       << "-g"
       << "--get-filename" << resouceUrl;
  engineProcess = new QProcess(this);
  connect(
      engineProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                         &QProcess::finished),
      [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0) {
          Q_UNUSED(exitStatus);
          QString output = engineProcess->readAll();
          qDebug() << output;
          if (output.contains("http", Qt::CaseInsensitive)) {
            consoleUi.textBrowser->append("<br><i "
                                          "style='color:lightgreen'>Media "
                                          "probe finished.</i><br>\n");
            consoleUi.textBrowser->append(
                "<i style='color:lightgreen'>Loading media...</i>\n");
            QTimer::singleShot(1500, [=]() {
              _loader->start();
              playMedia(output.split("\n").first().trimmed().simplified());
            });
            QRegExp rx("\\n\\w");
            if (output.contains(rx)) {
              QString name = rx.capturedTexts().first().trimmed() +
                             output.split(rx).last().trimmed().simplified();
              ui->clipname->setText(name.left(name.lastIndexOf(".")));
            } else {
              ui->clipname->setText(utils::generateRandomId(10));
            }
            ui->video->setEnabled(true);
            ui->gif->setEnabled(true);
            currentFileName = ui->clipname->text();

            QString mode = settings.value("mode", "video").toString();

            ui->video->toggle();
            ui->gif->toggle();

            ui->video->setChecked(mode == "video");
            ui->gif->setChecked(mode == "gif");
          } else {
            if (output.contains("forbidden", Qt::CaseInsensitive)) {
              settingsWidget->clearEngineCache();
            }
            consoleUi.textBrowser->setText(output);
          }
        } else {
          consoleUi.textBrowser->append("<br><i style='color:red'>An error "
                                        "occured while processing your "
                                        "request</i>\n");
          showError("Process exited with code " + QString::number(exitCode) +
                    "\n" + engineProcess->readAllStandardError());
        }
#ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
#endif
      });
  engineProcess->start("python3", args);
  if (engineProcess->waitForStarted(1000)) {
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::BusyCursor);
#endif
    consoleUi.textBrowser->clear();
    showConsole();
    consoleUi.textBrowser->append(
        "<i style='color:lightgreen'>Started media probe...</i>\n");
  } else {
    consoleUi.textBrowser->clear();
    consoleUi.textBrowser->append(
        "<i style='color:red'>Failed to start media probe.</i>\n");
  }
}

void MainWindow::playMedia(QString url) {

  player->setMedia(QUrl(url));
  player->play();
  hideConsole();
}

void MainWindow::on_pickStart_clicked() {
  if (clipOptionChecker())
    return;
  int pos = player->position();
  ui->startDur->setText(QString::number(pos));
  rsH->SetLowerValue(pos);
}

void MainWindow::on_pickEnd_clicked() {
  if (clipOptionChecker())
    return;
  int pos = player->position();
  ui->endDur->setText(QString::number(pos));
  rsH->SetUpperValue(pos);
}

void MainWindow::on_play_clicked() {
  if (clipOptionChecker())
    return;
  hideConsole();
  if (isPlayingPreview) {
    isPlayingPreview = false;
    ui->preview->setIcon(QIcon(":/icons/play-line.png"));
  }
  if (player->state() != QMediaPlayer::PausedState) {
    player->pause();
  } else if (player->state() == QMediaPlayer::PausedState) {
    player->play();
  }
}

void MainWindow::on_minusOneSecLower_clicked() {
  if (clipOptionChecker())
    return;
  int rsL = rsH->GetLowerValue();
  rsH->SetLowerValue(rsL - 1000);
}

void MainWindow::on_plusOneSecLower_clicked() {
  if (clipOptionChecker())
    return;
  int rsL = rsH->GetLowerValue();
  rsH->SetLowerValue(rsL + 1000);
}

void MainWindow::on_minusOneSecUpper_clicked() {
  if (clipOptionChecker())
    return;
  int rsU = rsH->GetUpperValue();
  rsH->SetUpperValue(rsU - 1000);
}

void MainWindow::on_plusOneSecUpper_clicked() {
  if (clipOptionChecker())
    return;
  int rsU = rsH->GetUpperValue();
  rsH->SetUpperValue(rsU + 1000);
}

void MainWindow::on_volume_valueChanged(int value) {
  player->setVolume(value);
  if (value > 0)
    tempVolume = value;
  ui->vIcon->setIcon(value == 0 ? QIcon(":/icons/volume-mute-line.png")
                                : QIcon(":/icons/volume-up-line.png"));
}

void MainWindow::on_moveToFrameLower_clicked() {
  if (clipOptionChecker())
    return;
  hideConsole();
  player->setPosition(rsH->GetLowerValue());
  player->pause();
}

void MainWindow::on_moveToFrameUpper_clicked() {
  if (clipOptionChecker())
    return;
  hideConsole();
  player->setPosition(rsH->GetUpperValue());
  player->pause();
}

void MainWindow::init_settings() {
  settingsWidget = new Settings(this);
  settingsWidget->setWindowTitle(QApplication::applicationName() +
                                 " | Settings");
  settingsWidget->setWindowFlag(Qt::Dialog);
  settingsWidget->setWindowModality(Qt::NonModal);

  connect(settingsWidget, &Settings::themeToggled, [=]() {
    setStyle(":/qbreeze/" + settings.value("theme", "dark").toString() +
             ".qss");
    if (settings.value("theme", "dark").toString() == "dark") {
      QPalette p = ui->url->palette();
      p.setColor(QPalette::PlaceholderText, QColor("#898b8d"));
      foreach (QLineEdit *edit, this->findChildren<QLineEdit *>()) {
        edit->setPalette(p);
      }
    }
  });

  connect(settingsWidget, &Settings::openSettingsAndClickDownload, [=]() {
    on_settingsButton_clicked();
    settingsWidget->downloadEngine();
  });
}

bool MainWindow::clipOptionChecker() {
  bool check = false;
  if (ffmpegProcess != nullptr && ffmpegProcess->state() == QProcess::Running) {
    showError("This option is not available while 'Clip' process is running.");
    check = true;
  } else {
    check = false;
  }
  return check;
}

void MainWindow::on_preview_clicked() {
  if (clipOptionChecker())
    return;
  hideConsole();
  if (isPlayingPreview && player->state() == QMediaPlayer::PlayingState) {
    isPlayingPreview = false;
    player->pause();
    ui->preview->setIcon(QIcon(":/icons/play-line.png"));
  } else {
    isPlayingPreview = true;
    on_moveToFrameLower_clicked();
    player->play();
  }
}

void MainWindow::on_settingsButton_clicked() {
  if (settingsWidget->isVisible() == false) {
    settingsWidget->showNormal();
  }
}

void MainWindow::on_vIcon_clicked() {
  ui->volume->setValue(ui->volume->value() > 0 ? 0 : tempVolume);
}

void MainWindow::on_changeLocationButton_clicked() {
  if (clipOptionChecker())
    return;
  QString path =
      settings
          .value("destination", QStandardPaths::writableLocation(
                                    QStandardPaths::DownloadLocation))
          .toString() +
      "/" + QApplication::applicationName();

  QString destination = QFileDialog::getExistingDirectory(
      this, tr("Select destination directory"), path,
      QFileDialog::ShowDirsOnly);
  QFileInfo dir(destination);
  if (dir.isWritable()) {
    ui->location->setText(destination);
  } else {
    showError(
        "Destination directory is not writable.<br>Please choose another.");
  }
}

void MainWindow::on_location_textChanged(const QString &arg1) {
  if (arg1.isEmpty() == false) {
    settings.setValue("destination", arg1);
    ui->clip->setEnabled(true);
  } else {
    ui->clip->setEnabled(false);
  }
}

void MainWindow::on_clip_clicked() {
  player->pause();
  isPlayingPreview = false;
  ui->preview->setIcon(QIcon(":/icons/play-line.png"));

  if (ffmpegProcess != nullptr) {
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    ffmpegProcess->blockSignals(true);
    ffmpegProcess->close();
    ffmpegProcess->disconnect();
    ffmpegProcess->deleteLater();
    ffmpegProcess->blockSignals(false);
    ffmpegProcess = nullptr;
    ui->video->setEnabled(true);
    ui->gif->setEnabled(true);
  }
  consoleUi.textBrowser->clear();
  QString fps = settings.value("framerate", "10").toString();
  QString scale = settings.value("scale", "320").toString();
  QStringList args;
  if (settings.value("mode", "video").toString() == "video") {
    args << "-c"
         << "ffmpeg -async 1 -ss " + ui->startDur->text().trimmed() + " -i \"" +
                player->media().canonicalUrl().toString() + "\" -t " +
                ui->clip_duration->text().trimmed() + getCodec() + +"\"" +
                ui->location->text().trimmed() + "/" +
                ui->clipname->text().trimmed() + "\" -y";
  } else {
    args << "-c"
         << "ffmpeg -ss " + ui->startDur->text().trimmed() + " -t " +
                ui->clip_duration->text().trimmed() + " -i \"" +
                player->media().canonicalUrl().toString() + "\" -vf \"fps=" +
                fps + ",scale=" + scale + ":-1:flags="
                                          "lanczos,split[s0][s1];[s0]"
                                          "palettegen[p];[s1][p]paletteuse\" "
                                          "-loop 0 \"" +
                ui->location->text().trimmed() + "/" +
                ui->clipname->text().trimmed() + "\" -y";
  }

  ffmpegProcess = new QProcess(this);
  ffmpegProcess->setProcessChannelMode(QProcess::MergedChannels);
  connect(ffmpegProcess, &QProcess::readyRead, [=]() {
    QString output = ffmpegProcess->readAll();
    if (output.contains("frame= ", Qt::CaseInsensitive))
      consoleUi.textBrowser->append("<i style='color:skyblue'>" +
                                    output.replace("\n", "<br>") + "</i>");
  });
  connect(
      ffmpegProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                         &QProcess::finished),
      [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitStatus);
        if (exitCode == 0) {
          consoleUi.textBrowser->append(
              "<br><i style='color:lightgreen'>Download Finished.</i>\n");
          QString path = settings
                             .value("destination",
                                    QStandardPaths::writableLocation(
                                        QStandardPaths::DownloadLocation) +
                                        "/" + QApplication::applicationName())
                             .toString();
          QString filepath = path + "/" + ui->clipname->text();
          consoleUi.textBrowser->append(
              "<br><a style='color:skyblue' href='" + path +
              "'>Open file location</a> "
              "or <a style='color:skyblue' href='file://" +
              filepath + "'>Play file</a><br>");
        } else {
          consoleUi.textBrowser->append("<br><i style='color:red'>An error "
                                        "occured while processing your "
                                        "request</i>\n");
          showError("Process exited with code " + QString::number(exitCode) +
                    "\n" + ffmpegProcess->readAll());
        }
#ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
#endif
        ui->cancel->hide();
        ui->clip->show();
        ui->video->setEnabled(true);
        ui->gif->setEnabled(true);
      });
  ffmpegProcess->start("bash", args);
  if (ffmpegProcess->waitForStarted(1000)) {
    ui->clip->hide();
    ui->cancel->show();
    ui->video->setEnabled(false);
    ui->gif->setEnabled(false);
    showConsole();
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::BusyCursor);
#endif
    consoleUi.textBrowser->clear();
    consoleUi.textBrowser->append(
        "<i style='color:skyblue'>Initializing trimming process...</i>\n");
  } else {
    consoleUi.textBrowser->clear();
    consoleUi.textBrowser->append(
        "<i style='color:red'>Failed to start trimming process.</i>\n");
  }
}

QString MainWindow::getCodec() {
  QString code;
  if (settings.value("codec", "mpeg")
          .toString()
          .contains("copy", Qt::CaseInsensitive)) {
    code = " -c copy ";
  } else {
    code = " -c:v libx264 -c:a aac ";
  }
  qDebug() << "used encoder:" << code;
  return code;
}

void MainWindow::on_cancel_clicked() {
  if (ffmpegProcess != nullptr) {
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    ffmpegProcess->close();
    ffmpegProcess->disconnect();
    ffmpegProcess->deleteLater();
    ffmpegProcess = nullptr;
    ui->cancel->hide();
    ui->clip->show();
    ui->video->setEnabled(true);
    ui->gif->setEnabled(true);
  }
}

void MainWindow::on_video_toggled(bool checked) {
  qDebug() << "video toggled" << checked;
  settings.setValue("mode", checked ? "video" : "gif");
  if (checked) {
    ui->clipname->setText(currentFileName + ".mkv");
  }
}

void MainWindow::on_gif_toggled(bool checked) {
  qDebug() << "gif toggled" << checked;
  settings.setValue("mode", checked ? "gif" : "video");
  if (checked) {
    ui->clipname->setText(currentFileName + ".gif");
  }
}

void MainWindow::showStatus(QString message) {
  QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
  consoleUi.textBrowser->setGraphicsEffect(eff);
  QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
  a->setDuration(1000);
  a->setStartValue(0);
  a->setEndValue(1);
  a->setEasingCurve(QEasingCurve::InCurve);
  a->start(QPropertyAnimation::DeleteWhenStopped);
  consoleUi.textBrowser->setText(message);
  consoleUi.textBrowser->show();
}

void MainWindow::on_selectLocal_clicked() {
  if (clipOptionChecker())
    return;
  QString path = settings
                     .value("localPath", QStandardPaths::writableLocation(
                                             QStandardPaths::MoviesLocation))
                     .toString();

  QString mediaFile = QFileDialog::getOpenFileName(
      this, tr("Select media file"), path, tr("Media files (*.*)"));
  if (mediaFile.isEmpty()) {
    showError("No file selected.");
  } else {
    ui->url->setText(mediaFile);
  }
}

void MainWindow::on_screenshotLowerFrame_clicked() { this->takeScreenshot(); }

void MainWindow::on_screenshotUpperFrame_clicked() { this->takeScreenshot(); }

void MainWindow::takeScreenshot() {

  if (clipOptionChecker())
    return;
  // get sender
  QPushButton *senderBtn = qobject_cast<QPushButton *>(sender());
  if (senderBtn == nullptr)
    return;
  bool isUpper =
      senderBtn->objectName().contains("UpperFrame", Qt::CaseInsensitive);
  player->pause();
  if (screenshotProcess != nullptr) {
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    screenshotProcess->blockSignals(true);
    screenshotProcess->close();
    screenshotProcess->disconnect();
    screenshotProcess->deleteLater();
    screenshotProcess->blockSignals(false);
    screenshotProcess = nullptr;
  }

  QString format = "." + settings.value("sc_format", "png").toString();
  QDir dir(utils::returnPath("screenshots"));
  dir.removeRecursively(); // empty dir before to save space.
  QString tempScLocation = utils::returnPath("screenshots");
  QString fileLocation = tempScLocation + "/" + currentFileName + format;
  qDebug() << fileLocation;
  QStringList args;
  args << "-c"
       << "ffmpeg -ss " + QString(isUpper ? ui->endDur->text().trimmed()
                                          : ui->startDur->text().trimmed()) +
              " -i \"" + player->media().canonicalUrl().toString() +
              "\" -vframes 1 \"" + fileLocation + "\" -y";
  screenshotProcess = new QProcess(this);
  connect(screenshotProcess, &QProcess::readyRead, [=]() {
    QString output = screenshotProcess->readAll();
    showConsole();
    if (output.contains("frame= ", Qt::CaseInsensitive))
      consoleUi.textBrowser->append("<i style='color:skyblue'>" +
                                    output.replace("\n", "<br>") + "</i>");
  });
  connect(screenshotProcess,
          static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
              &QProcess::finished),
          [this, fileLocation](int exitCode, QProcess::ExitStatus exitStatus) {
            Q_UNUSED(exitStatus);
#ifndef QT_NO_CURSOR
            QApplication::restoreOverrideCursor();
#endif
            if (exitCode == 0) {
              // init screenshot
              screenshot = new Screenshot(this, fileLocation);
              screenshot->setWindowTitle(QApplication::applicationName() +
                                         " | Screenshot capture");
              screenshot->setAttribute(Qt::WA_DeleteOnClose);
              screenshot->setWindowModality(Qt::ApplicationModal);
              screenshot->setWindowFlag(Qt::Dialog);
              connect(screenshot, &Screenshot::savedScreenshot,
                      [=](QString screenshotLocation) {
                        consoleUi.textBrowser->append(
                            "\n<i style='color:lightgreen'>Screenshot taken "
                            "hiding console in 10 seconds...</i>\n");
                        QTimer::singleShot(10000, this, SLOT(hideConsole()));
                        QString path =
                            settings
                                .value("sc_location",
                                       QStandardPaths::writableLocation(
                                           QStandardPaths::DownloadLocation) +
                                           "/" +
                                           QApplication::applicationName())
                                .toString();
                        consoleUi.textBrowser->append(
                            "<br><a style='color:skyblue' href='" + path +
                            "'>Open file location</a> "
                            "or <a style='color:skyblue' href='file://" +
                            screenshotLocation + "'>Open file</a><br>");
                      });
              connect(screenshot, &Screenshot::failedToSaveSc, [=]() {
                consoleUi.textBrowser->append(
                    "<br><i style='color:red'>Screenshot operation cancelled, "
                    "hiding console in 4 seconds...</i>\n");
                QTimer::singleShot(4000, this, SLOT(hideConsole()));
              });
              screenshot->show();
            } else {
              showError("Unable to take screenshot, process returned :\n\n" +
                        screenshotProcess->readAllStandardError());
            }
          });
  screenshotProcess->start("bash", args);
  if (screenshotProcess->waitForStarted(1000)) {
    showConsole();
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::BusyCursor);
#endif
    consoleUi.textBrowser->clear();
    consoleUi.textBrowser->append(
        "<i style='color:skyblue'>Taking screenshot please wait...</i>\n");
  } else {
    showError("Unable to take screenshot, not able to start process");
  }
}

void MainWindow::on_youtube_clicked() {
  if (youtubeWidget == nullptr) {
    youtubeWidget = new SearchProvider(this);
    youtubeWidget->setWindowTitle(QApplication::applicationName() +
                                  " | YouTube Search");
    youtubeWidget->setWindowFlag(Qt::Dialog);
    youtubeWidget->setWindowModality(Qt::NonModal);
    youtubeWidget->setMinimumSize(550, 480);
    connect(youtubeWidget, &SearchProvider::loadVideo, [=](QString videoId) {
      ui->url->setText(videoId);
      ui->start->click();
    });
  }

  if (youtubeWidget->isVisible() == false) {
    youtubeWidget->showNormal();
  } else {
    youtubeWidget->setFocus();
  }
}
