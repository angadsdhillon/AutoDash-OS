#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QProgressBar>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QProcess>
#include <QUrl>
#include <QUrlQuery>

#include "../system/Logger.h"
#include "../system/USBMonitor.h"

class MediaPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPlayer(QWidget *parent = nullptr);
    ~MediaPlayer();

private slots:
    void playPause();
    void stop();
    void next();
    void previous();
    void setVolume(int volume);
    void updateProgress();
    void seekToPosition(int position);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlayerStateChanged(QMediaPlayer::PlaybackState state);
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);
    void onUSBDeviceConnected(const USBDevice& device);
    void onUSBDeviceDisconnected(const QString& deviceId);
    void onMediaFilesChanged(const QString& deviceId, const QList<MediaFile>& files);
    void onPlaylistItemDoubleClicked(QListWidgetItem* item);
    void refreshPlaylist();
    void toggleShuffle();
    void toggleRepeat();
    void showEqualizer();
    void showPlaylistManager();

private:
    void setupUI();
    void setupControls();
    void setupPlaylist();
    void setupUSBMonitoring();
    void updatePlaylist();
    void updateNowPlaying();
    void updateTimeDisplay();
    void loadPlaylist();
    void savePlaylist();
    void createDummyFiles();
    QString formatTime(qint64 milliseconds);
    void updateVolumeDisplay();
    void showErrorMessage(const QString& message);
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlsLayout;
    QVBoxLayout *m_playlistLayout;
    
    // Playback controls
    QPushButton *m_playPauseButton;
    QPushButton *m_stopButton;
    QPushButton *m_nextButton;
    QPushButton *m_previousButton;
    QPushButton *m_shuffleButton;
    QPushButton *m_repeatButton;
    QPushButton *m_equalizerButton;
    QPushButton *m_playlistButton;
    
    // Progress and volume
    QProgressBar *m_progressBar;
    QSlider *m_volumeSlider;
    QLabel *m_timeLabel;
    QLabel *m_totalTimeLabel;
    QLabel *m_volumeLabel;
    
    // Playlist
    QListWidget *m_playlistWidget;
    QLabel *m_nowPlayingLabel;
    QLabel *m_artistLabel;
    QLabel *m_albumLabel;
    QLabel *m_deviceInfoLabel;
    
    // Media player
    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;
    
    // Timers
    QTimer *m_progressTimer;
    QTimer *m_updateTimer;
    
    // State
    QString m_currentDeviceId;
    QString m_currentTrack;
    int m_currentVolume;
    bool m_isShuffleEnabled;
    bool m_isRepeatEnabled;
    QStringList m_playlist;
    int m_currentPlaylistIndex;
    
    // USB monitoring
    USBMonitor *m_usbMonitor;
    
    // Constants
    static const int UPDATE_INTERVAL = 100; // 100ms for smooth progress updates
    static const int DEFAULT_VOLUME = 50;
};

#endif // MEDIAPLAYER_H 