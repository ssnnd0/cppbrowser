// Browser.cpp

#include "Browser.h"
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include <QShortcut>
#include <QStyle>
#include <QTimer>
#include <QWebEngineHistory>
#include <QWebEngineSettings>

Browser::Browser(QWidget *parent)
    : QMainWindow(parent)
    , m_webView(new QWebEngineView(this))
    , m_tabWidget(new QTabWidget(this))
    , m_urlBar(new QLineEdit(this))
    , m_progressBar(new QProgressBar(this))
    , m_profile(new QWebEngineProfile(this))
    , m_privateProfile(new QWebEngineProfile(this))
    , m_privacyManager(new PrivacyManager(this))
    , m_customizationEngine(new CustomizationEngine(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_isPrivateBrowsing(false)
    , m_startupUrl(QUrl("https://www.example.com"))
{
    setupUI();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWidgets();

    setupConnections();
    setupShortcuts();

    loadSettings();

    m_privateProfile->setOffTheRecord(true);

    newTab();
}

Browser::~Browser()
{
    saveSettings();
}

void Browser::loadUrl(const QUrl &url)
{
    if (currentWebView()) {
        currentWebView()->load(url);
    }
}

void Browser::setStartupUrl(const QUrl &url)
{
    m_startupUrl = url;
}

void Browser::enablePrivateBrowsing(bool enable)
{
    m_isPrivateBrowsing = enable;
    QWebEngineProfile *profile = enable ? m_privateProfile : m_profile;

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(i));
        if (view) {
            WebPage *page = qobject_cast<WebPage*>(view->page());
            if (page) {
                page->setProfile(profile);
            }
        }
    }

    updateWindowTitle();
}

void Browser::newTab(const QUrl &url)
{
    QWebEngineView *webView = new QWebEngineView(this);
    WebPage *page = new WebPage(m_isPrivateBrowsing ? m_privateProfile : m_profile, webView);
    webView->setPage(page);

    int index = m_tabWidget->addTab(webView, tr("New Tab"));
    m_tabWidget->setCurrentIndex(index);

    connect(webView, &QWebEngineView::urlChanged, this, &Browser::handleUrlChanged);
    connect(webView, &QWebEngineView::loadStarted, this, &Browser::handleLoadStarted);
    connect(webView, &QWebEngineView::loadProgress, this, &Browser::handleLoadProgress);
    connect(webView, &QWebEngineView::loadFinished, this, &Browser::handleLoadFinished);
    connect(webView, &QWebEngineView::iconChanged, this, &Browser::handleIconChanged);
    connect(webView, &QWebEngineView::titleChanged, this, &Browser::handleTitleChanged);

    connect(page, &WebPage::fullScreenRequested, this, &Browser::handleFullScreenRequest);
    connect(page, &WebPage::downloadRequested, this, &Browser::handleDownloadRequested);

    webView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(webView, &QWidget::customContextMenuRequested, this, &Browser::handleCustomContextMenuRequested);

    if (url.isValid()) {
        webView->load(url);
    } else {
        webView->load(m_startupUrl);
    }
}

void Browser::closeTab(int index)
{
    if (m_tabWidget->count() > 1) {
        QWidget *widget = m_tabWidget->widget(index);
        m_tabWidget->removeTab(index);
        delete widget;
    } else {
        close();
    }
}

void Browser::closeCurrentTab()
{
    closeTab(m_tabWidget->currentIndex());
}

void Browser::nextTab()
{
    int next = (m_tabWidget->currentIndex() + 1) % m_tabWidget->count();
    m_tabWidget->setCurrentIndex(next);
}

void Browser::previousTab()
{
    int prev = (m_tabWidget->currentIndex() - 1 + m_tabWidget->count()) % m_tabWidget->count();
    m_tabWidget->setCurrentIndex(prev);
}

void Browser::duplicateTab()
{
    if (currentWebView()) {
        newTab(currentWebView()->url());
    }
}

