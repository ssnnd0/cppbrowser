// AccessibilityManager.h

#ifndef ACCESSIBILITYMANAGER_H
#define ACCESSIBILITYMANAGER_H

#include <QObject>
#include <QWebEngineView>
#include <QTextToSpeech>

class AccessibilityManager : public QObject
{
    Q_OBJECT

public:
    explicit AccessibilityManager(QWebEngineView *webView, QObject *parent = nullptr);
    ~AccessibilityManager();

    void toggleScreenReader(bool enable);
    void setFontSize(int size);
    void setHighContrastMode(bool enable);
    void setColorBlindMode(const QString &mode);
    void enableKeyboardNavigation(bool enable);
    void setTextToSpeechRate(double rate);
    void setTextToSpeechPitch(double pitch);
    void setTextToSpeechVolume(double volume);

public slots:
    void speakSelectedText();
    void stopSpeaking();
    void navigateNext();
    void navigatePrevious();
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void screenReaderToggled(bool enabled);
    void fontSizeChanged(int size);
    void highContrastModeChanged(bool enabled);
    void colorBlindModeChanged(const QString &mode);
    void keyboardNavigationChanged(bool enabled);

private:
    QWebEngineView *m_webView;
    QTextToSpeech *m_tts;
    bool m_screenReaderEnabled;
    bool m_highContrastMode;
    QString m_colorBlindMode;
    bool m_keyboardNavigationEnabled;

    void injectAccessibilityScript();
    void updateAccessibilitySettings();
};

#endif // ACCESSIBILITYMANAGER_H