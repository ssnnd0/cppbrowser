// main.cpp

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QSslConfiguration>
#include <QNetworkProxy>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineUrlScheme>
#include <QFontDatabase>
#include <QStyleFactory>
#include <QThreadPool>
#include <QSysInfo>
#include <QLoggingCategory>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <QDebug>

#include "Browser.h"
#include "config.h"  // Assume this file contains build-time configuration

// Forward declarations
void setupLogging();
void setupTranslations(QApplication& app);
void setupNetworkSettings();
void setupWebEngineSettings();
void setupCustomUrlSchemes();
void loadFonts();
void setupStyleAndTheme(QApplication& app);
void checkSingleInstance();
void checkForUpdates();
void showSplashScreen(QSplashScreen& splash);
void cleanupTempFiles();
void setupSignalHandlers();
void createConfigDirectories();
bool isFirstRun();
void showFirstRunWizard();
void migrateFromPreviousVersion();
void registerFileAssociations();
void setupCrashReporter();
void initializePlugins();
void loadUserScripts();

// Global variables
const QString APP_NAME = "CustomBrowser";
const QString APP_VERSION = "1.0.0";
const QString ORGANIZATION_NAME = "YourCompany";
const QString ORGANIZATION_DOMAIN = "yourcompany.com";

int main(int argc, char *argv[])
{
    // Enable high DPI support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Create application instance
    QApplication app(argc, argv);

    // Set application information
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    // Setup logging
    setupLogging();

    qInfo() << "Starting" << APP_NAME << "version" << APP_VERSION;

    // Parse command-line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Custom Web Browser");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption privateOption("private", "Start in private browsing mode");
    parser.addOption(privateOption);

    QCommandLineOption urlOption(QStringList() << "u" << "url", "Open specified URL on startup", "url");
    parser.addOption(urlOption);

    QCommandLineOption profileOption(QStringList() << "p" << "profile", "Use specified profile", "profile");
    parser.addOption(profileOption);

    QCommandLineOption resetOption("reset", "Reset all settings to default");
    parser.addOption(resetOption);

    QCommandLineOption debugOption("debug", "Enable debug mode");
    parser.addOption(debugOption);

    parser.process(app);

    // Check for single instance
    checkSingleInstance();

    // Show splash screen
    QPixmap pixmap(":/images/splash.png");
    QSplashScreen splash(pixmap);
    showSplashScreen(splash);

    // Setup translations
    setupTranslations(app);

    // Setup network settings
    setupNetworkSettings();

    // Setup WebEngine settings
    setupWebEngineSettings();

    // Setup custom URL schemes
    setupCustomUrlSchemes();

    // Load custom fonts
    loadFonts();

    // Setup style and theme
    setupStyleAndTheme(app);

    // Create config directories
    createConfigDirectories();

    // Check for first run
    if (isFirstRun()) {
        showFirstRunWizard();
    }

    // Migrate from previous version if necessary
    migrateFromPreviousVersion();

    // Register file associations
    registerFileAssociations();

    // Setup crash reporter
    setupCrashReporter();

    // Initialize plugins
    initializePlugins();

    // Load user scripts
    loadUserScripts();

    // Clean up temporary files
    cleanupTempFiles();

    // Setup signal handlers
    setupSignalHandlers();

    // Check for updates
    checkForUpdates();

    // Create and show the browser
    Browser browser;

    // Apply command-line options
    if (parser.isSet(privateOption)) {
        browser.enablePrivateBrowsing(true);
    }

    if (parser.isSet(profileOption)) {
        browser.loadProfile(parser.value(profileOption));
    }

    if (parser.isSet(resetOption)) {
        browser.resetSettings();
    }

    if (parser.isSet(debugOption)) {
        browser.enableDebugMode(true);
    }

    browser.show();
    splash.finish(&browser);

    // If there's a URL passed as a command-line argument, load it
    if (parser.isSet(urlOption)) {
        browser.loadUrl(QUrl::fromUserInput(parser.value(urlOption)));
    } else {
        browser.loadHomePage();
    }

    // Run the application
    int result = app.exec();

    // Perform cleanup
    browser.saveSettings();
    cleanupTempFiles();

    qInfo() << "Exiting" << APP_NAME;

    return result;
}

