#include "MediaPlayer.h"
#include <QStyle>
#include <QApplication>

MediaPlayer::MediaPlayer(QWidget *parent)
    : QWidget(parent)
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_currentVolume(DEFAULT_VOLUME)
    , m_isShuffleEnabled(false)
    , m_isRepeatEnabled(false)
    , m_currentPlaylistIndex(0)
    , m_usbMonitor(&USBMonitor::getInstance())
{
    setupUI();
    setupControls();
    setupPlaylist();
    setupUSBMonitoring();
    
    // Initialize media player
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(m_currentVolume / 100.0);
    
    // Connect signals
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayer::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &MediaPlayer::onPlayerStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &MediaPlayer::onErrorOccurred);
    
    // Setup timers
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &MediaPlayer::updateProgress);
    m_progressTimer->start(UPDATE_INTERVAL);
    
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MediaPlayer::updateTimeDisplay);
    m_updateTimer->start(1000);
    
    LOG_INFO("MediaPlayer", "Media player initialized");
}

MediaPlayer::~MediaPlayer()
{
    LOG_INFO("MediaPlayer", "Media player destroyed");
}

void MediaPlayer::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Now playing section
    QGroupBox *nowPlayingGroup = new QGroupBox("Now Playing", this);
    nowPlayingGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    QVBoxLayout *nowPlayingLayout = new QVBoxLayout(nowPlayingGroup);
    
    m_nowPlayingLabel = new QLabel("No track selected", this);
    m_nowPlayingLabel->setStyleSheet("font-size: 18px; color: white;");
    m_nowPlayingLabel->setAlignment(Qt::AlignCenter);
    
    m_artistLabel = new QLabel("Unknown Artist", this);
    m_artistLabel->setStyleSheet("font-size: 14px; color: #cccccc;");
    m_artistLabel->setAlignment(Qt::AlignCenter);
    
    m_albumLabel = new QLabel("Unknown Album", this);
    m_albumLabel->setStyleSheet("font-size: 12px; color: #999999;");
    m_albumLabel->setAlignment(Qt::AlignCenter);
    
    nowPlayingLayout->addWidget(m_nowPlayingLabel);
    nowPlayingLayout->addWidget(m_artistLabel);
    nowPlayingLayout->addWidget(m_albumLabel);
    
    m_mainLayout->addWidget(nowPlayingGroup);
    
    // Progress section
    QHBoxLayout *progressLayout = new QHBoxLayout();
    
    m_timeLabel = new QLabel("00:00", this);
    m_timeLabel->setStyleSheet("color: white; min-width: 50px;");
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setStyleSheet("QProgressBar { border: 1px solid #404040; background-color: #2d2d2d; }"
                                 "QProgressBar::chunk { background-color: #4CAF50; }");
    m_progressBar->setTextVisible(false);
    
    m_totalTimeLabel = new QLabel("00:00", this);
    m_totalTimeLabel->setStyleSheet("color: white; min-width: 50px;");
    
    progressLayout->addWidget(m_timeLabel);
    progressLayout->addWidget(m_progressBar, 1);
    progressLayout->addWidget(m_totalTimeLabel);
    
    m_mainLayout->addLayout(progressLayout);
    
    // Controls section
    setupControls();
    
    // Device info
    m_deviceInfoLabel = new QLabel("No USB device connected", this);
    m_deviceInfoLabel->setStyleSheet("color: #999999; font-size: 12px;");
    m_deviceInfoLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_deviceInfoLabel);
    
    // Playlist section
    setupPlaylist();
    
    setLayout(m_mainLayout);
}

void MediaPlayer::setupControls()
{
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    
    // Control buttons
    m_previousButton = new QPushButton(this);
    m_previousButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    m_previousButton->setToolTip("Previous");
    connect(m_previousButton, &QPushButton::clicked, this, &MediaPlayer::previous);
    
    m_playPauseButton = new QPushButton(this);
    m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playPauseButton->setToolTip("Play/Pause");
    connect(m_playPauseButton, &QPushButton::clicked, this, &MediaPlayer::playPause);
    
    m_stopButton = new QPushButton(this);
    m_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_stopButton->setToolTip("Stop");
    connect(m_stopButton, &QPushButton::clicked, this, &MediaPlayer::stop);
    
    m_nextButton = new QPushButton(this);
    m_nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_nextButton->setToolTip("Next");
    connect(m_nextButton, &QPushButton::clicked, this, &MediaPlayer::next);
    
    // Additional controls
    m_shuffleButton = new QPushButton("Shuffle", this);
    m_shuffleButton->setCheckable(true);
    connect(m_shuffleButton, &QPushButton::clicked, this, &MediaPlayer::toggleShuffle);
    
    m_repeatButton = new QPushButton("Repeat", this);
    m_repeatButton->setCheckable(true);
    connect(m_repeatButton, &QPushButton::clicked, this, &MediaPlayer::toggleRepeat);
    
    m_equalizerButton = new QPushButton("Equalizer", this);
    connect(m_equalizerButton, &QPushButton::clicked, this, &MediaPlayer::showEqualizer);
    
    m_playlistButton = new QPushButton("Playlist", this);
    connect(m_playlistButton, &QPushButton::clicked, this, &MediaPlayer::showPlaylistManager);
    
    // Volume control
    m_volumeLabel = new QLabel("Volume:", this);
    m_volumeLabel->setStyleSheet("color: white;");
    
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(m_currentVolume);
    m_volumeSlider->setStyleSheet("QSlider::groove:horizontal { border: 1px solid #404040; "
                                  "background: #2d2d2d; height: 8px; border-radius: 4px; }"
                                  "QSlider::handle:horizontal { background: #4CAF50; "
                                  "border: 1px solid #4CAF50; width: 18px; "
                                  "margin: -2px 0; border-radius: 9px; }");
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MediaPlayer::setVolume);
    
    // Add controls to layout
    controlsLayout->addWidget(m_previousButton);
    controlsLayout->addWidget(m_playPauseButton);
    controlsLayout->addWidget(m_stopButton);
    controlsLayout->addWidget(m_nextButton);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_shuffleButton);
    controlsLayout->addWidget(m_repeatButton);
    controlsLayout->addWidget(m_equalizerButton);
    controlsLayout->addWidget(m_playlistButton);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_volumeLabel);
    controlsLayout->addWidget(m_volumeSlider);
    controlsLayout->setStretchFactor(m_volumeSlider, 1);
    
    m_mainLayout->addLayout(controlsLayout);
}

