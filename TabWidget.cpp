// TabWidget.cpp

#include "TabWidget.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QAction>
#include <QMenu>
#include <QTabBar>

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_profile(QWebEngineProfile::defaultProfile())
{
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(true);

    m_backAction = new QAction(this);
    m_backAction->setIcon(QIcon::fromTheme("go-previous"));
    m_backAction->setToolTip(tr("Go back"));
    connect(m_backAction, &QAction::triggered, this, [this]() {
        if (currentWebView())
            currentWebView()->back();
    });

    m_forwardAction = new QAction(this);
    m_forwardAction->setIcon(QIcon::fromTheme("go-next"));
    m_forwardAction->setToolTip(tr("Go forward"));
    connect(m_forwardAction, &QAction::triggered, this, [this]() {
        if (currentWebView())
            currentWebView()->forward();
    });

    m_reloadAction = new QAction(this);
    m_reloadAction->setIcon(QIcon::fromTheme("view-refresh"));
    m_reloadAction->setToolTip(tr("Reload page"));
    connect(m_reloadAction, &QAction::triggered, this, [this]() {
        if (currentWebView())
            currentWebView()->reload();
    });

    m_stopAction = new QAction(this);
    m_stopAction->setIcon(QIcon::fromTheme("process-stop"));
    m_stopAction->setToolTip(tr("Stop loading"));
    connect(m_stopAction, &QAction::triggered, this, [this]() {
        if (currentWebView())
            currentWebView()->stop();
    });

    connect(this, &QTabWidget::currentChanged, this, &TabWidget::handleCurrentChanged);
    connect(this, &QTabWidget::tabCloseRequested, this, &TabWidget::closeTab);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        int index = tabBar()->tabAt(pos);
        if (index != -1) {
            QMenu menu;
            menu.addAction(tr("New Tab"), this, &TabWidget::addTab);
            menu.addAction(tr("Close Tab"), this, [this, index]() { closeTab(index); });
            menu.addSeparator();
            menu.addAction(tr("Reload Tab"), this, [this, index]() { reloadTab(index); });
            menu.exec(tabBar()->mapToGlobal(pos));
        }
    });
}

QWebEngineView *TabWidget::currentWebView() const
{
    return qobject_cast<QWebEngineView*>(currentWidget());
}

void TabWidget::setUrl(const QUrl &url)
{
    if (currentWebView())
        currentWebView()->setUrl(url);
    else
        addTab(url);
}

void TabWidget::setWebProfile(QWebEngineProfile *profile)
{
    m_profile = profile;
}

void TabWidget::addTab(const QUrl &url)
{
    QWebEngineView *webView = createWebView();
    int index = QTabWidget::addTab(webView, tr("New Tab"));
    setCurrentIndex(index);
    if (url.isValid())
        webView->setUrl(url);
}

void TabWidget::closeTab(int index)
{
    if (count() > 1) {
        QWidget *widget = this->widget(index);
        removeTab(index);
        widget->deleteLater();
    }
}

void TabWidget::reloadTab(int index)
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(widget(index)))
        view->reload();
}

void TabWidget::stopTab(int index)
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(widget(index)))
        view->stop();
}

qreal TabWidget::zoomFactor() const
{
    if (currentWebView())
        return currentWebView()->zoomFactor();
    return 1.0;
}

void TabWidget::setZoomFactor(qreal factor)
{
    if (currentWebView())
        currentWebView()->setZoomFactor(factor);
}

QByteArray TabWidget::saveState() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << count();
    for (int i = 0; i < count(); ++i) {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(widget(i)))
            stream << view->url();
    }

    return data;
}

bool TabWidget::restoreState(const QByteArray &state)
{
    QDataStream stream(state);

    int tabCount;
    stream >> tabCount;

    for (int i = 0; i < tabCount; ++i) {
        QUrl url;
        stream >> url;
        addTab(url);
    }

    return true;
}

void TabWidget::handleCurrentChanged(int index)
{
    if (index != -1) {
        QWebEngineView *view = qobject_cast<QWebEngineView*>(widget(index));
        if (view) {
            emit urlChanged(view->url());
            emit loadProgress(view->loadProgress());
            emit loadFinished(true);  // Assume it's finished if we're switching to it
        }
    }
}

void TabWidget::handleTabUrlChanged(const QUrl &url)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    int index = indexOf(view);
    if (index != -1) {
        setTabToolTip(index, url.toString());
        if (index == currentIndex())
            emit urlChanged(url);
    }
}

void TabWidget::handleTabLoadProgress(int progress)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    int index = indexOf(view);
    if (index == currentIndex())
        emit loadProgress(progress);
}

void TabWidget::handleTabLoadFinished(bool ok)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    int index = indexOf(view);
    if (index == currentIndex())
        emit loadFinished(ok);
}

void TabWidget::handleTabTitleChanged(const QString &title)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    int index = indexOf(view);
    if (index != -1)
        setTabText(index, title);
}

void TabWidget::handleTabIconChanged(const QIcon &icon)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    int index = indexOf(view);
    if (index != -1)
        setTabIcon(index, icon);
}

QWebEngineView *TabWidget::createWebView()
{
    QWebEngineView *webView = new QWebEngineView(this);
    QWebEnginePage *page = new QWebEnginePage(m_profile, webView);
    webView->setPage(page);

    connect(webView, &QWebEngineView::urlChanged, this, &TabWidget::handleTabUrlChanged);
    connect(webView, &QWebEngineView::loadProgress, this, &TabWidget::handleTabLoadProgress);
    connect(webView, &QWebEngineView::loadFinished, this, &TabWidget::handleTabLoadFinished);
    connect(webView, &QWebEngineView::titleChanged, this, &TabWidget::handleTabTitleChanged);
    connect(webView, &QWebEngineView::iconChanged, this, &TabWidget::handleTabIconChanged);

    return webView;
}