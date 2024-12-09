// WebPage.h

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QMap>

class WebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit WebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

    bool certificateError(const QWebEngineCertificateError &error) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) override;

    void setCustomUserAgent(const QString &userAgent);
    QString customUserAgent() const;

    void enableContentBlocking(bool enable);
    bool isContentBlockingEnabled() const;

    void setCustomHeaders(const QMap<QString, QString> &headers);
    QMap<QString, QString> customHeaders() const;

    void enableCustomCSS(bool enable);
    void setCustomCSS(const QString &css);
    QString customCSS() const;

    void enableCustomJS(bool enable);
    void setCustomJS(const QString &js);
    QString customJS() const;

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
    QWebEnginePage *createWindow(WebWindowType type) override;

private slots:
    void handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator);
    void handleProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost);
    void handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature);
    void handleRenderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode);

private:
    QString m_customUserAgent;
    bool m_contentBlockingEnabled;
    QMap<QString, QString> m_customHeaders;
    QString m_customCSS;
    QString m_customJS;
    bool m_customCSSEnabled;
    bool m_customJSEnabled;

    void injectCustomCSS();
    void injectCustomJS();
};

#endif // WEBPAGE_H