void setupLogging()
{
    // Set up logging categories
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    // Set up log file
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logPath);
    QString logFile = logPath + "/browser.log";

    // Rotate log files if necessary
    QFileInfo fileInfo(logFile);
    if (fileInfo.size() > 10 * 1024 * 1024) {  // 10 MB
        QFile::rename(logFile, logFile + "." + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }

    // Open log file
    static QFile outFile(logFile);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);

    // Install message handler
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QTextStream out(&outFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
        switch (type) {
            case QtDebugMsg:     out << "Debug: "; break;
            case QtInfoMsg:      out << "Info: "; break;
            case QtWarningMsg:   out << "Warning: "; break;
            case QtCriticalMsg:  out << "Critical: "; break;
            case QtFatalMsg:     out << "Fatal: "; break;
        }
        out << msg << " (" << context.file << ":" << context.line << ", " << context.function << ")\n";
        out.flush();
    });
}

void setupTranslations(QApplication& app)
{
    // Load Qt translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // Load application translations
    QTranslator appTranslator;
    appTranslator.load(":/translations/" + QLocale::system().name());
    app.installTranslator(&appTranslator);
}

void setupNetworkSettings()
{
    // Set up SSL configuration
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    QSslConfiguration::setDefaultConfiguration(sslConfig);

    // Set up proxy if needed
    QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    // You could load proxy settings from a configuration file here
}

void setupWebEngineSettings()
{
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AllowGeolocationOnInsecureOrigins, false);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, false);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PrintElementBackgrounds, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AllowXSSAuditorBypass, false);

    // Set default font settings
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::StandardFont, "Arial");
    QWebEngineSettings::defaultSettings()->setFontSize(QWebEngineSettings::DefaultFontSize, 16);
    QWebEngineSettings::defaultSettings()->setFontSize(QWebEngineSettings::DefaultFixedFontSize, 13);
    QWebEngineSettings::defaultSettings()->setFontSize(QWebEngineSettings::MinimumFontSize, 10);

    // Configure WebEngine profile
    QWebEngineProfile::defaultProfile()->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    QWebEngineProfile::defaultProfile()->setHttpUserAgent("CustomBrowser/1.0");
}

void setupCustomUrlSchemes()
{
    // Register custom URL scheme for the browser
    QWebEngineUrlScheme customScheme("browser");
    customScheme.setFlags(QWebEngineUrlScheme::SecureScheme | 
                          QWebEngineUrlScheme::LocalScheme |
                          QWebEngineUrlScheme::LocalAccessAllowed);
    customScheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    QWebEngineUrlScheme::registerScheme(customScheme);
}

void loadFonts()
{
    QDir fontDir(":/fonts");
    for (const QString &fileName : fontDir.entryList(QDir::Files)) {
        QFontDatabase::addApplicationFont(":/fonts/" + fileName);
    }
}

void setupStyleAndTheme(QApplication& app)
{
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));

    // Set dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    app.setPalette(darkPalette);
}

void checkSingleInstance()
{
    // Implement single instance check using QSharedMemory or QLocalServer
    // For brevity, this implementation is omitted
}

void checkForUpdates()
{
    // Implement update checking logic
    // This could involve making a network request to check for new versions
    // For brevity, this implementation is omitted
}

void showSplashScreen(QSplashScreen& splash)
{
    splash.show();
    splash.showMessage("Loading...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    qApp->processEvents();
}

void cleanupTempFiles()
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + APP_NAME;
    QDir tempDir(tempPath);
    if (tempDir.exists()) {
        tempDir.removeRecursively();
    }
}

void setupSignalHandlers()
{
    // Set up signal handlers for crash reporting
    // This is platform-specific and requires careful implementation
    // For brevity, this implementation is omitted
}

