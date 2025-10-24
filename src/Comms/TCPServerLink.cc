/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "TCPServerLink.h"
#include "DeviceInfo.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>

QGC_LOGGING_CATEGORY(TCPServerLinkLog, "qgc.comms.tcpserverlink")

namespace {
    constexpr int CONNECT_TIMEOUT_MS = 1000;
    constexpr int TYPE_OF_SERVICE = 32; // Set ToS for low delay
}

//--------------------------------------------------------------------------
//-- TCPServerConfiguration
TCPServerConfiguration::TCPServerConfiguration(const QString& name, QObject *parent)
    : LinkConfiguration(name, parent)
{
    _port    = QGC_TCP_SERVER_PORT;
}

TCPServerConfiguration::TCPServerConfiguration(const TCPServerConfiguration* source, QObject *parent)
    : LinkConfiguration(source, parent)
{
    _port    = source->port();
}

void TCPServerConfiguration::copyFrom(const LinkConfiguration *source)
{
    Q_ASSERT(source);
    LinkConfiguration::copyFrom(source);

    const TCPServerConfiguration* const tcpServerSource = qobject_cast<const TCPServerConfiguration*>(source);
    Q_ASSERT(tcpServerSource);

    setPort(tcpServerSource->port());
}

void TCPServerConfiguration::setPort(quint16 port)
{
    _port = port;
}

/**
 * @param host Hostname in standard formatt, e.g. localhost:5671 or 192.168.1.1:5671
 */
void TCPServerConfiguration::setHost(const QString host)
{
    if (host.contains(":")) {
        setPort(host.split(":").last().toUInt());
    } else {
        // If no port, use default
        setPort(QGC_TCP_SERVER_PORT);
    }
}

void TCPServerConfiguration::saveSettings(QSettings& settings, const QString& root) const
{
    settings.beginGroup(root);

    settings.setValue("port", port());

    settings.endGroup();
}

void TCPServerConfiguration::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    _port = (quint16)settings.value("port", QGC_TCP_SERVER_PORT).toUInt();
    // _host = settings.value("host", _host).toString();
    settings.endGroup();
}

/*===========================================================================*/

TCPServerWorker::TCPServerWorker(const TCPServerConfiguration *config, QObject *parent)
    : QObject(parent)
    , _config(config)
{
    // qCDebug(TCPServerLinkLog) << Q_FUNC_INFO << this;
}

TCPServerWorker::~TCPServerWorker()
{
    disconnect();
}

bool TCPServerWorker::isConnected() const
{
    return (_socket && _socket->isOpen() && (_socket->state() == QAbstractSocket::ConnectedState));
}

void TCPServerWorker::setupSocketServer()
{
    Q_ASSERT(_socketServer == nullptr);
    _socketServer = new QTcpServer;
    connect(_socketServer,&QTcpServer::newConnection,this,&TCPServerWorker::newConnectSlot);
    _socketServer->listen(QHostAddress::Any, _config->port());
}

void TCPServerWorker::disconnect()
{
    qCDebug(TCPServerLinkLog) << "Attempting to disconnect:" << "port:" << _config->port();

    if(_socketServer) {
        _socketServer->disconnect();
        _socketServer->deleteLater();
        _socketServer = nullptr;
    }

    if (_socket) {
        // This prevents stale signal from calling the link after it has been deleted
        QObject::disconnect(_socket, &QTcpSocket::connected, this, &TCPServerWorker::_onSocketConnected);
        QObject::disconnect(_socket, &QTcpSocket::disconnected, this, &TCPServerWorker::_onSocketDisconnected);
        QObject::disconnect(_socket, &QTcpSocket::readyRead, this, &TCPServerWorker::_onSocketReadyRead);
        QObject::disconnect(_socket, &QTcpSocket::errorOccurred, this, &TCPServerWorker::_onSocketErrorOccurred);
        _socketIsConnected = false;
        _socket->disconnectFromHost();
        _socket->deleteLater();
        _socket = nullptr;
    }
    emit disconnected();
}

void TCPServerWorker::newConnectSlot()
{
    if(_socket) {
        qCInfo(TCPServerLinkLog) << "remove exist socket";
        // This prevents stale signal from calling the link after it has been deleted
        QObject::disconnect(_socket, &QTcpSocket::connected, this, &TCPServerWorker::_onSocketConnected);
        QObject::disconnect(_socket, &QTcpSocket::disconnected, this, &TCPServerWorker::_onSocketDisconnected);
        QObject::disconnect(_socket, &QTcpSocket::readyRead, this, &TCPServerWorker::_onSocketReadyRead);
        QObject::disconnect(_socket, &QTcpSocket::errorOccurred, this, &TCPServerWorker::_onSocketErrorOccurred);
        _socketIsConnected = false;
        _socket->disconnectFromHost();
        _socket->deleteLater();
        _socket = nullptr;
    }
    _socket = _socketServer->nextPendingConnection();
    if (!_socket) {
        qCCritical(TCPServerLinkLog) << "Can not find next pending connection";
        return;
    }
    qCInfo(TCPServerLinkLog) << "start new socket";
    _socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    _socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    _socket->setSocketOption(QAbstractSocket::TypeOfServiceOption, TYPE_OF_SERVICE);

    (void) connect(_socket, &QTcpSocket::connected, this, &TCPServerWorker::_onSocketConnected);
    (void) connect(_socket, &QTcpSocket::disconnected, this, &TCPServerWorker::_onSocketDisconnected);
    (void) connect(_socket, &QTcpSocket::readyRead, this, &TCPServerWorker::_onSocketReadyRead);
    (void) connect(_socket, &QTcpSocket::errorOccurred, this, &TCPServerWorker::_onSocketErrorOccurred);
}

