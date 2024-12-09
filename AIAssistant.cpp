// AIAssistant.cpp

#include "AIAssistant.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AIAssistant::AIAssistant(QObject *parent)
    : QObject(parent)
{
    // TODO: Load API key from secure storage
    m_apiKey = "YOUR_API_KEY_HERE";
}

void AIAssistant::processQuery(const QString &query)
{
    sendRequest(query);
}

void AIAssistant::sendRequest(const QString &query)
{
    QNetworkRequest request(QUrl("https://api.openai.com/v1/engines/davinci-codex/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    QJsonObject jsonBody;
    jsonBody["prompt"] = query;
    jsonBody["max_tokens"] = 150;
    jsonBody["n"] = 1;
    jsonBody["stop"] = QJsonArray();
    jsonBody["temperature"] = 0.7;

    QNetworkReply *reply = m_networkManager.post(request, QJsonDocument(jsonBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleResponse(reply);
    });
}

void AIAssistant::handleResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject jsonObject = response.object();
        QJsonArray choices = jsonObject["choices"].toArray();
        if (!choices.isEmpty()) {
            QString aiResponse = choices[0].toObject()["text"].toString();
            emit responseReady(aiResponse.trimmed());
        }
    } else {
        emit responseReady("Error: " + reply->errorString());
    }
    reply->deleteLater();
}