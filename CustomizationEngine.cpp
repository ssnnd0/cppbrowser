// CustomizationEngine.cpp

#include "CustomizationEngine.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

CustomizationEngine::CustomizationEngine(QWidget *mainWindow, QWebEngineView *webView, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_webView(webView)
{
    initializeDefaultThemes();
    loadCustomization();
}

void CustomizationEngine::applyTheme(const QString& themeName)
{
    if (!m_themes.contains(themeName)) {
        qWarning() << "Theme not found:" << themeName;
        return;
    }

    const Theme &theme = m_themes[themeName];

    // Apply theme to main window
    applyThemeToWidget(m_mainWindow, theme);

    // Apply theme to web view
    QString css = QString(
        "body {"
        "  background-color: %1;"
        "  color: %2;"
        "}"
        "a { color: %3; }"
    ).arg(theme.backgroundColor.name())
     .arg(theme.textColor.name())
     .arg(theme.accentColor.name());

    m_webView->page()->runJavaScript(QString("document.body.style.cssText = '%1';").arg(css));

    // Update application palette
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Window, theme.backgroundColor);
    palette.setColor(QPalette::WindowText, theme.textColor);
    palette.setColor(QPalette::Base, theme.backgroundColor.lighter(110));
    palette.setColor(QPalette::AlternateBase, theme.backgroundColor.darker(110));
    palette.setColor(QPalette::Text, theme.textColor);
    palette.setColor(QPalette::Button, theme.backgroundColor);
    palette.setColor(QPalette::ButtonText, theme.textColor);
    palette.setColor(QPalette::Highlight, theme.accentColor);
    palette.setColor(QPalette::HighlightedText, theme.backgroundColor);
    QApplication::setPalette(palette);

    // Set global font
    QApplication::setFont(theme.mainFont);

    emit themeChanged(themeName);
}

void CustomizationEngine::setTabOrientation(TabOrientation orientation)
{
    QTabWidget *tabWidget = m_mainWindow->findChild<QTabWidget*>();
    if (tabWidget) {
        tabWidget->setTabPosition(orientation == TabOrientation::Vertical ? QTabWidget::West : QTabWidget::North);
        emit tabOrientationChanged(orientation);
    }
}

void CustomizationEngine::injectCustomCSS(const QString& url, const QString& css)
{
    m_customCSS[url] = css;

    QWebEngineScript script;
    script.setName("CustomCSS_" + url);
    script.setSourceCode(QString(
        "(function() {"
        "    var style = document.createElement('style');"
        "    style.type = 'text/css';"
        "    style.innerHTML = '%1';"
        "    document.head.appendChild(style);"
        "})();"
    ).arg(css.replace("'", "\\'")));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    script.setRunsOnSubFrames(true);

    m_webView->page()->scripts().insert(script);

    emit customCSSInjected(url);
}

void CustomizationEngine::setToolbarPosition(Qt::ToolBarArea area)
{
    QMainWindow *mainWindow = qobject_cast<QMainWindow*>(m_mainWindow);
    if (mainWindow) {
        QToolBar *toolbar = mainWindow->findChild<QToolBar*>();
        if (toolbar) {
            mainWindow->removeToolBar(toolbar);
            mainWindow->addToolBar(area, toolbar);
            emit toolbarPositionChanged(area);
        }
    }
}

void CustomizationEngine::setStatusBarVisibility(bool visible)
{
    QMainWindow *mainWindow = qobject_cast<QMainWindow*>(m_mainWindow);
    if (mainWindow) {
        mainWindow->statusBar()->setVisible(visible);
        emit statusBarVisibilityChanged(visible);
    }
}

void CustomizationEngine::setSidebarPosition(Qt::DockWidgetArea area)
{
    QMainWindow *mainWindow = qobject_cast<QMainWindow*>(m_mainWindow);
    if (mainWindow) {
        QDockWidget *sidebar = mainWindow->findChild<QDockWidget*>();
        if (sidebar) {
            mainWindow->removeDockWidget(sidebar);
            mainWindow->addDockWidget(area, sidebar);
            emit sidebarPositionChanged(area);
        }
    }
}

void CustomizationEngine::setGlobalFont(const QFont& font)
{
    QApplication::setFont(font);
    emit globalFontChanged(font);
}

void CustomizationEngine::setMinimumFontSize(int size)
{
    m_webView->settings()->setFontSize(QWebEngineSettings::MinimumFontSize, size);
    emit minimumFontSizeChanged(size);
}

void CustomizationEngine::setColorScheme(const QString& schemeName)
{
    // Implement color scheme logic here
    // This could involve changing the theme and additional custom styling
    emit colorSchemeChanged(schemeName);
}

