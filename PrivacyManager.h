// PrivacyManager.h

#ifndef PRIVACYMANAGER_H
#define PRIVACYMANAGER_H

#include <QObject>
#include <QWebEngineProfile>
#include <QNetworkProxy>
#include <QMap>
#include <QStringList>

class QWebEngineView;

class PrivacyManager : public QObject
{
    Q_OBJECT

public:
    explicit PrivacyManager(QWebEngineView *webView, QObject *parent = nullptr);

    // VPN
    void toggleVPN();
    bool isVPNActive() const;

    // Ad blocking
    void enableAdBlocking(bool enable);
    bool isAdBlockingEnabled() const;
    void updateAdBlockList(const QStringList &rules);

    // Cookie management
    void clearCookies();
    void setAcceptCookies(bool accept);
    void setThirdPartyCookiesPolicy(QWebEngineProfile::ThirdPartyCookiesPolicy policy);

    // Browsing data
    void clearBrowsingData();
    void clearCache();
    void clearHistory();
    void clearDownloads();

    // HTTPS
    void setHttpsOnlyMode(bool enable);
    bool isHttpsOnlyModeEnabled() const;

    // Do Not Track
    void setDoNotTrack(bool enable);
    bool isDoNotTrackEnabled() const;

    // Fingerprinting protection
    void enableFingerprintingProtection(bool enable);
    bool isFingerprintingProtectionEnabled() const;

    // Content settings
    void setJavaScriptEnabled(bool enable);
    void setPluginsEnabled(bool enable);
    void setPopupsAllowed(bool allow);
    void setGeolocationAllowed(bool allow);
    void setNotificationsAllowed(bool allow);

    // Password management
    void setSavePasswordsEnabled(bool enable);
    bool isSavePasswordsEnabled() const;

    // Proxy settings
    void setProxy(const QNetworkProxy &proxy);
    QNetworkProxy proxy() const;

    // Privacy reports
    QString generatePrivacyReport() const;

public slots:
    void showCookieManager();

signals:
    void vpnStatusChanged(bool active);
    void adBlockingStatusChanged(bool enabled);
    void cookiePolicyChanged();
    void httpsOnlyModeChanged(bool enabled);
    void doNotTrackChanged(bool enabled);
    void fingerprintingProtectionChanged(bool enabled);
    void contentSettingsChanged();
    void savePasswordsEnabledChanged(bool enabled);
    void proxyChanged();

private:
    QWebEngineView *m_webView;
    bool m_vpnActive;
    bool m_adBlockingEnabled;
    bool m_httpsOnlyMode;
    bool m_doNotTrack;
    bool m_fingerprintingProtection;
    bool m_savePasswordsEnabled;
    QNetworkProxy m_proxy;
    QMap<QString, QStringList> m_adBlockLists;

    void initializeAdBlockLists();
    void applyAdBlockRules();
    void updateContentSettings();
};

#endif // PRIVACYMANAGER_H