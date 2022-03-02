#include "searchprovider.h"
#include "onlinesearchsuggestion.h"
#include "ui_searchprovider.h"
#include "utils.h"

SearchProvider::SearchProvider(QWidget *parent)
    : QWidget(parent), ui(new Ui::SearchProvider) {
  ui->setupUi(this);
  ui->search->setEnabled(false);
  onlineSearchSuggestion *ss = new onlineSearchSuggestion(ui->term);
  ui->term->installEventFilter(ss);
  // loader is the child of wall_view
  _loader = new WaitingSpinnerWidget(ui->results, true, false);
  _loader->setRoundness(70.0);
  _loader->setMinimumTrailOpacity(15.0);
  _loader->setTrailFadePercentage(70.0);
  _loader->setNumberOfLines(10);
  _loader->setLineLength(8);
  _loader->setLineWidth(2);
  _loader->setInnerRadius(2);
  _loader->setRevolutionsPerSecond(3);
  _loader->setColor(QColor("#1e90ff"));
  ui->term->addAction(QIcon(":/icons/youtube-line.png"),
                      QLineEdit::LeadingPosition);
}

SearchProvider::~SearchProvider() { delete ui; }

void SearchProvider::on_term_textChanged(const QString &arg1) {
  ui->search->setEnabled(arg1.isEmpty() == false);
}

void SearchProvider::on_search_clicked() {
  ui->results->clear();

  if (_request != nullptr) {
    _request->blockSignals(true);
    _request->disconnect();
    _request->deleteLater();
    _request->blockSignals(false);
    _request = nullptr;
    _loader->stop();
  }
  if (_request == nullptr) {
    _request = new Request(this);
    connect(_request, &Request::requestStarted, [=]() { _loader->start(); });
    connect(_request, &Request::requestFinished, [=](QString reply) {
      // load to view
      processResult(reply);
      _loader->stop();
    });
    connect(_request, &Request::downloadError, [=](QString errorString) {
      _loader->stop();
      showError(errorString);
    });
  }
  QString url =
      QUrl("http://ktechpit.com/USS/Olivia/manual_youtube_search.php?query=" +
           ui->term->text().trimmed())
          .toString(QUrl::EncodeSpaces);
  _request->get(QUrl(url));
}

void SearchProvider::showError(QString message) {
  // init error
  _error = new Error(this);
  _error->setAttribute(Qt::WA_DeleteOnClose);
  _error->setWindowTitle(QApplication::applicationName() + " | Error dialog");
  _error->setWindowFlag(Qt::Dialog);
  _error->setWindowModality(Qt::NonModal);
  _error->setError("An Error ocurred while processing your request!", message);
  _error->show();
}

void SearchProvider::processResult(QString reply) {
  QStringList list = reply.split("gettrackinfo(");
  list.removeFirst();
  foreach (QString str, list) {
    QString dataStr = QString(str.split(");\">").first()).remove("&quot;");
    if (!dataStr.isEmpty() &&
        (QString(dataStr.at(0)) == "'" || QString(dataStr.at(0)) == "\"")) {
      dataStr = dataStr.remove("'");
      dataStr = dataStr.remove("\"");
    }
    QString videoId, title, artist, album, coverUrl, songId, albumId, artistId,
        millis;
    QStringList arr = dataStr.split("!=-=!");
    title = arr[0];
    artist = arr[1];
    album = arr[2];
    coverUrl = arr[3];
    songId = arr[4];
    albumId = arr[5];
    artistId = arr[6];
    millis = arr[7];
    if (albumId.contains("undefined")) { // yt case
      videoId = arr[4];
    }

    // get meta from p
    QString p =
        str.split(QString("<p style=\"margin-left:13.6em  !important\">"))
            .last()
            .split(QString("</p>"))
            .first();
    QString p_title =
        p.split("Duration: ").first().prepend("<b>").append("</b>");
    QString p_des = "Duration: " + p.split("Duration: ").last();

    if (songId.isEmpty() == false) {
      QWidget *track_widget = new QWidget(ui->results);
      track_widget->setObjectName("track-widget-" + songId);
      track_ui.setupUi(track_widget);
      track_ui.title->setText(utils::htmlToPlainText(p_title).trimmed());
      track_ui.meta->setText(p_des);

      connect(track_ui.trackButton, &QPushButton::clicked,
              [=]() { emit loadVideo(songId); });

      QString cache_path =
          QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

      QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
      diskCache->setCacheDirectory(cache_path);
      double ratio = 320.0 / 180.0;
      double height = track_widget->height();
      double width = ratio * height;

      track_ui.cover->setFixedSize(width, height);
      track_ui.cover->setRemotePixmap(coverUrl, diskCache);

      QListWidgetItem *item;
      item = new QListWidgetItem(ui->results);
      track_ui.widget->adjustSize();
      item->setSizeHint(track_widget->minimumSizeHint());
      ui->results->setItemWidget(item, track_widget);
      ui->results->addItem(item);
    }
  }
}

void SearchProvider::on_term_returnPressed() {
  if (ui->search->isEnabled()) {
    ui->search->click();
  }
}