void createConfigDirectories()
{
    QStringList locations = {
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation),
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    };

    for (const QString &location : locations) {
        QDir dir(location);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
    }
}

bool isFirstRun()
{
    QSettings settings;
    return !settings.contains("firstRun");
}

void showFirstRunWizard()
{
    // Implement first run wizard
    // This could guide the user through initial setup and customization
    // For brevity, this implementation is omitted

    QSettings settings;
    settings.setValue("firstRun", false);
}

void migrateFromPreviousVersion()
{
    // Implement migration logic from previous versions
    // This could involve updating settings, bookmarks, or other user data
    // For brevity, this implementation is omitted
}

void registerFileAssociations()
{
    // Register file associations for the browser
    // This is platform-specific and may require system calls or registry edits
    // For brevity, this implementation is omitted
}

void setupCrashReporter()
{
    // Set up crash reporting
    // This could involve integrating a crash reporting library like Breakpad or Crashpad
    // For brevity, a simplified implementation is provided

    QString crashReportsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/crash_reports";
    QDir().mkpath(crashReportsPath);

    // In a real implementation, you would initialize the crash reporting library here
    qInfo() << "Crash reporter initialized. Reports will be saved to:" << crashReportsPath;
}

void initializePlugins()
{
    QString pluginsPath = QCoreApplication::applicationDirPath() + "/plugins";
    QDir pluginsDir(pluginsPath);

    if (pluginsDir.exists()) {
        for (const QString &fileName : pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            if (plugin) {
                // Initialize the plugin
                // In a real implementation, you would have an interface that all plugins implement
                // and you would call the initialization method through that interface
                qInfo() << "Loaded plugin:" << fileName;
            } else {
                qWarning() << "Failed to load plugin:" << fileName << "Error:" << loader.errorString();
            }
        }
    } else {
        qWarning() << "Plugins directory does not exist:" << pluginsPath;
    }
}

void loadUserScripts()
{
    QString scriptsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/user_scripts";
    QDir scriptsDir(scriptsPath);

    if (scriptsDir.exists()) {
        for (const QString &fileName : scriptsDir.entryList(QStringList() << "*.js", QDir::Files)) {
            QFile file(scriptsDir.absoluteFilePath(fileName));
            if (file.open(QIODevice::ReadOnly)) {
                QString scriptContent = file.readAll();
                // In a real implementation, you would inject this script into the WebEngine
                qInfo() << "Loaded user script:" << fileName;
            } else {
                qWarning() << "Failed to load user script:" << fileName;
            }
        }
    } else {
        qInfo() << "User scripts directory does not exist:" << scriptsPath;
    }
}

void setupSecurityPolicies()
{
    // Set up Content Security Policy
    QString csp = "default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: https:; connect-src 'self' https:; font-src 'self'; object-src 'none'; media-src 'self' https:; frame-src 'self' https:;";
    QWebEngineProfile::defaultProfile()->setHttpHeader("Content-Security-Policy", csp);

    // Enable HSTS (HTTP Strict Transport Security)
    QWebEngineProfile::defaultProfile()->setHttpHeader("Strict-Transport-Security", "max-age=31536000; includeSubDomains");

    // Set X-Frame-Options to prevent clickjacking
    QWebEngineProfile::defaultProfile()->setHttpHeader("X-Frame-Options", "SAMEORIGIN");

    // Set X-XSS-Protection
    QWebEngineProfile::defaultProfile()->setHttpHeader("X-XSS-Protection", "1; mode=block");

    // Set X-Content-Type-Options
    QWebEngineProfile::defaultProfile()->setHttpHeader("X-Content-Type-Options", "nosniff");

    // Set Referrer-Policy
    QWebEngineProfile::defaultProfile()->setHttpHeader("Referrer-Policy", "strict-origin-when-cross-origin");
}

void setupDataSynchronization()
{
    // Set up data synchronization
    // This could involve setting up a connection to a cloud service for syncing bookmarks, history, etc.
    // For brevity, a simplified implementation is provided

    QString syncDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sync";
    QDir().mkpath(syncDir);

    // In a real implementation, you would initialize the sync service here
    qInfo() << "Data synchronization initialized. Sync directory:" << syncDir;
}