void TCPServerWorker::writeData(const QByteArray &data)
{
    if (data.isEmpty()) {
        emit errorOccurred(tr("Data to Send is Empty"));
        return;
    }

    if (!isConnected()) {
        return;
    }

    qint64 totalBytesWritten = 0;
    while (totalBytesWritten < data.size()) {
        const qint64 bytesWritten = _socket->write(data.constData() + totalBytesWritten, data.size() - totalBytesWritten);
        if (bytesWritten == -1) {
            emit errorOccurred(tr("Could Not Send Data - Write Failed: %1").arg(_socket->errorString()));
            return;
        } else if (bytesWritten == 0) {
            emit errorOccurred(tr("Could Not Send Data - Write Returned 0 Bytes"));
            return;
        }
        totalBytesWritten += bytesWritten;
    }

    emit dataSent(data.first(totalBytesWritten));
}

void TCPServerWorker::_onSocketConnected()
{
    qCDebug(TCPServerLinkLog) << "Socket connected:" << _config->port();
    _errorEmitted = false;
    emit connected();
}

void TCPServerWorker::_onSocketDisconnected()
{
    qCDebug(TCPServerLinkLog) << "Socket disconnected:" << _config->port();
    _errorEmitted = false;
}

void TCPServerWorker::_onSocketReadyRead()
{
    if (_socket) {
        const QByteArray data = _socket->readAll();
        emit dataReceived(data);
    }
}

void TCPServerWorker::_onSocketBytesWritten(qint64 bytes)
{
    qCDebug(TCPServerLinkLog) << "Wrote" << bytes << "bytes";
}

void TCPServerWorker::_onSocketErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    const QString errorString = _socket->errorString();

    qCWarning(TCPServerLinkLog) << "Socket error:" << errorString;

    if (!_errorEmitted) {
        emit errorOccurred(errorString);
        _errorEmitted = true;
    }
}

/*===========================================================================*/

TCPServerLink::TCPServerLink(SharedLinkConfigurationPtr& config, QObject *parent)
    : LinkInterface(config, parent)
    , _tcpServerConfig(qobject_cast<TCPServerConfiguration*>(config.get()))
    , _worker(new TCPServerWorker(_tcpServerConfig))
    , _workerThread(new QThread(this))
{
    _workerThread->setObjectName(QStringLiteral("TCPSERVER_%1").arg(_tcpServerConfig->name()));

    _worker->moveToThread(_workerThread);

    (void) connect(_workerThread, &QThread::started, _worker, &TCPServerWorker::setupSocketServer);
    (void) connect(_workerThread, &QThread::finished, _worker, &QObject::deleteLater);

    (void) connect(_worker, &TCPServerWorker::connected, this, &TCPServerLink::_onConnected, Qt::QueuedConnection);
    (void) connect(_worker, &TCPServerWorker::disconnected, this, &TCPServerLink::_onDisconnected, Qt::QueuedConnection);
    (void) connect(_worker, &TCPServerWorker::errorOccurred, this, &TCPServerLink::_onErrorOccurred, Qt::QueuedConnection);
    (void) connect(_worker, &TCPServerWorker::dataReceived, this, &TCPServerLink::_onDataReceived, Qt::QueuedConnection);
    (void) connect(_worker, &TCPServerWorker::dataSent, this, &TCPServerLink::_onDataSent, Qt::QueuedConnection);

    _workerThread->start();
}

TCPServerLink::~TCPServerLink()
{
    disconnect();

    _workerThread->quit();
    if (!_workerThread->wait()) {
        qCWarning(TCPServerLinkLog) << "Failed to wait for TCP Thread to close";
    }
}

bool TCPServerLink::isConnected() const
{
    return _worker->isConnected();
}

bool TCPServerLink::_connect(void)
{
    return true;
}

void TCPServerLink::disconnect(void)
{
    (void) QMetaObject::invokeMethod(_worker, "disconnect", Qt::QueuedConnection);
}

void TCPServerLink::_onConnected()
{
    emit connected();
}

void TCPServerLink::_onDisconnected()
{
    emit disconnected();
}

void TCPServerLink::_onErrorOccurred(const QString &errorString)
{
    qCWarning(TCPServerLinkLog) << "Communication error:" << errorString;
    emit communicationError(tr("TCP Server Link Error"), tr("Link %1: (Port: %3) %4").arg(_tcpServerConfig->name()).arg(_tcpServerConfig->port()).arg(errorString));
}

void TCPServerLink::_onDataReceived(const QByteArray &data)
{
    emit bytesReceived(this, data);
}

void TCPServerLink::_onDataSent(const QByteArray &data)
{
    emit bytesSent(this, data);
}

void TCPServerLink::_writeBytes(const QByteArray& bytes)
{
    (void) QMetaObject::invokeMethod(_worker, "writeData", Qt::QueuedConnection, Q_ARG(QByteArray, bytes));
}

bool TCPServerLink::isSecureConnection() const
{
    return QGCDeviceInfo::isNetworkEthernet();
}



