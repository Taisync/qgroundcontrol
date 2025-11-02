/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QThread>
#include <QTcpSocket>
#include <QGeoCoordinate>
#include <QUrl>
#include <QTimer>
#include <QSet>

#include "rtcm.h"
#include "RTCMMavlink.h"
#include "NmeaMessage.h"

Q_DECLARE_LOGGING_CATEGORY(NTRIPLog)

class NTRIPSettings;

class NTRIPTCPLink : public QThread {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(NTRIPStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)

public:
    enum class NTRIPStatus {
        Off,
        Connecting,
        Connected,
        Retrying,
        TimedOut
    };
    Q_ENUM(NTRIPStatus)

    NTRIPTCPLink(const QString& hostAddress,
                 int port,
                 const QString& username,
                 const QString& password,
                 const QString& mountpoint,
                 const QString& whitelist,
                 const bool& enableVRS);
    ~NTRIPTCPLink();

    bool enabled() const { return _enabled; }
    void setEnabled(bool en);
    NTRIPStatus connectionStatus() const { return _connectionStatus; }

signals:
    void error(const QString errorMsg);
    void RTCMDataUpdate(QByteArray message);
    void connectionStatusChanged();
    void enabledChanged();

protected:
    void run() final;

private slots:
    void _readBytes();

private:
    enum class NTRIPState {
        uninitialised,
        waiting_for_http_response,
        waiting_for_rtcm_header,
        accumulating_rtcm_packet,
    };

    void _hardwareConnect();
    void _parse(const QByteArray &buffer);
    void _setConnectionStatus(NTRIPStatus newStatus);
    void _startNTRIP();
    void _stopNTRIP();
    void _retryConnection();
    void _sendNmeaGga();

    QTcpSocket*     _socket = nullptr;
    QString         _hostAddress;
    int             _port;
    QString         _username;
    QString         _password;
    QString         _mountpoint;
    QSet<int>       _whitelist;
    bool            _isVRSEnable = false;
    int             _vrsSendRateMSecs = 3000;
    bool            _ntripForceV1 = false;

    QTimer*         _reconnectTimer = nullptr;
    QTimer*         _vrsSendTimer = nullptr;
    int             _retryCount = 0;
    const int       _maxRetries = 5;
    bool            _enabled = false;

    NTRIPStatus     _connectionStatus = NTRIPStatus::Off;
    RTCMParsing*    _rtcm_parsing = nullptr;
    NTRIPState      _state;
};

//---------------------------------------------------------------

class NTRIP : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool masterEnable READ masterEnable NOTIFY masterEnableChanged)

public:
    explicit NTRIP(NTRIPSettings* settings, QObject* parent = nullptr);
    ~NTRIP();

    static NTRIP* instance();

    bool enabled() const;
    void setEnabled(bool en);
    int connectionStatus() const;
    bool masterEnable() const;

signals:
    void enabledChanged();
    void connectionStatusChanged();
    void masterEnableChanged();

private slots:
    void _tcpError(const QString errorMsg);

private:
    void _initLink();
    void _stopLink();

    NTRIPSettings*  _settings = nullptr;
    NTRIPTCPLink*   _tcpLink = nullptr;
    RTCMMavlink*    _rtcmMavlink = nullptr;
};
