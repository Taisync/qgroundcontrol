/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include <QTimer>
#include <QList>
#include <QDebug>
#include <QMutexLocker>
#include <iostream>
#include "TCPServerLink.h"
#include "LinkManager.h"
#include "QGC.h"
#include <QHostInfo>
#include <QSignalSpy>

TCPServerLink::TCPServerLink(SharedLinkConfigurationPtr& config)
    : LinkInterface(config)
    , _tcpConfig(qobject_cast<TCPServerConfiguration*>(config.get()))
    , _socket(nullptr)
    , _socketServer(nullptr)
    , _socketIsConnected(false)
{
    Q_ASSERT(_tcpConfig);
}

TCPServerLink::~TCPServerLink()
{
    disconnect();
}

#ifdef TCPLINK_READWRITE_DEBUG
void TCPServerLink::_writeDebugBytes(const QByteArray data)
{
    QString bytes;
    QString ascii;
    for (int i=0, size = data.size(); i<size; i++)
    {
        unsigned char v = data[i];
        bytes.append(QString::asprintf("%02x ", v));
        if (data[i] > 31 && data[i] < 127)
        {
            ascii.append(data[i]);
        }
        else
        {
            ascii.append(219);
        }
    }
    qDebug() << "Sent" << size << "bytes to" << _tcpConfig->host() << ":" << _tcpConfig->port() << "data:";
    qDebug() << bytes;
    qDebug() << "ASCII:" << ascii;
}
#endif

void TCPServerLink::_writeBytes(const QByteArray data)
{
#ifdef TCPLINK_READWRITE_DEBUG
    _writeDebugBytes(data);
#endif

    if (_socket) {
        _socket->write(data);
        emit bytesSent(this, data);
    }
}

void TCPServerLink::newConnectSlot()
{
    if(_socket)
    {
        // This prevents stale signal from calling the link after it has been deleted
        QObject::disconnect(_socket, &QIODevice::readyRead, this, &TCPServerLink::_readBytes);
        _socketIsConnected = false;
        _socket->disconnectFromHost(); // Disconnect tcp
        _socket->deleteLater(); // Make sure delete happens on correct thread
        _socket = nullptr;
    }
    _socket = _socketServer->nextPendingConnection();
    _socket->setSocketOption(QAbstractSocket::LowDelayOption,1);

    QObject::connect(_socket, &QIODevice::readyRead, this, &TCPServerLink::_readBytes);
    emit connected();
    _socketIsConnected = true;
    //QSignalSpy errorSpy(_socket, &QAbstractSocket::errorOccurred);
    //QObject::connect(_socket, &QAbstractSocket::errorOccurred, this, &TCPServerLink::_socketError);
}

void TCPServerLink::_readBytes()
{
    if (_socket) {
        qint64 byteCount = _socket->bytesAvailable();
        if (byteCount)
        {
            QByteArray buffer;
            buffer.resize(byteCount);
            _socket->read(buffer.data(), buffer.size());
            emit bytesReceived(this, buffer);
#ifdef TCPLINK_READWRITE_DEBUG
            _writeDebugBytes(buffer);
#endif
        }
    }
}

void TCPServerLink::disconnect(void)
{
    if(_socketServer)
    {
        _socketServer->disconnect();
        _socketServer->deleteLater();
        _socketServer = nullptr;
    }

    if (_socket) {
        // This prevents stale signal from calling the link after it has been deleted
        QObject::disconnect(_socket, &QIODevice::readyRead, this, &TCPServerLink::_readBytes);
        _socketIsConnected = false;
        _socket->disconnectFromHost(); // Disconnect tcp
        _socket->deleteLater(); // Make sure delete happens on correct thread
        _socket = nullptr;
        emit disconnected();
    }
}

bool TCPServerLink::_connect(void)
{
    if (_socketServer) {
        qWarning() << "connect called while already connected";
        return true;
    }
    return _hardwareConnect();
}

bool TCPServerLink::_hardwareConnect()
{
    Q_ASSERT(_socketServer == nullptr);
    _socketServer = new QTcpServer;
    connect(_socketServer,&QTcpServer::newConnection,this,&TCPServerLink::newConnectSlot);
    _socketServer->listen(QHostAddress::Any,_tcpConfig->port());
    return true;
}

void TCPServerLink::_socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit communicationError(tr("Link Error"), tr("Error on link %1. Error on socket: %2.").arg(_config->name()).arg(_socket->errorString()));
}

/**
 * @brief Check if connection is active.
 *
 * @return True if link is connected, false otherwise.
 **/
bool TCPServerLink::isConnected() const
{
    return _socketIsConnected;
}

//--------------------------------------------------------------------------
//-- TCPServerConfiguration

TCPServerConfiguration::TCPServerConfiguration(const QString& name) : LinkConfiguration(name)
{
    _port    = QGC_TCP_SERVER_PORT;
    // _host    = QLatin1String("0.0.0.0");
}

TCPServerConfiguration::TCPServerConfiguration(TCPServerConfiguration* source) : LinkConfiguration(source)
{
    _port    = source->port();
    // _host    = source->host();
}

void TCPServerConfiguration::copyFrom(LinkConfiguration *source)
{
    LinkConfiguration::copyFrom(source);
    auto* usource = qobject_cast<TCPServerConfiguration*>(source);
    Q_ASSERT(usource != nullptr);
    _port    = usource->port();
    // _host = usource->host();
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

void TCPServerConfiguration::saveSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    settings.setValue("port", (int)_port);
    // settings.setValue("host", _host);
    settings.endGroup();
}

void TCPServerConfiguration::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    _port = (quint16)settings.value("port", QGC_TCP_SERVER_PORT).toUInt();
    // _host = settings.value("host", _host).toString();
    settings.endGroup();
}
