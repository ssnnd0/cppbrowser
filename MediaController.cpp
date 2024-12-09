// MediaController.cpp

#include "MediaController.h"
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

MediaController::MediaController(QWebEngineView *webView, QObject *parent)
    : QObject(parent)
    , m_webView(webView)
    , m_pictureInPictureActive(false)
    , m_volume(100)
    , m_isPlaying(false)
    , m_currentPosition(0)
    , m_isMuted(false)
    , m_playbackRate(1.0)
    , m_autoplayEnabled(true)
    , m_currentAudioOutput("Default")
{
    injectMediaControlScript();
    connect(m_webView->page(), &QWebEnginePage::loadFinished, this, &MediaController::handleMediaStatusChanged);
}

void MediaController::togglePictureInPicture()
{
    executeMediaCommand("togglePictureInPicture");
}

void MediaController::setVolume(int volume)
{
    executeMediaCommand(QString("setVolume(%1)").arg(volume));
}

void MediaController::playPause()
{
    executeMediaCommand("playPause");
}

void MediaController::seekTo(qint64 position)
{
    executeMediaCommand(QString("seekTo(%1)").arg(position));
}

void MediaController::toggleMute()
{
    executeMediaCommand("toggleMute");
}

void MediaController::setPlaybackRate(qreal rate)
{
    executeMediaCommand(QString("setPlaybackRate(%1)").arg(rate));
}

void MediaController::enableAutoplay(bool enable)
{
    m_autoplayEnabled = enable;
    m_webView->settings()->setAttribute(QWebEngineSettings::AutoplayEnabled, enable);
    emit autoplayChanged(enable);
}

void MediaController::setAudioOutput(const QString &deviceName)
{
    // In a real implementation, this would involve setting the audio output device
    m_currentAudioOutput = deviceName;
    emit audioOutputChanged(deviceName);
}

bool MediaController::isPictureInPictureActive() const
{
    return m_pictureInPictureActive;
}

int MediaController::volume() const
{
    return m_volume;
}

bool MediaController::isPlaying() const
{
    return m_isPlaying;
}

qint64 MediaController::currentPosition() const
{
    return m_currentPosition;
}

bool MediaController::isMuted() const
{
    return m_isMuted;
}

qreal MediaController::playbackRate() const
{
    return m_playbackRate;
}

bool MediaController::isAutoplayEnabled() const
{
    return m_autoplayEnabled;
}

QString MediaController::currentAudioOutput() const
{
    return m_currentAudioOutput;
}

void MediaController::handleMediaStatusChanged()
{
    executeMediaCommand("getMediaStatus");
}

void MediaController::injectMediaControlScript()
{
    QString scriptSource = R"(
        function getMediaElements() {
            return document.querySelectorAll('video, audio');
        }

        function executeOnMedia(callback) {
            const mediaElements = getMediaElements();
            for (let media of mediaElements) {
                callback(media);
            }
        }

        window.togglePictureInPicture = function() {
            executeOnMedia(media => {
                if (document.pictureInPictureElement) {
                    document.exitPictureInPicture();
                } else if (document.pictureInPictureEnabled) {
                    media.requestPictureInPicture();
                }
            });
        };

        window.setVolume = function(volume) {
            executeOnMedia(media => {
                media.volume = volume / 100;
            });
        };

        window.playPause = function() {
            executeOnMedia(media => {
                if (media.paused) {
                    media.play();
                } else {
                    media.pause();
                }
            });
        };

        window.seekTo = function(position) {
            executeOnMedia(media => {
                media.currentTime = position;
            });
        };

        window.toggleMute = function() {
            executeOnMedia(media => {
                media.muted = !media.muted;
            });
        };

        window.setPlaybackRate = function(rate) {
            executeOnMedia(media => {
                media.playbackRate = rate;
            });
        };

        window.getMediaStatus = function() {
            const status = {
                pictureInPictureActive: !!document.pictureInPictureElement,
                volume: 0,
                isPlaying: false,
                currentPosition: 0,
                isMuted: false,
                playbackRate: 1.0
            };

            executeOnMedia(media => {
                status.volume = media.volume * 100;
                status.isPlaying = !media.paused;
                status.currentPosition = media.currentTime;
                status.isMuted = media.muted;
                status.playbackRate = media.playbackRate;
            });

            window.qt.mediaController.updateStatus(JSON.stringify(status));
        };
    )";

    QWebEngineScript script;
    script.setName("MediaControlScript");
    script.setSourceCode(scriptSource);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setRunsOnSubFrames(true);

    m_webView->page()->scripts().insert(script);
}

void MediaController::executeMediaCommand(const QString &command)
{
    m_webView->page()->runJavaScript(command);
}
