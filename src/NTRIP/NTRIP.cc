#include "NTRIP.h"
#include "NTRIPSettings.h"
#include "SettingsManager.h"
#include "MultiVehicleManager.h"
#include "QGCLoggingCategory.h"
#include "QGCApplication.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(NTRIPLog, "qgc.ntrip")

Q_APPLICATION_STATIC(NTRIP, _ntrip, SettingsManager::instance()->ntripSettings());

NTRIP* NTRIP::instance() { return _ntrip(); }

NTRIP::NTRIP(NTRIPSettings* settings, QObject* parent)
    : QObject(parent)
      , _settings(settings)
{
    connect(_settings->ntripServerConnectEnabled(), &Fact::rawValueChanged, this, [this](QVariant enabled) {
        if (!enabled.toBool() && _tcpLink) {
            _stopLink();
            emit enabledChanged();
        }
        emit masterEnableChanged();
    });
}


NTRIP::~NTRIP() {
    _stopLink();
}

void NTRIP::_initLink() {
    if (_tcpLink || _rtcmMavlink)
        return;

    _rtcmMavlink = new RTCMMavlink();

    _tcpLink = new NTRIPTCPLink(
        _settings->ntripServerHostAddress()->rawValue().toString(),
        _settings->ntripServerPort()->rawValue().toInt(),
        _settings->ntripUsername()->rawValue().toString(),
        _settings->ntripPassword()->rawValue().toString(),
        _settings->ntripMountpoint()->rawValue().toString(),
        _settings->ntripWhitelist()->rawValue().toString(),
        _settings->ntripEnableVRS()->rawValue().toBool(),
        this
        );

    connect(_tcpLink, &NTRIPTCPLink::error, this, &NTRIP::_tcpError, Qt::QueuedConnection);
    connect(_tcpLink, &NTRIPTCPLink::RTCMDataUpdate, _rtcmMavlink, &RTCMMavlink::RTCMDataUpdate);
    connect(_tcpLink, &NTRIPTCPLink::enabledChanged, this, &NTRIP::enabledChanged);
    connect(_tcpLink, &NTRIPTCPLink::connectionStatusChanged, this, &NTRIP::connectionStatusChanged);
}

void NTRIP::_stopLink() {
    if (_tcpLink) {
        _tcpLink->setEnabled(false);
        delete _tcpLink;
        _tcpLink = nullptr;
    }
    if (_rtcmMavlink) {
        delete _rtcmMavlink;
        _rtcmMavlink = nullptr;
    }
}

void NTRIP::_tcpError(const QString& errorMsg) {
    qCWarning(NTRIPLog) << "NTRIP connection error:" << errorMsg;
}

bool NTRIP::enabled() const { return _tcpLink && _tcpLink->enabled(); }
void NTRIP::setEnabled(bool en) {
    if (en) {
        _initLink();
        if (_tcpLink && !_tcpLink->enabled())
            _tcpLink->setEnabled(true);
    } else {
        _stopLink();
    }
}
int NTRIP::connectionStatus() const { return _tcpLink ? static_cast<int>(_tcpLink->connectionStatus()) : 0; }
bool NTRIP::masterEnable() const { return _settings && _settings->ntripServerConnectEnabled()->rawValue().toBool(); }

// =====================================================================
// NTRIPTCPLink Implementation

NTRIPTCPLink::NTRIPTCPLink(const QString &hostAddress, int port,
                           const QString &username, const QString &password,
                           const QString &mountpoint, const QString &whitelist,
                           const bool &enableVRS, QObject* parent)
    : QObject(parent)
      , _hostAddress(hostAddress)
      , _port(port)
      , _username(username)
      , _password(password)
      , _mountpoint(mountpoint)
      , _isVRSEnable(enableVRS)
{
    _rtcm_parsing = new RTCMParsing();
    _rtcm_parsing->reset();

    for (const auto &msg : whitelist.split(',')) {
        bool ok;
        int msgInt = msg.toInt(&ok);
        if (ok) _whitelist.insert(msgInt);
    }

    _reconnectTimer = new QTimer(this);
    _reconnectTimer->setSingleShot(true);
    connect(_reconnectTimer, &QTimer::timeout, this, &NTRIPTCPLink::_retryConnection);

    if (_isVRSEnable) {
        _vrsSendTimer = new QTimer(this);
        _vrsSendTimer->setInterval(_vrsSendRateMSecs);
        connect(_vrsSendTimer, &QTimer::timeout, this, &NTRIPTCPLink::_sendNmeaGga);
    }

    _heartbeatTimer = new QTimer(this);
    _heartbeatTimer->setSingleShot(true);
    connect(_heartbeatTimer, &QTimer::timeout, this, [this]() {
        if (!_enabled) return;
        emit error("NTRIP: connection lost (no data received)");
        _setConnectionStatus(NTRIPStatus::Retrying);

        if (_socket) {
            _socket->abort();  // force close
            _socket->deleteLater();
            _socket = nullptr;
        }

        _reconnectTimer->start(_reconnectDelayMS);
    });
}

NTRIPTCPLink::~NTRIPTCPLink() {
    setEnabled(false);
    delete _rtcm_parsing;
}

