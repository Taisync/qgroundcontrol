/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

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
        if (enabled.toBool()) {
            setEnabled(true);
        } else {
            setEnabled(false);
        }
    });

    // if (_settings->ntripServerConnectEnabled()->rawValue().toBool()) {
    //     setEnabled(true);
    // }
    // setEnabled(false);
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
        _settings->ntripEnableVRS()->rawValue().toBool()
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

void NTRIP::_tcpError(const QString errorMsg) {
    qCWarning(NTRIPLog) << "NTRIP connection error:" << errorMsg;
}

bool NTRIP::enabled() const {
    return _tcpLink && _tcpLink->enabled();
}

void NTRIP::setEnabled(bool en) {
    if (en) {
        _initLink();
        if (_tcpLink && !_tcpLink->enabled()) {
            _tcpLink->setEnabled(true);
            emit enabledChanged();
        }
    } else {
        _stopLink();
    }
}

int NTRIP::connectionStatus() const {
    return _tcpLink ? static_cast<int>(_tcpLink->connectionStatus()) : 0;
}

bool NTRIP::masterEnable() const {
    return _settings && _settings->ntripServerConnectEnabled()->rawValue().toBool();
}

//=======================================================================
// NTRIPTCPLink Implementation (mostly unchanged except toolbox calls)

NTRIPTCPLink::NTRIPTCPLink(const QString &hostAddress, int port,
                           const QString &username, const QString &password,
                           const QString &mountpoint, const QString &whitelist,
                           const bool &enableVRS)
    : QThread()
    , _hostAddress(hostAddress)
    , _port(port)
    , _username(username)
    , _password(password)
    , _mountpoint(mountpoint)
    , _isVRSEnable(enableVRS)
{
    _reconnectTimer = new QTimer(this);
    _reconnectTimer->setSingleShot(true);
    connect(_reconnectTimer, &QTimer::timeout, this, &NTRIPTCPLink::_retryConnection);

    for (const auto &msg : whitelist.split(',')) {
        int msgInt = msg.toInt();
        if (msgInt) {
            _whitelist.insert(msgInt);
        }
    }

    _rtcm_parsing = new RTCMParsing();
    _rtcm_parsing->reset();
    _state = NTRIPState::uninitialised;

    moveToThread(this);
    start();
}

NTRIPTCPLink::~NTRIPTCPLink() {
    if (_socket) {
        if (_isVRSEnable && _vrsSendTimer) {
            _vrsSendTimer->stop();
            disconnect(_vrsSendTimer, &QTimer::timeout, this, &NTRIPTCPLink::_sendNmeaGga);
            delete _vrsSendTimer;
            _vrsSendTimer = nullptr;
        }

        disconnect(_socket, &QTcpSocket::readyRead, this, &NTRIPTCPLink::_readBytes);
        _socket->disconnectFromHost();
        _socket->deleteLater();
        _socket = nullptr;

        delete _rtcm_parsing;
        _rtcm_parsing = nullptr;
    }
    quit();
    wait();
}

void NTRIPTCPLink::run() {
    if (_isVRSEnable) {
        _vrsSendTimer = new QTimer();
        _vrsSendTimer->setInterval(_vrsSendRateMSecs);
        connect(_vrsSendTimer, &QTimer::timeout, this, &NTRIPTCPLink::_sendNmeaGga);
        _vrsSendTimer->start();
    }

    exec();
}

void NTRIPTCPLink::_hardwareConnect() {
    _socket = new QTcpSocket();
    connect(_socket, &QTcpSocket::readyRead, this, &NTRIPTCPLink::_readBytes);
    _socket->connectToHost(_hostAddress, static_cast<quint16>(_port));
    _setConnectionStatus(NTRIPStatus::Connecting);

    if (!_socket->waitForConnected(1000)) {
        emit error("NTRIP connection failed, retrying...");
        _reconnectTimer->start(2000);
        return;
    }

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
    _state = NTRIPState::waiting_for_http_response;
}

void NTRIPTCPLink::_parse(const QByteArray &buffer) {
    _setConnectionStatus(NTRIPStatus::Connected);
    _retryCount = 0;

    for (const uint8_t byte : buffer) {
        if (_state == NTRIPState::waiting_for_rtcm_header) {
            if (byte != RTCM3_PREAMBLE && byte != RTCM2_PREAMBLE) continue;
            _state = NTRIPState::accumulating_rtcm_packet;
        }

        if (_rtcm_parsing->addByte(byte)) {
            _state = NTRIPState::waiting_for_rtcm_header;
            QByteArray msg((char*)_rtcm_parsing->message(), _rtcm_parsing->messageLength());
            uint16_t id = _rtcm_parsing->messageId();
            uint8_t version = _rtcm_parsing->rtcmVersion();
            qCDebug(NTRIPLog) << "RTCM version " << version;
            qCDebug(NTRIPLog) << "RTCM message ID " << id;
            qCDebug(NTRIPLog) << "RTCM message size " << msg.size();

            if (version == 3 && (_whitelist.empty() || _whitelist.contains(id))) {
                emit RTCMDataUpdate(msg);
            }
            _rtcm_parsing->reset();
        }
    }
}

void NTRIPTCPLink::_readBytes() {
    if (!_socket) return;

    if (_state == NTRIPState::waiting_for_http_response) {
        QString line = _socket->readLine();
        if (line.contains("200")) {
            _state = NTRIPState::waiting_for_rtcm_header;
        } else if (line.contains("401")) {
            emit error("NTRIP Unauthorized (401)");
            _state = NTRIPState::uninitialised;
        }
    }

    if (_state == NTRIPState::uninitialised) {
        _socket->readAll();
        return;
    }

    QByteArray bytes = _socket->readAll();
    _parse(bytes);
}

void NTRIPTCPLink::_sendNmeaGga() {
    auto vehicleMgr = MultiVehicleManager::instance();
    if (!vehicleMgr->activeVehicle()) return;

    Vehicle* vehicle = vehicleMgr->activeVehicle();
    NmeaMessage nmea(vehicle->coordinate());
    if (_socket) {
        _socket->write(nmea.getGGA().toUtf8());
    }
}

void NTRIPTCPLink::_setConnectionStatus(NTRIPStatus newStatus) {
    if (_connectionStatus != newStatus) {
        _connectionStatus = newStatus;
        emit connectionStatusChanged();
    }
}

void NTRIPTCPLink::setEnabled(bool en) {
    if (_enabled == en) return;
    _enabled = en;
    emit enabledChanged();

    if (_enabled) _startNTRIP();
    else _stopNTRIP();
}

void NTRIPTCPLink::_startNTRIP() {
    _retryCount = 0;
    _setConnectionStatus(NTRIPStatus::Connecting);
    _hardwareConnect();
}

void NTRIPTCPLink::_stopNTRIP() {
    if (_socket) {
        _socket->close();
        delete _socket;
        _socket = nullptr;
    }
    if (_reconnectTimer) _reconnectTimer->stop();
    _setConnectionStatus(NTRIPStatus::Off);
}

void NTRIPTCPLink::_retryConnection() {
    if (_retryCount < _maxRetries && _enabled) {
        _retryCount++;
        _setConnectionStatus(NTRIPStatus::Connecting);
        _hardwareConnect();
    } else {
        _setConnectionStatus(NTRIPStatus::TimedOut);
    }
}