void Browser::reloadTab()
{
    if (currentWebView()) {
        currentWebView()->reload();
    }
}

void Browser::stopLoading()
{
    if (currentWebView()) {
        currentWebView()->stop();
    }
}

void Browser::navigateBack()
{
    if (currentWebView()) {
        currentWebView()->back();
    }
}

void Browser::navigateForward()
{
    if (currentWebView()) {
        currentWebView()->forward();
    }
}

void Browser::navigateHome()
{
    loadUrl(m_startupUrl);
}

void Browser::zoomIn()
{
    if (currentWebView()) {
        currentWebView()->setZoomFactor(currentWebView()->zoomFactor() * 1.1);
    }
}

void Browser::zoomOut()
{
    if (currentWebView()) {
        currentWebView()->setZoomFactor(currentWebView()->zoomFactor() / 1.1);
    }
}

void Browser::resetZoom()
{
    if (currentWebView()) {
        currentWebView()->setZoomFactor(1.0);
    }
}

void Browser::findInPage()
{
    // Implement find in page functionality
}

void Browser::print()
{
    if (currentWebView()) {
        QPrinter printer;
        QPrintDialog dialog(&printer, this);
        if (dialog.exec() == QDialog::Accepted) {
            currentWebView()->page()->print(&printer, [](bool) {});
        }
    }
}

void Browser::viewPageSource()
{
    if (currentWebView()) {
        currentWebView()->page()->toHtml([this](const QString &html) {
            QTextEdit *sourceView = new QTextEdit(this);
            sourceView->setReadOnly(true);
            sourceView->setPlainText(html);
            sourceView->resize(800, 600);
            sourceView->show();
        });
    }
}

void Browser::showBookmarks()
{
    m_bookmarksDock->show();
}

void Browser::addBookmark()
{
    if (currentWebView()) {
        QString title = currentWebView()->title();
        QString url = currentWebView()->url().toString();
        
        QStandardItem *item = new QStandardItem(title);
        item->setData(url, Qt::UserRole);
        m_bookmarksModel->appendRow(item);
    }
}

void Browser::showHistory()
{
    m_historyDock->show();
}

void Browser::clearHistory()
{
    m_historyModel->clear();
    if (currentWebView()) {
        currentWebView()->history()->clear();
    }
}

void Browser::showDownloads()
{
    m_downloadsDock->show();
}

void Browser::clearDownloads()
{
    m_downloadsModel->clear();
}

void Browser::showSettings()
{
    // Implement settings dialog
}

void Browser::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void Browser::showAboutDialog()
{
    QMessageBox::about(this, tr("About Browser"),
                       tr("Custom Web Browser\n"
                          "Version 1.0\n"
                          "© 2023 Your Company"));
}

void Browser::checkForUpdates()
{
    // Implement update checking functionality
}

void Browser::handleUrlChanged(const QUrl &url)
{
    m_urlBar->setText(url.toString());
    updateNavigationActions();
}

void Browser::handleLoadStarted()
{
    m_progressBar->setValue(0);
    m_progressBar->show();
    m_stopAction->setEnabled(true);
}

void Browser::handleLoadProgress(int progress)
{
    m_progressBar->setValue(progress);
}

void Browser::handleLoadFinished(bool ok)
{
    m_progressBar->hide();
    m_stopAction->setEnabled(false);
    updateNavigationActions();

    if (!ok) {
        // Handle load error
    }
}

void Browser::handleIconChanged(const QIcon &icon)
{
    int index = m_tabWidget->indexOf(qobject_cast<QWebEngineView*>(sender()));
    if (index != -1) {
        m_tabWidget->setTabIcon(index, icon);
    }
}

void Browser::handleTitleChanged(const QString &title)
{
    int index = m_tabWidget->indexOf(qobject_cast<QWebEngineView*>(sender()));
    if (index != -1) {
        m_tabWidget->setTabText(index, title);
        if (index == m_tabWidget->currentIndex()) {
            updateWindowTitle();
        }
    }
}

