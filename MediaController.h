// MediaController.h

#ifndef MEDIACONTROLLER_H
#define MEDIACONTROLLER_H

#include <QObject>
#include <QWebEngineView>

class MediaController : public QObject
{
    Q_OBJECT

public:
    explicit MediaController(QWebEngineView *webView, QObject *parent = nullptr);

    void togglePictureInPicture();
    void setVolume(int volume);
    void playPause();
    void seekTo(qint64 position);
    void toggleMute();
    void setPlaybackRate(qreal rate);
    void enableAutoplay(bool enable);
    void setAudioOutput(const QString &deviceName);

    bool isPictureInPictureActive() const;
    int volume() const;
    bool isPlaying() const;
    qint64 currentPosition() const;
    bool isMuted() const;
    qreal playbackRate() const;
    bool isAutoplayEnabled() const;
    QString currentAudioOutput() const;

public slots:
    void handleMediaStatusChanged();

signals:
    void pictureInPictureChanged(bool active);
    void volumeChanged(int volume);
    void playbackStateChanged(bool playing);
    void positionChanged(qint64 position);
    void mutedChanged(bool muted);
    void playbackRateChanged(qreal rate);
    void autoplayChanged(bool enabled);
    void audioOutputChanged(const QString &deviceName);

private:
    QWebEngineView *m_webView;
    bool m_pictureInPictureActive;
    int m_volume;
    bool m_isPlaying;
    qint64 m_currentPosition;
    bool m_isMuted;
    qreal m_playbackRate;
    bool m_autoplayEnabled;
    QString m_currentAudioOutput;

    void injectMediaControlScript();
    void executeMediaCommand(const QString &command);
};

#endif // MEDIACONTROLLER_H