void CustomizationEngine::installExtension(const QString& extensionPath)
{
    // Implement extension installation logic
    // This would involve parsing the extension manifest, copying files, and updating settings
    QString extensionId = QFileInfo(extensionPath).baseName(); // Simple ID generation
    emit extensionInstalled(extensionId);
    updateExtensionSettings();
}

void CustomizationEngine::uninstallExtension(const QString& extensionId)
{
    // Implement extension uninstallation logic
    // This would involve removing extension files and updating settings
    emit extensionUninstalled(extensionId);
    updateExtensionSettings();
}

void CustomizationEngine::enableExtension(const QString& extensionId, bool enable)
{
    // Implement extension enable/disable logic
    // This would involve updating the extension's enabled state in settings
    emit extensionEnabledStateChanged(extensionId, enable);
    updateExtensionSettings();
}

void CustomizationEngine::addUserScript(const QString& name, const QString& script, const QStringList& urlPatterns)
{
    QWebEngineScript userScript;
    userScript.setName(name);
    userScript.setSourceCode(script);
    userScript.setInjectionPoint(QWebEngineScript::DocumentReady);
    userScript.setWorldId(QWebEngineScript::ApplicationWorld);
    userScript.setRunsOnSubFrames(true);

    for (const QString& pattern : urlPatterns) {
        userScript.setUrlPattern(QUrl(pattern));
    }

    m_webView->page()->scripts().insert(userScript);
    emit userScriptAdded(name);
}

void CustomizationEngine::removeUserScript(const QString& name)
{
    QWebEngineScript script = m_webView->page()->scripts().findScript(name);
    if (!script.isNull()) {
        m_webView->page()->scripts().remove(script);
        emit userScriptRemoved(name);
    }
}

void CustomizationEngine::setKeyboardShortcut(const QString& actionName, const QKeySequence& shortcut)
{
    m_keyboardShortcuts[actionName] = shortcut;
    // Update the actual QAction shortcut here
    emit keyboardShortcutChanged(actionName, shortcut);
}

void CustomizationEngine::setDoNotTrack(bool enable)
{
    m_webView->page()->profile()->setHttpAcceptLanguage(enable ? "DNT=1" : QString());
    emit doNotTrackChanged(enable);
}

void CustomizationEngine::setThirdPartyCookiesPolicy(QWebEngineProfile::ThirdPartyCookiesPolicy policy)
{
    m_webView->page()->profile()->setThirdPartyCookiePolicy(policy);
    emit thirdPartyCookiesPolicyChanged(policy);
}

void CustomizationEngine::saveCustomization()
{
    saveSettingsToJson("customization.json");
}

void CustomizationEngine::loadCustomization()
{
    loadSettingsFromJson("customization.json");
}

void CustomizationEngine::initializeDefaultThemes()
{
    // Light theme
    Theme lightTheme;
    lightTheme.name = "Light";
    lightTheme.backgroundColor = QColor("#FFFFFF");
    lightTheme.textColor = QColor("#000000");
    lightTheme.accentColor = QColor("#1E90FF");
    lightTheme.mainFont = QFont("Arial", 12);
    m_themes["Light"] = lightTheme;

    // Dark theme
    Theme darkTheme;
    darkTheme.name = "Dark";
    darkTheme.backgroundColor = QColor("#2D2D2D");
    darkTheme.textColor = QColor("#FFFFFF");
    darkTheme.accentColor = QColor("#00BFFF");
    darkTheme.mainFont = QFont("Arial", 12);
    m_themes["Dark"] = darkTheme;

    // Add more default themes here
}

void CustomizationEngine::applyThemeToWidget(QWidget *widget, const Theme &theme)
{
    QPalette palette = widget->palette();
    palette.setColor(QPalette::Window, theme.backgroundColor);
    palette.setColor(QPalette::WindowText, theme.textColor);
    palette.setColor(QPalette::Base, theme.backgroundColor.lighter(110));
    palette.setColor(QPalette::AlternateBase, theme.backgroundColor.darker(110));
    palette.setColor(QPalette::Text, theme.textColor);
    palette.setColor(QPalette::Button, theme.backgroundColor);
    palette.setColor(QPalette::ButtonText, theme.textColor);
    palette.setColor(QPalette::Highlight, theme.accentColor);
    palette.setColor(QPalette::HighlightedText, theme.backgroundColor);
    widget->setPalette(palette);

    widget->setFont(theme.mainFont);

    // Recursively apply theme to child widgets
    for (QObject *child : widget->children()) {
        if (QWidget *childWidget = qobject_cast<QWidget*>(child)) {
            applyThemeToWidget(childWidget, theme);
        }
    }
}

void CustomizationEngine::updateExtensionSettings()
{
    // Implement logic to update extension settings
    // This could involve updating a configuration file or database
}