void Browser::handleTabChanged(int index)
{
    if (index != -1) {
        QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(index));
        if (view) {
            m_urlBar->setText(view->url().toString());
            updateWindowTitle();
            updateNavigationActions();
        }
    }
}

void Browser::handleTabCloseRequested(int index)
{
    closeTab(index);
}

void Browser::handleFullScreenRequest(QWebEngineFullScreenRequest request)
{
    if (request.toggleOn()) {
        showFullScreen();
    } else {
        showNormal();
    }
    request.accept();
}

void Browser::handleDownloadRequested(QWebEngineDownloadItem *download)
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath() + "/" + download->suggestedFileName());
    if (!path.isEmpty()) {
        download->setPath(path);
        download->accept();
        
        QStandardItem *item = new QStandardItem(download->suggestedFileName());
        item->setData(QVariant::fromValue(download), Qt::UserRole);
        m_downloadsModel->appendRow(item);
        
        connect(download, &QWebEngineDownloadItem::downloadProgress, this, [this, item](qint64 bytesReceived, qint64 bytesTotal) {
            int progress = (bytesReceived * 100) / bytesTotal;
            item->setData(progress, Qt::UserRole + 1);
        });
        
        connect(download, &QWebEngineDownloadItem::finished, this, [this, item]() {
            item->setData(100, Qt::UserRole + 1);
        });
    }
}

void Browser::handleCustomContextMenuRequested(const QPoint &pos)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    if (!view)
        return;

    QMenu menu;
    menu.addAction(m_backAction);
    menu.addAction(m_forwardAction);
    menu.addAction(m_reloadAction);
    menu.addSeparator();
    menu.addAction(m_viewSourceAction);
    menu.addSeparator();

    QWebEngineContextMenuData contextMenuData = view->page()->contextMenuData();
    if (!contextMenuData.linkUrl().isEmpty()) {
        menu.addAction(tr("Open Link in New Tab"), [this, url = contextMenuData.linkUrl()]() {
            newTab(url);
        });
        menu.addAction(tr("Copy Link Address"), [url = contextMenuData.linkUrl()]() {
            QApplication::clipboard()->setText(url.toString());
        });
    }

    if (!contextMenuData.selectedText().isEmpty()) {
        menu.addAction(tr("Copy"), [view]() {
            view->page()->triggerAction(QWebEnginePage::Copy);
        });
    }

    menu.exec(view->mapToGlobal(pos));
}

void Browser::handleBookmarkClicked(const QModelIndex &index)
{
    QString url = index.data(Qt::UserRole).toString();
    if (!url.isEmpty()) {
        loadUrl(QUrl(url));
    }
}

void Browser::handleHistoryClicked(const QModelIndex &index)
{
    QString url = index.data(Qt::UserRole).toString();
    if (!url.isEmpty()) {
        loadUrl(QUrl(url));
    }
}

void Browser::handleFindTextChanged(const QString &text)
{
    if (currentWebView()) {
        currentWebView()->findText(text);
    }
}

void Browser::handleFindNextClicked()
{
    if (currentWebView()) {
        currentWebView()->findText(m_findBar->text());
    }
}

void Browser::handleFindPreviousClicked()
{
    if (currentWebView()) {
        currentWebView()->findText(m_findBar->text(), QWebEnginePage::FindBackward);
    }
}

void Browser::handleAIAssistantResponse(const QString &response)
{
    // Display AI assistant response in a dialog or sidebar
    QMessageBox::information(this, tr("AI Assistant"), response);
}

void Browser::setupUI()
{
    setCentralWidget(m_tabWidget);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    m_urlBar->setClearButtonEnabled(true);

    m_progressBar->setMaximumWidth(120);
    m_progressBar->setMaximumHeight(14);
    m_progressBar->setTextVisible(false);

    resize(1024, 768);
}

