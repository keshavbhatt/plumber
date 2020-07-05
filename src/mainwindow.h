#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QTime>
#include <QToolTip>
#include <QProcess>
#include <QPushButton>

#include "error.h"
#include "RangeSlider.h"
#include "waitingspinnerwidget.h"
#include "settings.h"
#include "ui_console.h"
#include "utils.h"
#include "controlbutton.h"
#include "screenshot.h"
#include "searchprovider.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool consoleHidden_ READ consoleHidden_ WRITE setConsoleHidden NOTIFY consoleVisibilityChanged)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void consoleVisibilityChanged(const bool isVisible);

private slots:

    void setConsoleHidden(bool hidden);

    void setStyle(QString fname);

    void updateDuration();

    void showError(QString message);

    void init_player();

    void on_url_textChanged(const QString &arg1);

    void on_start_clicked();

    void on_pickStart_clicked();

    void on_pickEnd_clicked();

    void on_play_clicked();

    void on_minusOneSecLower_clicked();

    void on_plusOneSecLower_clicked();

    void on_minusOneSecUpper_clicked();

    void on_plusOneSecUpper_clicked();

    void on_volume_valueChanged(int value);

    void on_moveToFrameLower_clicked();

    void on_moveToFrameUpper_clicked();

    void on_preview_clicked();

    void on_settingsButton_clicked();

    void on_vIcon_clicked();

    void init_settings();

    void resizeFix();

    void playMedia(QString url);

    void on_changeLocationButton_clicked();

    void on_location_textChanged(const QString &arg1);

    void on_clip_clicked();

    QString getCodec();

    void on_cancel_clicked();

    void showConsole();

    void hideConsole();

    void on_video_toggled(bool checked);

    void on_gif_toggled(bool checked);

    void showStatus(QString message);

    void on_selectLocal_clicked();

    bool clipOptionChecker();


    void on_screenshotLowerFrame_clicked();

    void on_screenshotUpperFrame_clicked();

    void takeScreenshot();
    void on_youtube_clicked();

protected slots:
    void resizeEvent(QResizeEvent *event);

    void closeEvent(QCloseEvent *event);
private:
    Ui::MainWindow *ui;

    Ui::consoleUi consoleUi;

    QWidget *consoleWidget = nullptr;

    QSettings settings;

    RangeSlider *rsH;

    QMediaPlayer *player = nullptr;

    Error * _error = nullptr;

    WaitingSpinnerWidget *_loader = nullptr;

    int tempVolume;

    Settings *settingsWidget = nullptr;

    QProcess *engineProcess = nullptr;

    QProcess *ffmpegProcess = nullptr;

    QProcess *screenshotProcess = nullptr;

    bool isPlayingPreview = false;

    bool consoleHidden_() const;

    bool consoleHidden;

    controlButton *consoleButton = nullptr;

    QString currentFileName;

    Screenshot *screenshot = nullptr;

    int forbiddenRetryCount = 0;

    SearchProvider *youtubeWidget = nullptr;
};

#endif // MAINWINDOW_H
