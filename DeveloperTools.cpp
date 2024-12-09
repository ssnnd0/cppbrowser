// DeveloperTools.cpp

#include "DeveloperTools.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QLabel>
#include <QDateTime>
#include <QStandardItemModel>
#include <QJsonObject>
#include <QJsonArray>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

DeveloperTools::DeveloperTools(QWebEngineView *webView, QWidget *parent)
    : QWidget(parent)
    , m_webView(webView)
{
    setupUI();
    setupConnections();
}

DeveloperTools::~DeveloperTools()
{
    // Clean up any resources if needed
}

void DeveloperTools::toggleVisibility()
{
    setVisible(!isVisible());
}

void DeveloperTools::inspectElement()
{
    m_tabWidget->setCurrentWidget(m_elementInspector);
    m_elementInspector->startInspection();
}

void DeveloperTools::showConsole()
{
    m_tabWidget->setCurrentWidget(m_consolePanel);
}

void DeveloperTools::showNetworkMonitor()
{
    m_tabWidget->setCurrentWidget(m_networkMonitor);
}

void DeveloperTools::startPerformanceProfile()
{
    m_tabWidget->setCurrentWidget(m_performanceProfiler);
    m_performanceProfiler->startProfiling();
}

void DeveloperTools::handleTabChange(int index)
{
    // Handle tab changes if needed
}

void DeveloperTools::handleConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    m_consolePanel->appendMessage(level, message, lineNumber, sourceID);
}

void DeveloperTools::handleNetworkRequestFinished(QNetworkReply *reply)
{
    m_networkMonitor->addRequest(reply);
}

void DeveloperTools::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    m_elementInspector = new ElementInspector(m_webView);
    m_tabWidget->addTab(m_elementInspector, "Elements");

    m_consolePanel = new ConsolePanel(m_webView);
    m_tabWidget->addTab(m_consolePanel, "Console");

    m_networkMonitor = new NetworkMonitor(m_webView->page()->profile()->networkAccessManager());
    m_tabWidget->addTab(m_networkMonitor, "Network");

    m_performanceProfiler = new PerformanceProfiler(m_webView);
    m_tabWidget->addTab(m_performanceProfiler, "Performance");

    setLayout(mainLayout);
}

void DeveloperTools::setupConnections()
{
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &DeveloperTools::handleTabChange);
    connect(m_webView->page(), &QWebEnginePage::consoleMessage, this, &DeveloperTools::handleConsoleMessage);
    connect(m_webView->page()->profile()->networkAccessManager(), &QNetworkAccessManager::finished, this, &DeveloperTools::handleNetworkRequestFinished);
}

// ElementInspector implementation

ElementInspector::ElementInspector(QWebEngineView *webView, QWidget *parent)
    : QWidget(parent)
    , m_webView(webView)
{
    setupUI();
    injectInspectionScript();
}

void ElementInspector::startInspection()
{
    m_webView->page()->runJavaScript("window.startElementInspection();");
}

void ElementInspector::stopInspection()
{
    m_webView->page()->runJavaScript("window.stopElementInspection();");
}

void ElementInspector::handleElementSelected(const QJsonObject &elementInfo)
{
    // Update the element tree and style editor with the selected element's information
}

void ElementInspector::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    m_elementTree = new QTreeView(this);
    splitter->addWidget(m_elementTree);

    m_styleEditor = new QTextEdit(this);
    splitter->addWidget(m_styleEditor);

    setLayout(mainLayout);
}

void ElementInspector::injectInspectionScript()
{
    QString scriptSource = R"(
        window.startElementInspection = function() {
            // Implement element inspection logic
        };

        window.stopElementInspection = function() {
            // Implement logic to stop element inspection
        };

        window.sendElementInfo = function(elementInfo) {
            // Send element info back to C++
        };
    )";

    QWebEngineScript script;
    script.setName("ElementInspector");
    script.setSourceCode(scriptSource);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::MainWorld);
    m_webView->page()->scripts().insert(script);
}