void Browser::createActions()
{
    m_backAction = new QAction(style()->standardIcon(QStyle::SP_ArrowBack), tr("Back"), this);
    m_forwardAction = new QAction(style()->standardIcon(QStyle::SP_ArrowForward), tr("Forward"), this);
    m_reloadAction = new QAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Reload"), this);
    m_stopAction = new QAction(style()->standardIcon(QStyle::SP_BrowserStop), tr("Stop"), this);
    m_homeAction = new QAction(style()->standardIcon(QStyle::SP_DirHomeIcon), tr("Home"), this);

    m_newTabAction = new QAction(tr("New Tab"), this);
    m_closeTabAction = new QAction(tr("Close Tab"), this);
    m_nextTabAction = new QAction(tr("Next Tab"), this);
    m_previousTabAction = new QAction(tr("Previous Tab"), this);

    m_zoomInAction = new QAction(tr("Zoom In"), this);
    m_zoomOutAction = new QAction(tr("Zoom Out"), this);
    m_resetZoomAction = new QAction(tr("Reset Zoom"), this);

    m_findAction = new QAction(tr("Find"), this);
    m_printAction = new QAction(tr("Print"), this);
    m_viewSourceAction = new QAction(tr("View Page Source"), this);

    m_bookmarksAction = new QAction(tr("Show Bookmarks"), this);
    m_addBookmarkAction = new QAction(tr("Add Bookmark"), this);
    m_historyAction = new QAction(tr("Show History"), this);
    m_clearHistoryAction = new QAction(tr("Clear History"), this);

    m_downloadsAction = new QAction(tr("Show Downloads"), this);
    m_clearDownloadsAction = new QAction(tr("Clear Downloads"), this);

    m_settingsAction = new QAction(tr("Settings"), this);
    m_fullScreenAction = new QAction(tr("Toggle Full Screen"), this);

    m_aboutAction = new QAction(tr("About"), this);
    m_updateAction = new QAction(tr("Check for Updates"), this);

    m_accessibilityManager = new AccessibilityManager(m_webView, this);
    
}

void Browser::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_newTabAction);
    fileMenu->addAction(m_closeTabAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_printAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Exit"), this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_findAction);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_zoomInAction);
    viewMenu->addAction(m_zoomOutAction);
    viewMenu->addAction(m_resetZoomAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_fullScreenAction);

    QMenu *historyMenu = menuBar()->addMenu(tr("&History"));
    historyMenu->addAction(m_historyAction);
    historyMenu->addAction(m_clearHistoryAction);

    QMenu *bookmarksMenu = menuBar()->addMenu(tr("&Bookmarks"));
    bookmarksMenu->addAction(m_bookmarksAction);
    bookmarksMenu->addAction(m_addBookmarkAction);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(m_downloadsAction);
    toolsMenu->addAction(m_viewSourceAction);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_settingsAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_aboutAction);
    helpMenu->addAction(m_updateAction);
}

void Browser::createToolBars()
{
    QToolBar *navigationBar = addToolBar(tr("Navigation"));
    navigationBar->addAction(m_backAction);
    navigationBar->addAction(m_forwardAction);
    navigationBar->addAction(m_reloadAction);
    navigationBar->addAction(m_stopAction);
    navigationBar->addAction(m_homeAction);
    navigationBar->addWidget(m_urlBar);
}

void Browser::createStatusBar()
{
    statusBar()->addPermanentWidget(m_progressBar);
}

