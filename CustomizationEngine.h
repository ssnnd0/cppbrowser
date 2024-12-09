// CustomizationEngine.h

#ifndef CUSTOMIZATIONENGINE_H
#define CUSTOMIZATIONENGINE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QColor>
#include <QFont>
#include <QJsonObject>

class QWebEngineView;
class QWidget;

enum class TabOrientation {
    Horizontal,
    Vertical
};

struct Theme {
    QString name;
    QColor backgroundColor;
    QColor textColor;
    QColor accentColor;
    QFont mainFont;
    // Add more theme properties as needed
};

class CustomizationEngine : public QObject
{
    Q_OBJECT

public:
    explicit CustomizationEngine(QWidget *mainWindow, QWebEngineView *webView, QObject *parent = nullptr);

    void applyTheme(const QString& themeName);
    void setTabOrientation(TabOrientation orientation);
    void injectCustomCSS(const QString& url, const QString& css);
    void saveCustomization();
    void loadCustomization();

    // UI Layout customization
    void setToolbarPosition(Qt::ToolBarArea area);
    void setStatusBarVisibility(bool visible);
    void setSidebarPosition(Qt::DockWidgetArea area);

    // Font customization
    void setGlobalFont(const QFont& font);
    void setMinimumFontSize(int size);

    // Color scheme customization
    void setColorScheme(const QString& schemeName);

    // Extension management
    void installExtension(const QString& extensionPath);
    void uninstallExtension(const QString& extensionId);
    void enableExtension(const QString& extensionId, bool enable);

    // Custom scripts
    void addUserScript(const QString& name, const QString& script, const QStringList& urlPatterns);
    void removeUserScript(const QString& name);

    // Keyboard shortcuts
    void setKeyboardShortcut(const QString& actionName, const QKeySequence& shortcut);

    // Privacy settings
    void setDoNotTrack(bool enable);
    void setThirdPartyCookiesPolicy(QWebEngineProfile::ThirdPartyCookiesPolicy policy);

signals:
    void themeChanged(const QString& themeName);
    void tabOrientationChanged(TabOrientation orientation);
    void customCSSInjected(const QString& url);
    void toolbarPositionChanged(Qt::ToolBarArea area);
    void statusBarVisibilityChanged(bool visible);
    void sidebarPositionChanged(Qt::DockWidgetArea area);
    void globalFontChanged(const QFont& font);
    void minimumFontSizeChanged(int size);
    void colorSchemeChanged(const QString& schemeName);
    void extensionInstalled(const QString& extensionId);
    void extensionUninstalled(const QString& extensionId);
    void extensionEnabledStateChanged(const QString& extensionId, bool enabled);
    void userScriptAdded(const QString& name);
    void userScriptRemoved(const QString& name);
    void keyboardShortcutChanged(const QString& actionName, const QKeySequence& shortcut);
    void doNotTrackChanged(bool enabled);
    void thirdPartyCookiesPolicyChanged(QWebEngineProfile::ThirdPartyCookiesPolicy policy);

private:
    QWidget *m_mainWindow;
    QWebEngineView *m_webView;
    QMap<QString, Theme> m_themes;
    QMap<QString, QString> m_customCSS;
    QMap<QString, QKeySequence> m_keyboardShortcuts;

    void initializeDefaultThemes();
    void applyThemeToWidget(QWidget *widget, const Theme &theme);
    void updateExtensionSettings();
    void saveSettingsToJson(const QString &filename);
    void loadSettingsFromJson(const QString &filename);
};

#endif // CUSTOMIZATIONENGINE_H