// DeveloperTools.h

#ifndef DEVELOPERTOOLS_H
#define DEVELOPERTOOLS_H

#include <QObject>
#include <QWidget>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QTabWidget>
#include <QTreeView>
#include <QTextEdit>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QJsonDocument>

class ElementInspector;
class ConsolePanel;
class NetworkMonitor;
class PerformanceProfiler;

class DeveloperTools : public QWidget
{
    Q_OBJECT

public:
    explicit DeveloperTools(QWebEngineView *webView, QWidget *parent = nullptr);
    ~DeveloperTools();

    void toggleVisibility();
    void inspectElement();
    void showConsole();
    void showNetworkMonitor();
    void startPerformanceProfile();

private slots:
    void handleTabChange(int index);
    void handleConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID);
    void handleNetworkRequestFinished(QNetworkReply *reply);

private:
    void setupUI();
    void setupConnections();

    QWebEngineView *m_webView;
    QTabWidget *m_tabWidget;
    ElementInspector *m_elementInspector;
    ConsolePanel *m_consolePanel;
    NetworkMonitor *m_networkMonitor;
    PerformanceProfiler *m_performanceProfiler;
};

class ElementInspector : public QWidget
{
    Q_OBJECT

public:
    explicit ElementInspector(QWebEngineView *webView, QWidget *parent = nullptr);

    void startInspection();
    void stopInspection();

private slots:
    void handleElementSelected(const QJsonObject &elementInfo);

private:
    void setupUI();
    void injectInspectionScript();

    QWebEngineView *m_webView;
    QTreeView *m_elementTree;
    QTextEdit *m_styleEditor;
};

class ConsolePanel : public QWidget
{
    Q_OBJECT

public:
    explicit ConsolePanel(QWebEngineView *webView, QWidget *parent = nullptr);

    void appendMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID);
    void clearConsole();

private slots:
    void executeJavaScript();

private:
    void setupUI();

    QWebEngineView *m_webView;
    QTextEdit *m_consoleOutput;
    QLineEdit *m_consoleInput;
};

class NetworkMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkMonitor(QNetworkAccessManager *networkManager, QWidget *parent = nullptr);

    void addRequest(QNetworkReply *reply);
    void clearRequests();

private:
    void setupUI();

    QNetworkAccessManager *m_networkManager;
    QTreeView *m_requestsTree;
    QTextEdit *m_requestDetails;
};

class PerformanceProfiler : public QWidget
{
    Q_OBJECT

public:
    explicit PerformanceProfiler(QWebEngineView *webView, QWidget *parent = nullptr);

    void startProfiling();
    void stopProfiling();

private slots:
    void handleProfilingData(const QJsonDocument &data);

private:
    void setupUI();
    void injectProfilingScript();

    QWebEngineView *m_webView;
    QTreeView *m_profileTree;
    QTextEdit *m_profileDetails;
};

#endif // DEVELOPERTOOLS_H