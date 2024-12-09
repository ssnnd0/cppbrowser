// WebPage.h

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>

class WebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    WebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    bool certificateError(const QWebEngineCertificateError &error) override;
    QWebEnginePage *createWindow(WebWindowType type) override;

private slots:
    void handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator);
    void handleProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost);
    void handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature);
};

#endif // WEBPAGE_H