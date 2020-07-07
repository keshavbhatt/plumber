#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QSettings>
#include <QProcess>

#include "engine.h"

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
signals:
    void themeToggled();

    void openSettingsAndClickDownload();

public slots:

    void downloadEngine();

    void clearEngineCache();

    bool engineReady();
private slots:

    void readSettings();

    void on_github_clicked();

    void on_rate_clicked();

    void on_donate_clicked();

    void on_dark_toggled(bool checked);

    void on_light_toggled(bool checked);

    void on_clear_engine_cache_clicked();

    void on_download_engine_clicked();

    void init_engine();

    void on_copy_toggled(bool checked);

    void on_mpeg_toggled(bool checked);

    void on_frameRate_valueChanged(const QString &arg1);

    void on_scale_valueChanged(const QString &arg1);

    void on_restore_clicked();

    void on_jpg_toggled(bool checked);

    void on_png_toggled(bool checked);
private:
    Ui::Settings *ui;

    QSettings settings;

    Engine *engine = nullptr;
};

#endif // SETTINGS_H