void MediaPlayer::setupPlaylist()
{
    QGroupBox *playlistGroup = new QGroupBox("Playlist", this);
    playlistGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    QVBoxLayout *playlistLayout = new QVBoxLayout(playlistGroup);
    
    m_playlistWidget = new QListWidget(this);
    m_playlistWidget->setStyleSheet("QListWidget { background-color: #2d2d2d; color: white; "
                                    "border: 1px solid #404040; }"
                                    "QListWidget::item { padding: 8px; }"
                                    "QListWidget::item:selected { background-color: #404040; }"
                                    "QListWidget::item:hover { background-color: #353535; }");
    m_playlistWidget->setMaximumHeight(200);
    connect(m_playlistWidget, &QListWidget::itemDoubleClicked, 
            this, &MediaPlayer::onPlaylistItemDoubleClicked);
    
    playlistLayout->addWidget(m_playlistWidget);
    m_mainLayout->addWidget(playlistGroup);
}

void MediaPlayer::setupUSBMonitoring()
{
    connect(m_usbMonitor, &USBMonitor::deviceConnected, 
            this, &MediaPlayer::onUSBDeviceConnected);
    connect(m_usbMonitor, &USBMonitor::deviceDisconnected, 
            this, &MediaPlayer::onUSBDeviceDisconnected);
    connect(m_usbMonitor, &USBMonitor::mediaFilesChanged, 
            this, &MediaPlayer::onMediaFilesChanged);
}

void MediaPlayer::playPause()
{
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->pause();
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    } else {
        m_mediaPlayer->play();
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

void MediaPlayer::stop()
{
    m_mediaPlayer->stop();
    m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void MediaPlayer::next()
{
    if (m_playlist.isEmpty()) return;
    
    m_currentPlaylistIndex = (m_currentPlaylistIndex + 1) % m_playlist.size();
    loadCurrentTrack();
}

void MediaPlayer::previous()
{
    if (m_playlist.isEmpty()) return;
    
    m_currentPlaylistIndex = (m_currentPlaylistIndex - 1 + m_playlist.size()) % m_playlist.size();
    loadCurrentTrack();
}

void MediaPlayer::setVolume(int volume)
{
    m_currentVolume = volume;
    m_audioOutput->setVolume(volume / 100.0);
    updateVolumeDisplay();
}

void MediaPlayer::updateProgress()
{
    if (m_mediaPlayer->isAvailable()) {
        qint64 position = m_mediaPlayer->position();
        qint64 duration = m_mediaPlayer->duration();
        
        if (duration > 0) {
            m_progressBar->setRange(0, duration);
            m_progressBar->setValue(position);
        }
        
        m_timeLabel->setText(formatTime(position));
        m_totalTimeLabel->setText(formatTime(duration));
    }
}

void MediaPlayer::seekToPosition(int position)
{
    m_mediaPlayer->setPosition(position);
}

void MediaPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
        case QMediaPlayer::LoadedMedia:
            LOG_INFO("MediaPlayer", "Media loaded successfully");
            break;
        case QMediaPlayer::EndOfMedia:
            LOG_INFO("MediaPlayer", "Media playback ended");
            if (m_isRepeatEnabled) {
                m_mediaPlayer->setPosition(0);
                m_mediaPlayer->play();
            } else {
                next();
            }
            break;
        case QMediaPlayer::InvalidMedia:
            LOG_ERROR("MediaPlayer", "Invalid media file");
            showErrorMessage("Invalid media file");
            break;
        default:
            break;
    }
}

