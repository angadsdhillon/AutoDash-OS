#ifndef CAMERAMODULE_H
#define CAMERAMODULE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QProgressBar>
#include <QGroupBox>
#include <QFrame>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QApplication>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../system/Logger.h"

class CameraModule : public QWidget
{
    Q_OBJECT

public:
    explicit CameraModule(QWidget *parent = nullptr);
    ~CameraModule();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void startCamera();
    void stopCamera();
    void toggleReverseMode();
    void setBrightness(int brightness);
    void setContrast(int contrast);
    void setSaturation(int saturation);
    void toggleGridLines();
    void toggleDistanceLines();
    void toggleParkingLines();
    void captureImage();
    void recordVideo();
    void showCameraSettings();
    void onCameraFrameReceived();
    void onCameraError(const QString& error);
    void simulateCameraFailure();
    void resetCameraSettings();
    void updateCameraDisplay();

private:
    void setupUI();
    void setupCameraView();
    void setupControls();
    void setupOverlayControls();
    void setupRecordingControls();
    void setupImageProcessing();
    void updateOverlay();
    void processFrame(cv::Mat& frame);
    void drawGridLines(cv::Mat& frame);
    void drawDistanceLines(cv::Mat& frame);
    void drawParkingLines(cv::Mat& frame);
    void drawGuidelines(cv::Mat& frame);
    void showErrorMessage(const QString& message);
    void showInfoMessage(const QString& message);
    QPixmap matToPixmap(const cv::Mat& mat);
    cv::Mat pixmapToMat(const QPixmap& pixmap);
    void createDummyFrame();
    void updateCameraStatus();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlsLayout;
    QHBoxLayout *m_overlayLayout;
    
    // Camera view
    QGraphicsView *m_cameraView;
    QGraphicsScene *m_cameraScene;
    QGraphicsPixmapItem *m_cameraPixmapItem;
    
    // Camera controls
    QPushButton *m_startStopButton;
    QPushButton *m_reverseModeButton;
    QPushButton *m_captureButton;
    QPushButton *m_recordButton;
    QPushButton *m_settingsButton;
    QPushButton *m_simulateErrorButton;
    QPushButton *m_resetButton;
    
    // Image processing controls
    QSlider *m_brightnessSlider;
    QSlider *m_contrastSlider;
    QSlider *m_saturationSlider;
    QLabel *m_brightnessLabel;
    QLabel *m_contrastLabel;
    QLabel *m_saturationLabel;
    
    // Overlay controls
    QPushButton *m_gridLinesButton;
    QPushButton *m_distanceLinesButton;
    QPushButton *m_parkingLinesButton;
    QPushButton *m_guidelinesButton;
    
    // Status displays
    QLabel *m_cameraStatusLabel;
    QLabel *m_recordingStatusLabel;
    QProgressBar *m_recordingProgressBar;
    QLabel *m_resolutionLabel;
    QLabel *m_fpsLabel;
    
    // OpenCV components
    cv::VideoCapture *m_camera;
    cv::Mat m_currentFrame;
    cv::Mat m_processedFrame;
    
    // Timers
    QTimer *m_cameraTimer;
    QTimer *m_recordingTimer;
    
    // State
    bool m_cameraRunning;
    bool m_reverseModeEnabled;
    bool m_gridLinesEnabled;
    bool m_distanceLinesEnabled;
    bool m_parkingLinesEnabled;
    bool m_guidelinesEnabled;
    bool m_recordingEnabled;
    int m_brightness;
    int m_contrast;
    int m_saturation;
    int m_recordingDuration;
    QString m_cameraStatus;
    
    // Recording state
    cv::VideoWriter *m_videoWriter;
    QString m_recordingFileName;
    int m_recordingFrameCount;
    
    // Constants
    static const int CAMERA_UPDATE_INTERVAL = 33; // ~30 FPS
    static const int RECORDING_UPDATE_INTERVAL = 1000; // 1 second
    static const int DEFAULT_BRIGHTNESS = 50;
    static const int DEFAULT_CONTRAST = 50;
    static const int DEFAULT_SATURATION = 50;
    static const int MAX_RECORDING_DURATION = 60; // 60 seconds
    static const int CAMERA_WIDTH = 640;
    static const int CAMERA_HEIGHT = 480;
};

#endif // CAMERAMODULE_H 