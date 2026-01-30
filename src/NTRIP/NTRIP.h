#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QSet>

#include "rtcm.h"
#include "RTCMMavlink.h"
#include "NmeaMessage.h"

Q_DECLARE_LOGGING_CATEGORY(NTRIPLog)

class NTRIPSettings;

class NTRIPTCPLink : public QObject {
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

    explicit NTRIPTCPLink(const QString& hostAddress,
                          int port,
                          const QString& username,
                          const QString& password,
                          const QString& mountpoint,
                          const QString& whitelist,
                          const bool& enableVRS,
                          QObject* parent = nullptr);
    ~NTRIPTCPLink();

    bool enabled() const { return _enabled; }
    void setEnabled(bool en);
    NTRIPStatus connectionStatus() const { return _connectionStatus; }

   signals:
    void error(const QString& errorMsg);
    void RTCMDataUpdate(QByteArray message);
    void connectionStatusChanged();
    void enabledChanged();

   private slots:
    void _socketConnected();
    void _socketDisconnected();
    void _socketError(QAbstractSocket::SocketError err);
    void _readBytes();
    void _retryConnection();
    void _sendNmeaGga();

   private:
    void _initSocket();
    void _setConnectionStatus(NTRIPStatus newStatus);

    QString _hostAddress;
    int _port;
    QString _username;
    QString _password;
    QString _mountpoint;
    QSet<int> _whitelist;
    bool _isVRSEnable = false;
    int _vrsSendRateMSecs = 3000;

    int _rapidRetryCount = 0;
    const int _maxRapidRetries = 10;           // Max consecutive rapid failures before timeout
    const int _reconnectDelayMS = 2000;
    const int _stableConnectionThresholdMS = 5000;  // Connection must be stable this long to reset retry count
    qint64 _lastConnectedTimestamp = 0;

    QTcpSocket* _socket = nullptr;
    RTCMParsing* _rtcm_parsing = nullptr;
    QTimer* _reconnectTimer = nullptr;
    QTimer* _vrsSendTimer = nullptr;

    bool _enabled = false;
    NTRIPStatus _connectionStatus = NTRIPStatus::Off;

    QTimer* _heartbeatTimer = nullptr;
    const int _heartbeatTimeoutMS = 5000; // 5 seconds, adjust as needed

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
    void _tcpError(const QString& errorMsg);

   private:
    void _initLink();
    void _stopLink();

    NTRIPSettings* _settings = nullptr;
    NTRIPTCPLink* _tcpLink = nullptr;
    RTCMMavlink* _rtcmMavlink = nullptr;
};
