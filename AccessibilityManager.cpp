// AccessibilityManager.cpp

#include "AccessibilityManager.h"
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

AccessibilityManager::AccessibilityManager(QWebEngineView *webView, QObject *parent)
    : QObject(parent)
    , m_webView(webView)
    , m_tts(new QTextToSpeech(this))
    , m_screenReaderEnabled(false)
    , m_highContrastMode(false)
    , m_colorBlindMode("none")
    , m_keyboardNavigationEnabled(false)
{
    injectAccessibilityScript();
}

AccessibilityManager::~AccessibilityManager()
{
    // Cleanup if necessary
}

void AccessibilityManager::toggleScreenReader(bool enable)
{
    m_screenReaderEnabled = enable;
    updateAccessibilitySettings();
    emit screenReaderToggled(enable);
}

void AccessibilityManager::setFontSize(int size)
{
    m_webView->setZoomFactor(size / 100.0);
    emit fontSizeChanged(size);
}

void AccessibilityManager::setHighContrastMode(bool enable)
{
    m_highContrastMode = enable;
    updateAccessibilitySettings();
    emit highContrastModeChanged(enable);
}

void AccessibilityManager::setColorBlindMode(const QString &mode)
{
    m_colorBlindMode = mode;
    updateAccessibilitySettings();
    emit colorBlindModeChanged(mode);
}

void AccessibilityManager::enableKeyboardNavigation(bool enable)
{
    m_keyboardNavigationEnabled = enable;
    updateAccessibilitySettings();
    emit keyboardNavigationChanged(enable);
}

void AccessibilityManager::setTextToSpeechRate(double rate)
{
    m_tts->setRate(rate);
}

void AccessibilityManager::setTextToSpeechPitch(double pitch)
{
    m_tts->setPitch(pitch);
}

void AccessibilityManager::setTextToSpeechVolume(double volume)
{
    m_tts->setVolume(volume);
}

void AccessibilityManager::speakSelectedText()
{
    m_webView->page()->runJavaScript("window.getSelection().toString()", [this](const QVariant &result) {
        QString selectedText = result.toString();
        if (!selectedText.isEmpty()) {
            m_tts->say(selectedText);
        }
    });
}

void AccessibilityManager::stopSpeaking()
{
    m_tts->stop();
}

void AccessibilityManager::navigateNext()
{
    m_webView->page()->runJavaScript("window.accessibilityNavigateNext()");
}

void AccessibilityManager::navigatePrevious()
{
    m_webView->page()->runJavaScript("window.accessibilityNavigatePrevious()");
}

void AccessibilityManager::zoomIn()
{
    m_webView->setZoomFactor(m_webView->zoomFactor() * 1.1);
}

void AccessibilityManager::zoomOut()
{
    m_webView->setZoomFactor(m_webView->zoomFactor() / 1.1);
}

void AccessibilityManager::resetZoom()
{
    m_webView->setZoomFactor(1.0);
}

void AccessibilityManager::injectAccessibilityScript()
{
    QString scriptSource = R"(
        window.accessibilityNavigateNext = function() {
            // Implement navigation logic here
        };

        window.accessibilityNavigatePrevious = function() {
            // Implement navigation logic here
        };

        window.applyAccessibilitySettings = function(settings) {
            if (settings.highContrast) {
                document.body.style.backgroundColor = '#000000';
                document.body.style.color = '#FFFFFF';
            } else {
                document.body.style.backgroundColor = '';
                document.body.style.color = '';
            }

            if (settings.colorBlindMode !== 'none') {
                // Apply color filters based on the color blind mode
                // This would require more complex CSS or SVG filters
            }

            if (settings.keyboardNavigation) {
                // Enable keyboard navigation enhancements
            }
        };
    )";

    QWebEngineScript script;
    script.setName("AccessibilityScript");
    script.setSourceCode(scriptSource);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setRunsOnSubFrames(true);

    m_webView->page()->scripts().insert(script);
}

void AccessibilityManager::updateAccessibilitySettings()
{
    QString settingsJson = QString(R"(
    {
        "screenReader": %1,
        "highContrast": %2,
        "colorBlindMode": "%3",
        "keyboardNavigation": %4
    })").arg(m_screenReaderEnabled ? "true" : "false")
        .arg(m_highContrastMode ? "true" : "false")
        .arg(m_colorBlindMode)
        .arg(m_keyboardNavigationEnabled ? "true" : "false");

    m_webView->page()->runJavaScript(QString("window.applyAccessibilitySettings(%1)").arg(settingsJson));
}