void MediaPlayer::onPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
        case QMediaPlayer::PlayingState:
            m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        case QMediaPlayer::PausedState:
        case QMediaPlayer::StoppedState:
            m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
    }
}

void MediaPlayer::onErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    LOG_ERROR("MediaPlayer", QString("Media player error: %1").arg(errorString));
    showErrorMessage("Media playback error: " + errorString);
}

void MediaPlayer::onUSBDeviceConnected(const USBDevice& device)
{
    m_currentDeviceId = device.deviceId;
    m_deviceInfoLabel->setText(QString("USB Device: %1 (%2 GB free)")
                               .arg(device.deviceName)
                               .arg(device.freeSpace / 1000000000.0, 0, 'f', 1));
    
    LOG_INFO("MediaPlayer", QString("USB device connected: %1").arg(device.deviceName));
    updatePlaylist();
}

void MediaPlayer::onUSBDeviceDisconnected(const QString& deviceId)
{
    if (deviceId == m_currentDeviceId) {
        m_currentDeviceId.clear();
        m_deviceInfoLabel->setText("No USB device connected");
        m_playlist.clear();
        m_playlistWidget->clear();
        m_mediaPlayer->stop();
        
        LOG_INFO("MediaPlayer", "USB device disconnected");
    }
}

void MediaPlayer::onMediaFilesChanged(const QString& deviceId, const QList<MediaFile>& files)
{
    if (deviceId == m_currentDeviceId) {
        updatePlaylist();
    }
}

void MediaPlayer::onPlaylistItemDoubleClicked(QListWidgetItem* item)
{
    int index = m_playlistWidget->row(item);
    if (index >= 0 && index < m_playlist.size()) {
        m_currentPlaylistIndex = index;
        loadCurrentTrack();
        m_mediaPlayer->play();
    }
}

void MediaPlayer::refreshPlaylist()
{
    updatePlaylist();
}

void MediaPlayer::toggleShuffle()
{
    m_isShuffleEnabled = m_shuffleButton->isChecked();
    LOG_INFO("MediaPlayer", QString("Shuffle %1").arg(m_isShuffleEnabled ? "enabled" : "disabled"));
}

void MediaPlayer::toggleRepeat()
{
    m_isRepeatEnabled = m_repeatButton->isChecked();
    LOG_INFO("MediaPlayer", QString("Repeat %1").arg(m_isRepeatEnabled ? "enabled" : "disabled"));
}

void MediaPlayer::showEqualizer()
{
    QMessageBox::information(this, "Equalizer", "Equalizer not implemented yet.");
}

void MediaPlayer::showPlaylistManager()
{
    QMessageBox::information(this, "Playlist Manager", "Playlist manager not implemented yet.");
}

void MediaPlayer::updatePlaylist()
{
    m_playlistWidget->clear();
    m_playlist.clear();
    
    if (m_currentDeviceId.isEmpty()) return;
    
    QList<MediaFile> files = m_usbMonitor->getMediaFiles(m_currentDeviceId);
    for (const MediaFile& file : files) {
        QString displayText = QString("%1 - %2 (%3)")
                             .arg(file.artist.isEmpty() ? "Unknown Artist" : file.artist)
                             .arg(file.title.isEmpty() ? file.fileName : file.title)
                             .arg(file.duration);
        
        QListWidgetItem* item = new QListWidgetItem(displayText, m_playlistWidget);
        item->setData(Qt::UserRole, file.filePath);
        m_playlist.append(file.filePath);
    }
    
    LOG_INFO("MediaPlayer", QString("Playlist updated with %1 tracks").arg(files.size()));
}

void MediaPlayer::updateNowPlaying()
{
    if (m_currentTrack.isEmpty()) {
        m_nowPlayingLabel->setText("No track selected");
        m_artistLabel->setText("Unknown Artist");
        m_albumLabel->setText("Unknown Album");
        return;
    }
    
    // Extract track info from current file
    QFileInfo fileInfo(m_currentTrack);
    QString fileName = fileInfo.baseName();
    
    m_nowPlayingLabel->setText(fileName);
    m_artistLabel->setText("Unknown Artist");
    m_albumLabel->setText("Unknown Album");
}

void MediaPlayer::updateTimeDisplay()
{
    updateProgress();
}

void MediaPlayer::loadCurrentTrack()
{
    if (m_currentPlaylistIndex >= 0 && m_currentPlaylistIndex < m_playlist.size()) {
        m_currentTrack = m_playlist[m_currentPlaylistIndex];
        m_mediaPlayer->setSource(QUrl::fromLocalFile(m_currentTrack));
        updateNowPlaying();
        
        LOG_INFO("MediaPlayer", QString("Loaded track: %1").arg(m_currentTrack));
    }
}

void MediaPlayer::updateVolumeDisplay()
{
    m_volumeLabel->setText(QString("Volume: %1%").arg(m_currentVolume));
}

void MediaPlayer::showErrorMessage(const QString& message)
{
    QMessageBox::warning(this, "Media Player Error", message);
}

QString MediaPlayer::formatTime(qint64 milliseconds)
{
    int seconds = milliseconds / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
} 