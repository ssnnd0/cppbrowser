// Browser.h

#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QStandardItemModel>
#include <QListView>
#include <QTreeView>
#include <QStackedWidget>
#include <QNetworkAccessManager>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineDownloadItem>

#include "WebPage.h"
#include "PrivacyManager.h"
#include "CustomizationEngine.h"
#include "DeveloperTools.h"
#include "MediaController.h"
#include "AIAssistant.h"

class Browser : public QMainWindow
{
    Q_OBJECT

public:
    explicit Browser(QWidget *parent = nullptr);
    ~Browser();

    void loadUrl(const QUrl &url);
    void setStartupUrl(const QUrl &url);
    void enablePrivateBrowsing(bool enable);

public slots:
    void newTab(const QUrl &url = QUrl());
    void closeTab(int index);
    void closeCurrentTab();
    void nextTab();
    void previousTab();
    void duplicateTab();
    void reloadTab();
    void stopLoading();

    void navigateBack();
    void navigateForward();
    void navigateHome();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void findInPage();
    void print();
    void viewPageSource();

    void showBookmarks();
    void addBookmark();
    void showHistory();
    void clearHistory();

    void showDownloads();
    void clearDownloads();

    void showSettings();
    void toggleFullScreen();

    void showAboutDialog();
    void checkForUpdates();

private slots:
    void handleUrlChanged(const QUrl &url);
    void handleLoadStarted();
    void handleLoadProgress(int progress);
    void handleLoadFinished(bool ok);
    void handleIconChanged(const QIcon &icon);
    void handleTitleChanged(const QString &title);

    void handleTabChanged(int index);
    void handleTabCloseRequested(int index);

    void handleFullScreenRequest(QWebEngineFullScreenRequest request);
    void handleDownloadRequested(QWebEngineDownloadItem *download);

    void handleCustomContextMenuRequested(const QPoint &pos);

    void handleBookmarkClicked(const QModelIndex &index);
    void handleHistoryClicked(const QModelIndex &index);

    void handleFindTextChanged(const QString &text);
    void handleFindNextClicked();
    void handleFindPreviousClicked();

    void handleAIAssistantResponse(const QString &response);

private:
    void setupUI();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWidgets();

    void setupConnections();
    void setupShortcuts();

    void loadSettings();
    void saveSettings();

    void updateWindowTitle();
    void updateNavigationActions();

    QWebEngineView *currentWebView() const;
    WebPage *currentPage() const;

    AccessibilityManager *m_accessibilityManager;
    QTabWidget *m_tabWidget;
    QLineEdit *m_urlBar;
    QProgressBar *m_progressBar;

    QDockWidget *m_bookmarksDock;
    QListView *m_bookmarksView;
    QStandardItemModel *m_bookmarksModel;

    QDockWidget *m_historyDock;
    QTreeView *m_historyView;
    QStandardItemModel *m_historyModel;

    QDockWidget *m_downloadsDock;
    QListView *m_downloadsView;
    QStandardItemModel *m_downloadsModel;

    QDockWidget *m_developerToolsDock;
    DeveloperTools *m_developerTools;

    QWebEngineProfile *m_profile;
    QWebEngineProfile *m_privateProfile;

    PrivacyManager *m_privacyManager;
    CustomizationEngine *m_customizationEngine;
    MediaController *m_mediaController;
    AIAssistant *m_aiAssistant;

    QNetworkAccessManager *m_networkManager;

    QAction *m_backAction;
    QAction *m_forwardAction;
    QAction *m_reloadAction;
    QAction *m_stopAction;
    QAction *m_homeAction;

    QAction *m_newTabAction;
    QAction *m_closeTabAction;
    QAction *m_nextTabAction;
    QAction *m_previousTabAction;

    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_resetZoomAction;

    QAction *m_findAction;
    QAction *m_printAction;
    QAction *m_viewSourceAction;

    QAction *m_bookmarksAction;
    QAction *m_addBookmarkAction;
    QAction *m_historyAction;
    QAction *m_clearHistoryAction;

    QAction *m_downloadsAction;
    QAction *m_clearDownloadsAction;

    QAction *m_settingsAction;
    QAction *m_fullScreenAction;

    QAction *m_aboutAction;
    QAction *m_updateAction;

    bool m_isPrivateBrowsing;
    QUrl m_startupUrl;
};

#endif // BROWSER_H