void Browser::createDockWidgets()
{
    m_bookmarksDock = new QDockWidget(tr("Bookmarks"), this);
    m_bookmarksView = new QListView(m_bookmarksDock);
    m_bookmarksModel = new QStandardItemModel(this);
    m_bookmarksView->setModel(m_bookmarksModel);
    m_bookmarksDock->setWidget(m_bookmarksView);
    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarksDock);
    m_bookmarksDock->hide();

    m_historyDock = new QDockWidget(tr("History"), this);
    m_historyView = new QTreeView(m_historyDock);
    m_historyModel = new QStandardItemModel(this);
    m_historyView->setModel(m_historyModel);
    m_historyDock->setWidget(m_historyView);
    addDockWidget(Qt::LeftDockWidgetArea, m_historyDock);
    m_historyDock->hide();

    m_downloadsDock = new QDockWidget(tr("Downloads"), this);
    m_downloadsView = new QListView(m_downloadsDock);
    m_downloadsModel = new QStandardItemModel(this);
    m_downloadsView->setModel(m_downloadsModel);
    m_downloadsDock->setWidget(m_downloadsView);
    addDockWidget(Qt::BottomDockWidgetArea, m_downloadsDock);
    m_downloadsDock->hide();

    m_developerToolsDock = new QDockWidget(tr("Developer Tools"), this);
    m_developerTools = new DeveloperTools(currentWebView(), this);
    m_developerToolsDock->setWidget(m_developerTools);
    addDockWidget(Qt::BottomDockWidgetArea, m_developerToolsDock);
    m_developerToolsDock->hide();
}

void Browser::setupConnections()
{
    connect(m_urlBar, &QLineEdit::returnPressed, this, [this]() {
        loadUrl(QUrl::fromUserInput(m_urlBar->text()));
    });

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &Browser::handleTabChanged);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &Browser::handleTabCloseRequested);

    connect(m_backAction, &QAction::triggered, this, &Browser::navigateBack);
    connect(m_forwardAction, &QAction::triggered, this, &Browser::navigateForward);
    connect(m_reloadAction, &QAction::triggered, this, &Browser::reloadTab);
    connect(m_stopAction, &QAction::triggered, this, &Browser::stopLoading);
    connect(m_homeAction, &QAction::triggered, this, &Browser::navigateHome);

    connect(m_newTabAction, &QAction::triggered, this, &Browser::newTab);
    connect(m_closeTabAction, &QAction::triggered, this, &Browser::closeCurrentTab);
    connect(m_nextTabAction, &QAction::triggered, this, &Browser::nextTab);
    connect(m_previousTabAction, &QAction::triggered, this, &Browser::previousTab);

    connect(m_zoomInAction, &QAction::triggered, this, &Browser::zoomIn);
    connect(m_zoomOutAction, &QAction::triggered, this, &Browser::zoomOut);
    connect(m_resetZoomAction, &QAction::triggered, this, &Browser::resetZoom);

    connect(m_findAction, &QAction::triggered, this, &Browser::findInPage);
    connect(m_printAction, &QAction::triggered, this, &Browser::print);
    connect(m_viewSourceAction, &QAction::triggered, this, &Browser::viewPageSource);

    connect(m_bookmarksAction, &QAction::triggered, this, &Browser::showBookmarks);
    connect(m_addBookmarkAction, &QAction::triggered, this, &Browser::addBookmark);
    connect(m_historyAction, &QAction::triggered, this, &Browser::showHistory);
    connect(m_clearHistoryAction, &QAction::triggered, this, &Browser::clearHistory);

    connect(m_downloadsAction, &QAction::triggered, this, &Browser::showDownloads);
    connect(m_clearDownloadsAction, &QAction::triggered, this, &Browser::clearDownloads);

    connect(m_settingsAction, &QAction::triggered, this, &Browser::showSettings);
    connect(m_fullScreenAction, &QAction::triggered, this, &Browser::toggleFullScreen);

    connect(m_aboutAction, &QAction::triggered, this, &Browser::showAboutDialog);
    connect(m_updateAction, &QAction::triggered, this, &Browser::checkForUpdates);

    connect(m_bookmarksView, &QListView::clicked, this, &Browser::handleBookmarkClicked);
    connect(m_historyView, &QTreeView::clicked, this, &Browser::handleHistoryClicked);

    connect(m_accessibilityToggleAction, &QAction::toggled, m_accessibilityManager, &AccessibilityManager::toggleScreenReader);
    connect(m_fontSizeSlider, &QSlider::valueChanged, m_accessibilityManager, &AccessibilityManager::setFontSize);
    
    connect(m_aiAssistant, &AIAssistant::responseReady, this, &Browser::handleAIAssistantResponse);
}

