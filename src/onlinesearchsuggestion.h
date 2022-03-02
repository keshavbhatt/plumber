#ifndef ONLINESEARCHSUGGESTION_H
#define ONLINESEARCHSUGGESTION_H

#include <QByteArray>
#include <QEvent>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QTreeWidget>

class onlineSearchSuggestion : public QObject {
  Q_OBJECT

public:
  explicit onlineSearchSuggestion(QLineEdit *parent = nullptr);
  ~onlineSearchSuggestion();
  bool eventFilter(QObject *obj, QEvent *ev) override;
  void showCompletion(const QVector<QString> &choices);

public slots:

  void doneCompletion();
  void preventSuggest();
  void autoSuggest();
  void handleNetworkData(QNetworkReply *networkReply);

private slots:
  bool checkBlackList(QString word);

private:
  QLineEdit *editor = nullptr;
  QTreeWidget *popup = nullptr;
  QTimer timer;
  QNetworkAccessManager networkManager;
  QStringList blacklist;
  QSettings settingsObj;
};

#endif // ONLINESEARCHSUGGESTION_H
