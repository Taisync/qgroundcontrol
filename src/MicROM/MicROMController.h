/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

/// @file MicROMController.h
/// @brief Controller for OFIL MicROM UV camera payload via UDP/IP

#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QLoggingCategory>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>

Q_DECLARE_LOGGING_CATEGORY(MicROMControllerLog)

class MicROMController : public QObject
{
    Q_OBJECT

public:
    explicit MicROMController(QObject* parent = nullptr);
    ~MicROMController();

    Q_PROPERTY(bool     connected       READ connected      NOTIFY connectedChanged)
    Q_PROPERTY(bool     recording       READ recording      NOTIFY recordingChanged)
    Q_PROPERTY(int      zoom            READ zoom           NOTIFY zoomChanged)
    Q_PROPERTY(int      gain            READ gain           NOTIFY gainChanged)
    Q_PROPERTY(int      uvColor         READ uvColor        NOTIFY uvColorChanged)
    Q_PROPERTY(QString  cameraIP        READ cameraIP       WRITE setCameraIP   NOTIFY cameraIPChanged)
    Q_PROPERTY(QString  lastError       READ lastError      NOTIFY lastErrorChanged)
    Q_PROPERTY(bool     sdCardPresent   READ sdCardPresent  NOTIFY sdCardPresentChanged)

    bool    connected() const       { return _connected; }
    bool    recording() const       { return _recording; }
    int     zoom() const            { return _zoom; }
    int     gain() const            { return _gain; }
    int     uvColor() const         { return _uvColor; }
    QString cameraIP() const        { return _cameraIP; }
    QString lastError() const       { return _lastError; }
    bool    sdCardPresent() const   { return _sdCardPresent; }

    void setCameraIP(const QString& ip);

    // Commands callable from QML
    Q_INVOKABLE void takePhoto();
    Q_INVOKABLE void startVideo();
    Q_INVOKABLE void stopVideo();
    Q_INVOKABLE void setZoom(int value);
    Q_INVOKABLE void setGain(int value);
    Q_INVOKABLE void setUVColor(int value);
    Q_INVOKABLE void queryStatus();

signals:
    void connectedChanged();
    void recordingChanged();
    void zoomChanged();
    void gainChanged();
    void uvColorChanged();
    void cameraIPChanged();
    void lastErrorChanged();
    void sdCardPresentChanged();
    void photoTaken();
    void photoError();
    void videoError();

private slots:
    void _readPendingDatagrams();
    void _sendKeepalive();

private:
    void _sendCommand(const QString& command);
    void _parseResponse(const QByteArray& data);
    void _setRecording(bool recording);
    void _setLastError(const QString& error);

    QUdpSocket* _sendSocket          = nullptr;
    QUdpSocket* _recvSocket          = nullptr;
    QTimer*     _keepaliveTimer      = nullptr;

    QString     _cameraIP            = "192.168.144.2";
    quint16     _sendPort            = 4526;
    quint16     _recvPort            = 4527;

    bool        _connected           = false;
    bool        _recording           = false;
    bool        _sdCardPresent       = true;   // Assume present until we know otherwise
    int         _zoom                = 0;
    int         _gain                = 130;    // Default gain per OFIL docs
    int         _uvColor             = 0;      // UV color palette (0-7)
    int         _keepaliveMisses     = 0;
    bool        _pendingVideoStart   = false;  // Track if we're waiting for video start confirmation
    bool        _pendingVideoStop    = false;  // Track if we're waiting for video stop confirmation
    QString     _lastError;

    static const int KEEPALIVE_INTERVAL_MS   = 5000;
    static const int MAX_KEEPALIVE_MISSES    = 3;
};