void setupAccessibility()
{
    // Set up accessibility features
    // This could involve initializing screen reader support, high contrast modes, etc.
    // For brevity, a simplified implementation is provided

    QAccessible::installFactory([](const QString &classname, QObject *object) -> QAccessibleInterface* {
        // Custom accessible interfaces could be created here based on the classname and object
        return nullptr;
    });

    qInfo() << "Accessibility features initialized";
}

void setupPerformanceMonitoring()
{
    // Set up performance monitoring
    // This could involve initializing profiling tools, setting up metrics collection, etc.
    // For brevity, a simplified implementation is provided

    QThread *monitorThread = QThread::create([]() {
        while (true) {
            // Collect and log performance metrics
            qint64 memoryUsage = QProcess::systemEnvironment().indexOf("MEMORY_USAGE");
            qDebug() << "Memory usage:" << memoryUsage << "bytes";

            QThread::sleep(60); // Sleep for 60 seconds
        }
    });
    monitorThread->start();

    qInfo() << "Performance monitoring initialized";
}

void setupNetworkCache()
{
    // Set up network cache
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/network_cache";
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(qApp);
    diskCache->setCacheDirectory(cachePath);
    diskCache->setMaximumCacheSize(100 * 1024 * 1024); // 100 MB
    QWebEngineProfile::defaultProfile()->setHttpCache(diskCache);

    qInfo() << "Network cache initialized. Cache directory:" << cachePath;
}

void setupContentFilters()
{
    // Set up content filters
    // This could involve loading filter lists for ad blocking, privacy protection, etc.
    // For brevity, a simplified implementation is provided

    QString filtersPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/filters";
    QDir().mkpath(filtersPath);

    QFile adBlockList(filtersPath + "/adblock.txt");
    if (adBlockList.open(QIODevice::ReadOnly)) {
        QStringList rules = QString(adBlockList.readAll()).split('\n');
        // In a real implementation, you would parse these rules and apply them to web requests
        qInfo() << "Loaded" << rules.count() << "ad blocking rules";
    }

    qInfo() << "Content filters initialized";
}

void setupExtensionsFramework()
{
    // Set up extensions framework
    // This could involve setting up a system for loading and managing browser extensions
    // For brevity, a simplified implementation is provided

    QString extensionsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/extensions";
    QDir().mkpath(extensionsPath);

    QDir extensionsDir(extensionsPath);
    for (const QString &extDir : extensionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString manifestPath = extensionsPath + "/" + extDir + "/manifest.json";
        QFile manifestFile(manifestPath);
        if (manifestFile.open(QIODevice::ReadOnly)) {
            QJsonDocument manifest = QJsonDocument::fromJson(manifestFile.readAll());
            // In a real implementation, you would parse the manifest and load the extension
            qInfo() << "Loaded extension:" << manifest["name"].toString();
        }
    }

    qInfo() << "Extensions framework initialized";
}

void setupAutoUpdater()
{
    // Set up auto-updater
    // This could involve setting up a system for automatically checking for and installing updates
    // For brevity, a simplified implementation is provided

    QTimer *updateTimer = new QTimer(qApp);
    QObject::connect(updateTimer, &QTimer::timeout, []() {
        // In a real implementation, you would check for updates here
        qInfo() << "Checking for updates...";
    });
    updateTimer->start(24 * 60 * 60 * 1000); // Check once a day

    qInfo() << "Auto-updater initialized";
}

void setupTelemetry()
{
    // Set up telemetry
    // This could involve setting up a system for collecting anonymous usage statistics
    // For brevity, a simplified implementation is provided

    QSettings settings;
    bool telemetryEnabled = settings.value("telemetry/enabled", true).toBool();

    if (telemetryEnabled) {
        // In a real implementation, you would initialize the telemetry system here
        qInfo() << "Telemetry system initialized";
    } else {
        qInfo() << "Telemetry is disabled";
    }
}