void Browser::setupShortcuts()
{
    QShortcut *newTabShortcut = new QShortcut(QKeySequence::AddTab, this);
    connect(newTabShortcut, &QShortcut::activated, this, &Browser::newTab);

    QShortcut *closeTabShortcut = new QShortcut(QKeySequence::Close, this);
    connect(closeTabShortcut, &QShortcut::activated, this, &Browser::closeCurrentTab);

    QShortcut *nextTabShortcut = new QShortcut(QKeySequence::NextChild, this);
    connect(nextTabShortcut, &QShortcut::activated, this, &Browser::nextTab);

    QShortcut *previousTabShortcut = new QShortcut(QKeySequence::PreviousChild, this);
    connect(previousTabShortcut, &QShortcut::activated, this, &Browser::previousTab);

    QShortcut *findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, &Browser::findInPage);

    QShortcut *printShortcut = new QShortcut(QKeySequence::Print, this);
    connect(printShortcut, &QShortcut::activated, this, &Browser::print);

    QShortcut *zoomInShortcut = new QShortcut(QKeySequence::ZoomIn, this);
    connect(zoomInShortcut, &QShortcut::activated, this, &Browser::zoomIn);

    QShortcut *zoomOutShortcut = new QShortcut(QKeySequence::ZoomOut, this);
    connect(zoomOutShortcut, &QShortcut::activated, this, &Browser::zoomOut);

    QShortcut *fullScreenShortcut = new QShortcut(Qt::Key_F11, this);
    connect(fullScreenShortcut, &QShortcut::activated, this, &Browser::toggleFullScreen);
}

void Browser::loadSettings()
{
    QSettings settings("YourCompany", "CustomBrowser");

    // Load general settings
    m_startupUrl = settings.value("general/startup_url", QUrl("https://www.example.com")).toUrl();
    restoreGeometry(settings.value("window/geometry").toByteArray());
    restoreState(settings.value("window/state").toByteArray());

    // Load privacy settings
    bool privateMode = settings.value("privacy/private_mode", false).toBool();
    enablePrivateBrowsing(privateMode);

    // Load customization settings
    QString theme = settings.value("customization/theme", "default").toString();
    m_customizationEngine->applyTheme(theme);

    // Load bookmarks
    int bookmarkCount = settings.beginReadArray("bookmarks");
    for (int i = 0; i < bookmarkCount; ++i) {
        settings.setArrayIndex(i);
        QString title = settings.value("title").toString();
        QString url = settings.value("url").toString();
        QStandardItem *item = new QStandardItem(title);
        item->setData(url, Qt::UserRole);
        m_bookmarksModel->appendRow(item);
    }
    settings.endArray();

    // Load other settings as needed
}

void Browser::saveSettings()
{
    QSettings settings("YourCompany", "CustomBrowser");

    // Save general settings
    settings.setValue("general/startup_url", m_startupUrl);
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());

    // Save privacy settings
    settings.setValue("privacy/private_mode", m_isPrivateBrowsing);

    // Save customization settings
    settings.setValue("customization/theme", m_customizationEngine->currentTheme());

    // Save bookmarks
    settings.beginWriteArray("bookmarks");
    for (int i = 0; i < m_bookmarksModel->rowCount(); ++i) {
        settings.setArrayIndex(i);
        QStandardItem *item = m_bookmarksModel->item(i);
        settings.setValue("title", item->text());
        settings.setValue("url", item->data(Qt::UserRole).toString());
    }
    settings.endArray();

    // Save other settings as needed
}

void Browser::updateWindowTitle()
{
    QString title = currentWebView() ? currentWebView()->title() : QString();
    if (title.isEmpty()) {
        title = currentWebView() ? currentWebView()->url().toString() : tr("New Tab");
    }
    setWindowTitle(title + " - Custom Browser" + (m_isPrivateBrowsing ? " (Private)" : ""));
}