void CustomizationEngine::saveSettingsToJson(const QString &filename)
{
    QJsonObject root;

    // Save theme
    root["currentTheme"] = m_themes.keys().contains(m_mainWindow->property("currentTheme").toString()) ?
                           m_mainWindow->property("currentTheme").toString() : "Light";

    // Save tab orientation
    QTabWidget *tabWidget = m_mainWindow->findChild<QTabWidget*>();
    if (tabWidget) {
        root["tabOrientation"] = tabWidget->tabPosition() == QTabWidget::West ? "Vertical" : "Horizontal";
    }

    // Save custom CSS
    QJsonObject customCSSObj;
    for (auto it = m_customCSS.constBegin(); it != m_customCSS.constEnd(); ++it) {
        customCSSObj[it.key()] = it.value();
    }
    root["customCSS"] = customCSSObj;

// Save toolbar position
    QMainWindow *mainWindow = qobject_cast<QMainWindow*>(m_mainWindow);
    if (mainWindow) {
        QToolBar *toolbar = mainWindow->findChild<QToolBar*>();
        if (toolbar) {
            root["toolbarPosition"] = static_cast<int>(mainWindow->toolBarArea(toolbar));
        }
    }

    // Save status bar visibility
    if (mainWindow) {
        root["statusBarVisible"] = mainWindow->statusBar()->isVisible();
    }

    // Save sidebar position
    if (mainWindow) {
        QDockWidget *sidebar = mainWindow->findChild<QDockWidget*>();
        if (sidebar) {
            root["sidebarPosition"] = static_cast<int>(mainWindow->dockWidgetArea(sidebar));
        }
    }

    // Save global font
    QFont globalFont = QApplication::font();
    QJsonObject fontObj;
    fontObj["family"] = globalFont.family();
    fontObj["size"] = globalFont.pointSize();
    fontObj["weight"] = globalFont.weight();
    fontObj["italic"] = globalFont.italic();
    root["globalFont"] = fontObj;

    // Save minimum font size
    root["minimumFontSize"] = m_webView->settings()->fontSize(QWebEngineSettings::MinimumFontSize);

    // Save color scheme
    root["colorScheme"] = m_mainWindow->property("currentColorScheme").toString();

    // Save extensions
    QJsonArray extensionsArray;
    // Implement logic to save installed extensions and their states
    root["extensions"] = extensionsArray;

    // Save user scripts
    QJsonArray userScriptsArray;
    const auto scripts = m_webView->page()->scripts().toList();
    for (const QWebEngineScript &script : scripts) {
        if (script.worldId() == QWebEngineScript::ApplicationWorld) {
            QJsonObject scriptObj;
            scriptObj["name"] = script.name();
            scriptObj["source"] = script.sourceCode();
            scriptObj["urlPattern"] = script.urlPattern().pattern();
            userScriptsArray.append(scriptObj);
        }
    }
    root["userScripts"] = userScriptsArray;

    // Save keyboard shortcuts
    QJsonObject shortcutsObj;
    for (auto it = m_keyboardShortcuts.constBegin(); it != m_keyboardShortcuts.constEnd(); ++it) {
        shortcutsObj[it.key()] = it.value().toString();
    }
    root["keyboardShortcuts"] = shortcutsObj;

    // Save privacy settings
    root["doNotTrack"] = m_webView->page()->profile()->httpAcceptLanguage().contains("DNT=1");
    root["thirdPartyCookiesPolicy"] = static_cast<int>(m_webView->page()->profile()->thirdPartyCookiePolicy());

    // Write to file
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
        file.close();
    } else {
        qWarning() << "Failed to save customization settings to" << filename;
    }
}