// -------------------- Enable / Disable -------------------

void NTRIPTCPLink::setEnabled(bool en) {
    if (_enabled == en) return;
    _enabled = en;
    emit enabledChanged();

    if (_enabled) {
        _retryCount = 0;
        _initSocket();
    } else {
        if (_socket) {
            _socket->disconnectFromHost();
            _socket->deleteLater();
            _socket = nullptr;
        }
        if (_vrsSendTimer) _vrsSendTimer->stop();
        if (_reconnectTimer) _reconnectTimer->stop();
        _setConnectionStatus(NTRIPStatus::Off);
    }
}

// -------------------- Socket Handling -------------------

void NTRIPTCPLink::_initSocket() {
    if (!_enabled) return;

    _setConnectionStatus(NTRIPStatus::Connecting);

    _socket = new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, this, &NTRIPTCPLink::_socketConnected);
    connect(_socket, &QTcpSocket::disconnected, this, &NTRIPTCPLink::_socketDisconnected);
    connect(_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NTRIPTCPLink::_socketError);
    connect(_socket, &QTcpSocket::readyRead, this, &NTRIPTCPLink::_readBytes);

    _socket->connectToHost(_hostAddress, static_cast<quint16>(_port));
}

void NTRIPTCPLink::_socketConnected() {
    QString digest = QString(_username + ":" + _password).toUtf8().toBase64();
    QString auth = QString("Authorization: Basic %1\r\n").arg(digest);
    QString query = QString(
                        "GET /%1 HTTP/1.1\r\n"
                        "User-Agent: NTRIP QGroundControl\r\n"
                        "Ntrip-Version: Ntrip/2.0\r\n"
                        "%2"
                        "Connection: close\r\n\r\n"
                        ).arg(_mountpoint).arg(auth);
    _socket->write(query.toUtf8());
    _heartbeatTimer->start(_heartbeatTimeoutMS);
}

void NTRIPTCPLink::_socketDisconnected() {
    if (!_enabled) return;
    emit error("NTRIP socket disconnected");
    _setConnectionStatus(NTRIPStatus::Retrying);
    _reconnectTimer->start(_reconnectDelayMS);
}

void NTRIPTCPLink::_socketError(QAbstractSocket::SocketError) {
    if (!_enabled) return;
    emit error(_socket ? _socket->errorString() : "Unknown socket error");
    _setConnectionStatus(NTRIPStatus::Retrying);
    _reconnectTimer->start(_reconnectDelayMS);
}

// -------------------- Read / Parse -------------------

void NTRIPTCPLink::_readBytes() {
    if (!_socket) return;

            // Handle initial HTTP response
    if (_connectionStatus == NTRIPStatus::Connecting) {
        while (_socket->canReadLine()) {
            QString line = _socket->readLine().trimmed();
            if (line.isEmpty()) continue;

            if (line.startsWith("HTTP/1.1 200")) {
                _setConnectionStatus(NTRIPStatus::Connected);
                if (_isVRSEnable && _vrsSendTimer) _vrsSendTimer->start();
            } else if (line.startsWith("HTTP/1.1 401")) {
                emit error("NTRIP Unauthorized (401)");
                setEnabled(false);
                return;
            }
        }
        if (_connectionStatus != NTRIPStatus::Connected) return;
    }

            // Parse RTCM bytes
    QByteArray bytes = _socket->readAll();
    for (const uint8_t byte : bytes) {
        if (_rtcm_parsing->addByte(byte)) {
            QByteArray msg((char*)_rtcm_parsing->message(), _rtcm_parsing->messageLength());
            uint16_t id = _rtcm_parsing->messageId();
            uint8_t version = _rtcm_parsing->rtcmVersion();

            qCDebug(NTRIPLog) << "RTCM version " << version;
            qCDebug(NTRIPLog) << "RTCM message ID " << id;

            if (version == 3 && (_whitelist.empty() || _whitelist.contains(id))) {
                emit RTCMDataUpdate(msg);
            }
            _rtcm_parsing->reset();
        }
    }

    if (_heartbeatTimer)
        _heartbeatTimer->start(_heartbeatTimeoutMS); // restart timer on each successful read

}

// -------------------- Retry / GGA -------------------

void NTRIPTCPLink::_retryConnection() {
    if (!_enabled) return;

    if (_retryCount < _maxRetries) {
        _retryCount++;
        _initSocket();
    } else {
        _setConnectionStatus(NTRIPStatus::TimedOut);
    }
}

void NTRIPTCPLink::_sendNmeaGga() {
    auto vehicleMgr = MultiVehicleManager::instance();
    if (!vehicleMgr->activeVehicle()) return;
    Vehicle* vehicle = vehicleMgr->activeVehicle();
    NmeaMessage nmea(vehicle->coordinate());
    if (_socket) _socket->write(nmea.getGGA().toUtf8());
}

// -------------------- Status -------------------

void NTRIPTCPLink::_setConnectionStatus(NTRIPStatus newStatus) {
    if (_connectionStatus != newStatus) {
        _connectionStatus = newStatus;
        emit connectionStatusChanged();
    }
}
