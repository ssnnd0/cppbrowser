// AIAssistant.h

#ifndef AIASSISTANT_H
#define AIASSISTANT_H

#include <QObject>
#include <QNetworkAccessManager>

class AIAssistant : public QObject
{
    Q_OBJECT

public:
    explicit AIAssistant(QObject *parent = nullptr);

public slots:
    void processQuery(const QString &query);

signals:
    void responseReady(const QString &response);

private:
    QNetworkAccessManager m_networkManager;
    QString m_apiKey;

    void sendRequest(const QString &query);
    void handleResponse(QNetworkReply *reply);
};

#endif // AIASSISTANT_H