// ConsolePanel implementation

ConsolePanel::ConsolePanel(QWebEngineView *webView, QWidget *parent)
    : QWidget(parent)
    , m_webView(webView)
{
    setupUI();
}

void ConsolePanel::appendMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    QString levelStr;
    switch (level) {
        case QWebEnginePage::InfoMessageLevel:
            levelStr = "Info";
            break;
        case QWebEnginePage::WarningMessageLevel:
            levelStr = "Warning";
            break;
        case QWebEnginePage::ErrorMessageLevel:
            levelStr = "Error";
            break;
    }

    QString formattedMessage = QString("[%1] %2 (Line %3, Source: %4)")
                                   .arg(levelStr)
                                   .arg(message)
                                   .arg(lineNumber)
                                   .arg(sourceID);

    m_consoleOutput->append(formattedMessage);
}

void ConsolePanel::clearConsole()
{
    m_consoleOutput->clear();
}

void ConsolePanel::executeJavaScript()
{
    QString code = m_consoleInput->text();
    m_webView->page()->runJavaScript(code, [this](const QVariant &result) {
        m_consoleOutput->append("> " + m_consoleInput->text());
        m_consoleOutput->append(result.toString());
        m_consoleInput->clear();
    });
}

void ConsolePanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_consoleOutput = new QTextEdit(this);
    m_consoleOutput->setReadOnly(true);
    mainLayout->addWidget(m_consoleOutput);

    m_consoleInput = new QLineEdit(this);
    mainLayout->addWidget(m_consoleInput);

    connect(m_consoleInput, &QLineEdit::returnPressed, this, &ConsolePanel::executeJavaScript);

    setLayout(mainLayout);
}

// NetworkMonitor implementation

NetworkMonitor::NetworkMonitor(QNetworkAccessManager *networkManager, QWidget *parent)
    : QWidget(parent)
    , m_networkManager(networkManager)
{
    setupUI();
}

void NetworkMonitor::addRequest(QNetworkReply *reply)
{
    // Add the network request to the requests tree
    // Update the request details when selected
}

void NetworkMonitor::clearRequests()
{
    // Clear all requests from the tree and details view
}

void NetworkMonitor::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    m_requestsTree = new QTreeView(this);
    splitter->addWidget(m_requestsTree);

    m_requestDetails = new QTextEdit(this);
    m_requestDetails->setReadOnly(true);
    splitter->addWidget(m_requestDetails);

    setLayout(mainLayout);
}

// PerformanceProfiler implementation

PerformanceProfiler::PerformanceProfiler(QWebEngineView *webView, QWidget *parent)
    : QWidget(parent)
    , m_webView(webView)
{
    setupUI();
    injectProfilingScript();
}

void PerformanceProfiler::startProfiling()
{
    m_webView->page()->runJavaScript("window.startProfiling();");
}

void PerformanceProfiler::stopProfiling()
{
    m_webView->page()->runJavaScript("window.stopProfiling();");
}

void PerformanceProfiler::handleProfilingData(const QJsonDocument &data)
{
    // Process and display the profiling data
}

void PerformanceProfiler::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    m_profileTree = new QTreeView(this);
    splitter->addWidget(m_profileTree);

    m_profileDetails = new QTextEdit(this);
    m_profileDetails->setReadOnly(true);
    splitter->addWidget(m_profileDetails);

    setLayout(mainLayout);
}

void PerformanceProfiler::injectProfilingScript()
{
    QString scriptSource = R"(
        window.startProfiling = function() {
            // Implement profiling start logic
        };

        window.stopProfiling = function() {
            // Implement profiling stop logic and send data back to C++
        };
    )";

    QWebEngineScript script;
    script.setName("PerformanceProfiler");
    script.setSourceCode(scriptSource);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::MainWorld);
    m_webView->page()->scripts().insert(script);
}