void setupDevTools()
{
    // Set up developer tools
    // This could involve initializing the Web Inspector and other development tools
    // For brevity, a simplified implementation is provided

    QWebEngineSettings::globalSettings()->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled, true);

    qInfo() << "Developer tools initialized";
}

int main(int argc, char *argv[])
{
    // Enable high DPI support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Create application instance
    QApplication app(argc, argv);

    // Set application information
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    // Setup logging
    setupLogging();

    qInfo() << "Starting" << APP_NAME << "version" << APP_VERSION;

    try {
        // Parse command-line arguments
        QCommandLineParser parser;
        parser.setApplicationDescription("Custom Web Browser");
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption privateOption("private", "Start in private browsing mode");
        parser.addOption(privateOption);

        QCommandLineOption urlOption(QStringList() << "u" << "url", "Open specified URL on startup", "url");
        parser.addOption(urlOption);

        QCommandLineOption profileOption(QStringList() << "p" << "profile", "Use specified profile", "profile");
        parser.addOption(profileOption);

        QCommandLineOption resetOption("reset", "Reset all settings to default");
        parser.addOption(resetOption);

        QCommandLineOption debugOption("debug", "Enable debug mode");
        parser.addOption(debugOption);

        parser.process(app);

        // Check for single instance
        checkSingleInstance();

        // Show splash screen
        QPixmap pixmap(":/images/splash.png");
        QSplashScreen splash(pixmap);
        showSplashScreen(splash);

        // Setup translations
        setupTranslations(app);

        // Setup network settings
        setupNetworkSettings();

        // Setup WebEngine settings
        setupWebEngineSettings();

        // Setup custom URL schemes
        setupCustomUrlSchemes();

        // Load custom fonts
        loadFonts();

        // Setup style and theme
        setupStyleAndTheme(app);

        // Create config directories
        createConfigDirectories();

        // Check for first run
        if (isFirstRun()) {
            showFirstRunWizard();
        }

        // Migrate from previous version if necessary
        migrateFromPreviousVersion();

        // Register file associations
        registerFileAssociations();

        // Setup crash reporter
        setupCrashReporter();

        // Initialize plugins
        initializePlugins();

        // Load user scripts
        loadUserScripts();

        // Clean up temporary files
        cleanupTempFiles();

        // Setup signal handlers
        setupSignalHandlers();

        // Check for updates
        checkForUpdates();

        // Additional setup functions
        setupSecurityPolicies();
        setupDataSynchronization();
        setupAccessibility();
        setupPerformanceMonitoring();
        setupNetworkCache();
        setupContentFilters();
        setupExtensionsFramework();
        setupAutoUpdater();
        setupTelemetry();
        setupDevTools();

        // Create and show the browser
        Browser browser;

        // Apply command-line options
        if (parser.isSet(privateOption)) {
            browser.enablePrivateBrowsing(true);
        }

        if (parser.isSet(profileOption)) {
            browser.loadProfile(parser.value(profileOption));
        }

        if (parser.isSet(resetOption)) {
            browser.resetSettings();
        }

        if (parser.isSet(debugOption)) {
            browser.enableDebugMode(true);
        }

        browser.show();
        splash.finish(&browser);

        // If there's a URL passed as a command-line argument, load it
        if (parser.isSet(urlOption)) {
            browser.loadUrl(QUrl::fromUserInput(parser.value(urlOption)));
        } else {
            browser.loadHomePage();
        }

        // Run the application
        int result = app.exec();

        // Perform cleanup
        browser.saveSettings();
        cleanupTempFiles();

        qInfo() << "Exiting" << APP_NAME;

        return result;
    }
    catch (const std::exception& e) {
        qCritical() << "Unhandled exception:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error", QString("An unhandled exception occurred: %1").arg(e.what()));
        return 1;
    }
    catch (...) {
        qCritical() << "Unknown exception occurred";
        QMessageBox::critical(nullptr, "Fatal Error", "An unknown error occurred. The application will now exit.");
        return 1;
    }
}