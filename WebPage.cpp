// WebPage.cpp

#include "WebPage.h"
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QAuthenticator>
#include <QMessageBox>

WebPage::WebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
    , m_contentBlockingEnabled(false)
    , m_customCSSEnabled(false)
    , m_customJSEnabled(false)
{
    connect(this, &QWebEnginePage::authenticationRequired,
            this, &WebPage::handleAuthenticationRequired);
    connect(this, &QWebEnginePage::proxyAuthenticationRequired,
            this, &WebPage::handleProxyAuthenticationRequired);
    connect(this, &QWebEnginePage::featurePermissionRequested,
            this, &WebPage::handleFeaturePermissionRequested);
    connect(this, &QWebEnginePage::renderProcessTerminated,
            this, &WebPage::handleRenderProcessTerminated);
}

bool WebPage::certificateError(const QWebEngineCertificateError &error)
{
    QMessageBox::StandardButton btn = QMessageBox::warning(
        nullptr,
        tr("Security Error"),
        error.errorDescription(),
        QMessageBox::Abort | QMessageBox::Ignore,
        QMessageBox::Abort);

    return btn == QMessageBox::Ignore;
}

void WebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    // Log JavaScript console messages
    QString levelStr;
    switch (level) {
        case InfoMessageLevel: levelStr = "Info"; break;
        case WarningMessageLevel: levelStr = "Warning"; break;
        case ErrorMessageLevel: levelStr = "Error"; break;
    }

    qDebug() << "JavaScript" << levelStr << "at line" << lineNumber << "in" << sourceID << ":" << message;
}

void WebPage::setCustomUserAgent(const QString &userAgent)
{
    m_customUserAgent = userAgent;
    profile()->setHttpUserAgent(userAgent);
}

QString WebPage::customUserAgent() const
{
    return m_customUserAgent;
}

void WebPage::enableContentBlocking(bool enable)
{
    m_contentBlockingEnabled = enable;
    // In a real implementation, this would set up content blocking rules
}

bool WebPage::isContentBlockingEnabled() const
{
    return m_contentBlockingEnabled;
}

void WebPage::setCustomHeaders(const QMap<QString, QString> &headers)
{
    m_customHeaders = headers;
    // In a real implementation, this would modify the network request to include these headers
}

QMap<QString, QString> WebPage::customHeaders() const
{
    return m_customHeaders;
}

void WebPage::enableCustomCSS(bool enable)
{
    m_customCSSEnabled = enable;
    if (enable && !m_customCSS.isEmpty()) {
        injectCustomCSS();
    }
}

void WebPage::setCustomCSS(const QString &css)
{
    m_customCSS = css;
    if (m_customCSSEnabled) {
        injectCustomCSS();
    }
}

QString WebPage::customCSS() const
{
    return m_customCSS;
}

void WebPage::enableCustomJS(bool enable)
{
    m_customJSEnabled = enable;
    if (enable && !m_customJS.isEmpty()) {
        injectCustomJS();
    }
}

void WebPage::setCustomJS(const QString &js)
{
    m_customJS = js;
    if (m_customJSEnabled) {
        injectCustomJS();
    }
}

QString WebPage::customJS() const
{
    return m_customJS;
}

bool WebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if (m_contentBlockingEnabled) {
        // Implement content blocking logic here
        // Return false if the URL should be blocked
    }

    // Implement any custom navigation logic here

    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QWebEnginePage *WebPage::createWindow(WebWindowType type)
{
    // Create a new WebPage for new windows/tabs
    WebPage *newPage = new WebPage(profile(), this);
    
    // Copy settings from this page to the new page
    newPage->setCustomUserAgent(m_customUserAgent);
    newPage->enableContentBlocking(m_contentBlockingEnabled);
    newPage->setCustomHeaders(m_customHeaders);
    newPage->setCustomCSS(m_customCSS);
    newPage->enableCustomCSS(m_customCSSEnabled);
    newPage->setCustomJS(m_customJS);
    newPage->enableCustomJS(m_customJSEnabled);

    return newPage;
}

void WebPage::handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator)
{
    // Handle authentication requests
    // In a real implementation, this would show a dialog to enter credentials
    qDebug() << "Authentication required for" << requestUrl.toString();
}

void WebPage::handleProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost)
{
    // Handle proxy authentication requests
    // In a real implementation, this would show a dialog to enter proxy credentials
    qDebug() << "Proxy authentication required for" << requestUrl.toString() << "via" << proxyHost;
}

void WebPage::handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature)
{
    // Handle permission requests (e.g., geolocation, notifications)
    // In a real implementation, this would show a dialog to ask the user for permission
    qDebug() << "Permission requested for" << securityOrigin.toString() << "feature:" << feature;
}

void WebPage::handleRenderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode)
{
    QString status;
    switch (terminationStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = "Normal";
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = "Abnormal";
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = "Crashed";
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = "Killed";
            break;
    }

    qWarning() << "Render process terminated with status:" << status << "and exit code:" << exitCode;

    // In a real implementation, you might want to show an error page or reload the page
}

void WebPage::injectCustomCSS()
{
    QWebEngineScript script;
    script.setName("CustomCSS");
    script.setSourceCode(QString("(function() {"
                                 "    var style = document.createElement('style');"
                                 "    style.type = 'text/css';"
                                 "    style.innerHTML = %1;"
                                 "    document.head.appendChild(style);"
                                 "})();").arg(m_customCSS));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    script.setRunsOnSubFrames(true);

    scripts().insert(script);
}

void WebPage::injectCustomJS()
{
    QWebEngineScript script;
    script.setName("CustomJS");
    script.setSourceCode(m_customJS);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    script.setRunsOnSubFrames(true);

    scripts().insert(script);
}