void CustomizationEngine::loadSettingsFromJson(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load customization settings from" << filename;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format in" << filename;
        return;
    }

    QJsonObject root = doc.object();

    // Load theme
    if (root.contains("currentTheme")) {
        applyTheme(root["currentTheme"].toString());
    }

    // Load tab orientation
    if (root.contains("tabOrientation")) {
        setTabOrientation(root["tabOrientation"].toString() == "Vertical" ? TabOrientation::Vertical : TabOrientation::Horizontal);
    }

    // Load custom CSS
    if (root.contains("customCSS") && root["customCSS"].isObject()) {
        QJsonObject customCSSObj = root["customCSS"].toObject();
        for (auto it = customCSSObj.constBegin(); it != customCSSObj.constEnd(); ++it) {
            injectCustomCSS(it.key(), it.value().toString());
        }
    }

    // Load toolbar position
    if (root.contains("toolbarPosition")) {
        setToolbarPosition(static_cast<Qt::ToolBarArea>(root["toolbarPosition"].toInt()));
    }

    // Load status bar visibility
    if (root.contains("statusBarVisible")) {
        setStatusBarVisibility(root["statusBarVisible"].toBool());
    }

    // Load sidebar position
    if (root.contains("sidebarPosition")) {
        setSidebarPosition(static_cast<Qt::DockWidgetArea>(root["sidebarPosition"].toInt()));
    }

    // Load global font
    if (root.contains("globalFont") && root["globalFont"].isObject()) {
        QJsonObject fontObj = root["globalFont"].toObject();
        QFont font;
        font.setFamily(fontObj["family"].toString());
        font.setPointSize(fontObj["size"].toInt());
        font.setWeight(fontObj["weight"].toInt());
        font.setItalic(fontObj["italic"].toBool());
        setGlobalFont(font);
    }

    // Load minimum font size
    if (root.contains("minimumFontSize")) {
        setMinimumFontSize(root["minimumFontSize"].toInt());
    }

    // Load color scheme
    if (root.contains("colorScheme")) {
        setColorScheme(root["colorScheme"].toString());
    }

    // Load extensions
    if (root.contains("extensions") && root["extensions"].isArray()) {
        QJsonArray extensionsArray = root["extensions"].toArray();
        for (const QJsonValue &extensionValue : extensionsArray) {
            if (extensionValue.isObject()) {
                QJsonObject extensionObj = extensionValue.toObject();
                // Implement logic to load and enable extensions
            }
        }
    }

    // Load user scripts
    if (root.contains("userScripts") && root["userScripts"].isArray()) {
        QJsonArray userScriptsArray = root["userScripts"].toArray();
        for (const QJsonValue &scriptValue : userScriptsArray) {
            if (scriptValue.isObject()) {
                QJsonObject scriptObj = scriptValue.toObject();
                addUserScript(scriptObj["name"].toString(),
                              scriptObj["source"].toString(),
                              QStringList() << scriptObj["urlPattern"].toString());
            }
        }
    }

    // Load keyboard shortcuts
    if (root.contains("keyboardShortcuts") && root["keyboardShortcuts"].isObject()) {
        QJsonObject shortcutsObj = root["keyboardShortcuts"].toObject();
        for (auto it = shortcutsObj.constBegin(); it != shortcutsObj.constEnd(); ++it) {
            setKeyboardShortcut(it.key(), QKeySequence(it.value().toString()));
        }
    }

    // Load privacy settings
    if (root.contains("doNotTrack")) {
        setDoNotTrack(root["doNotTrack"].toBool());
    }
    if (root.contains("thirdPartyCookiesPolicy")) {
        setThirdPartyCookiesPolicy(static_cast<QWebEngineProfile::ThirdPartyCookiesPolicy>(root["thirdPartyCookiesPolicy"].toInt()));
    }
}

// Additional helper methods

void CustomizationEngine::createCustomTheme(const QString &name, const QColor &backgroundColor, const QColor &textColor, const QColor &accentColor, const QFont &font)
{
    Theme newTheme;
    newTheme.name = name;
    newTheme.backgroundColor = backgroundColor;
    newTheme.textColor = textColor;
    newTheme.accentColor = accentColor;
    newTheme.mainFont = font;
    m_themes[name] = newTheme;
}

void CustomizationEngine::removeCustomTheme(const QString &name)
{
    if (m_themes.contains(name) && name != "Light" && name != "Dark") {
        m_themes.remove(name);
    }
}

QStringList CustomizationEngine::availableThemes() const
{
    return m_themes.keys();
}

void CustomizationEngine::setZoomFactor(qreal factor)
{
    m_webView->setZoomFactor(factor);
}

qreal CustomizationEngine::zoomFactor() const
{
    return m_webView->zoomFactor();
}

void CustomizationEngine::resetToDefaultSettings()
{
    // Implement logic to reset all customization settings to their default values
    applyTheme("Light");
    setTabOrientation(TabOrientation::Horizontal);
    setToolbarPosition(Qt::TopToolBarArea);
    setStatusBarVisibility(true);
    setSidebarPosition(Qt::LeftDockWidgetArea);
    setGlobalFont(QFont("Arial", 12));
    setMinimumFontSize(12);
    setColorScheme("Default");
    setZoomFactor(1.0);
    setDoNotTrack(false);
    setThirdPartyCookiesPolicy(QWebEngineProfile::AllowThirdPartyCookies);

    // Clear custom CSS
    m_customCSS.clear();

    // Remove all user scripts
    m_webView->page()->scripts().clear();

    // Reset keyboard shortcuts to default
    m_keyboardShortcuts.clear();
    // Add default shortcuts here

    // Remove all extensions
    // Implement logic to remove all installed extensions

    saveCustomization();
}