void Browser::updateNavigationActions()
{
    if (currentWebView()) {
        m_backAction->setEnabled(currentWebView()->history()->canGoBack());
        m_forwardAction->setEnabled(currentWebView()->history()->canGoForward());
    } else {
        m_backAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
    }
}

QWebEngineView *Browser::currentWebView() const
{
    return qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget());
}

WebPage *Browser::currentPage() const
{
    QWebEngineView *view = currentWebView();
    return view ? qobject_cast<WebPage*>(view->page()) : nullptr;
}

// Additional helper methods

void Browser::addToHistory(const QUrl &url, const QString &title)
{
    QStandardItem *item = new QStandardItem(title.isEmpty() ? url.toString() : title);
    item->setData(url, Qt::UserRole);
    m_historyModel->insertRow(0, item);

    // Limit history size (e.g., to 1000 items)
    while (m_historyModel->rowCount() > 1000) {
        m_historyModel->removeRow(m_historyModel->rowCount() - 1);
    }
}

void Browser::clearCache()
{
    m_profile->clearHttpCache();
    QWebEngineProfile::defaultProfile()->clearHttpCache();
}

void Browser::clearCookies()
{
    m_profile->cookieStore()->deleteAllCookies();
    QWebEngineProfile::defaultProfile()->cookieStore()->deleteAllCookies();
}

void Browser::showFindBar()
{
    if (!m_findBar) {
        m_findBar = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(m_findBar);
        m_findLineEdit = new QLineEdit(m_findBar);
        QPushButton *nextButton = new QPushButton(tr("Next"), m_findBar);
        QPushButton *prevButton = new QPushButton(tr("Previous"), m_findBar);
        QPushButton *closeButton = new QPushButton(tr("Close"), m_findBar);

        layout->addWidget(m_findLineEdit);
        layout->addWidget(nextButton);
        layout->addWidget(prevButton);
        layout->addWidget(closeButton);

        connect(m_findLineEdit, &QLineEdit::textChanged, this, &Browser::handleFindTextChanged);
        connect(nextButton, &QPushButton::clicked, this, &Browser::handleFindNextClicked);
        connect(prevButton, &QPushButton::clicked, this, &Browser::handleFindPreviousClicked);
        connect(closeButton, &QPushButton::clicked, m_findBar, &QWidget::hide);
    }

    m_findBar->show();
    m_findLineEdit->setFocus();
}

void Browser::showDeveloperTools()
{
    if (currentWebView()) {
        m_developerTools->setWebView(currentWebView());
        m_developerToolsDock->show();
    }
}

void Browser::askAIAssistant()
{
    QString query = QInputDialog::getText(this, tr("AI Assistant"), tr("What would you like to ask?"));
    if (!query.isEmpty()) {
        m_aiAssistant->processQuery(query);
    }
}

void Browser::takeScreenshot()
{
    if (currentWebView()) {
        QPixmap screenshot = currentWebView()->grab();
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), QDir::homePath(), tr("Images (*.png *.jpg)"));
        if (!fileName.isEmpty()) {
            screenshot.save(fileName);
        }
    }
}

void Browser::translatePage()
{
    // Implement page translation functionality
    // This could involve using a translation API or service
}

void Browser::showAccessibilityOptions()
{
    // Implement accessibility options dialog
    // This could include options for text size, contrast, screen reader support, etc.
}

void Browser::toggleAdBlocker()
{
    bool enabled = m_privacyManager->toggleAdBlocker();
    // Update UI to reflect ad blocker status
}

void Browser::showExtensionsManager()
{
    // Implement extensions manager dialog
    // This would allow users to install, enable, disable, and remove browser extensions
}

void Browser::showSyncSettings()
{
    // Implement sync settings dialog
    // This would allow users to set up and manage browser data synchronization
}

void Browser::showPerformanceStats()
{
    // Implement performance statistics dialog
    // This could show memory usage, CPU usage, network statistics, etc.
}

// ... Add more methods as needed to implement additional features
