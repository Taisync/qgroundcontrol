/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QLoggingCategory>
#include <QtCore/QString>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QHostAddress>

#include "LinkConfiguration.h"
#include "LinkInterface.h"

//#define TCPLINK_READWRITE_DEBUG   // Use to debug data reads/writes

class QTcpSocket;
class QTcpServer;

Q_DECLARE_LOGGING_CATEGORY(TCPServerLinkLog)

#define QGC_TCP_SERVER_PORT 5760

class TCPServerConfiguration : public LinkConfiguration
{
    Q_OBJECT

    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit TCPServerConfiguration(const QString& name, QObject *parent = nullptr);
    explicit TCPServerConfiguration(const TCPServerConfiguration* source, QObject *parent = nullptr);

    LinkType type(void) const override { return LinkConfiguration::TypeTcpServer; }
    void copyFrom(const LinkConfiguration* source) override;
    void loadSettings(QSettings& settings, const QString& root) override;
    void saveSettings(QSettings& settings, const QString& root) const override;
    QString settingsURL(void) const override { return "TcpServerSettings.qml"; }
    QString settingsTitle(void) const override { return tr("TCP Link Settings"); }

    quint16 port(void) const { return _port; }
    void setHost(const QString host);

private:
    void setPort(quint16 port);

signals:
    void portChanged(void);

private:
    quint16 _port;
};

/*===========================================================================*/

class TCPServerWorker : public QObject
{
    Q_OBJECT

public:
    friend class TCPServerLink;

    explicit TCPServerWorker(const TCPServerConfiguration *config, QObject *parent = nullptr);
    ~TCPServerWorker();

    bool isConnected() const;

    QTcpSocket* getSocket           (void) { return _socket; }
    QTcpServer* getSocketServer           (void) { return _socketServer; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &errorString);
    void dataReceived(const QByteArray &data);
    void dataSent(const QByteArray &data);

public slots:
    void setupSocketServer();
    void disconnect();
    void writeData(const QByteArray &data);

private slots:
    void _onSocketConnected();
    void _onSocketDisconnected();
    void _onSocketReadyRead();
    void _onSocketBytesWritten(qint64 bytes);
    void _onSocketErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    void newConnectSlot();

    const TCPServerConfiguration *_config = nullptr;
    QTcpSocket *_socket = nullptr;
    QTcpServer *_socketServer = nullptr;
    bool _errorEmitted = false;
    bool _socketIsConnected;
};

/*===========================================================================*/

class TCPServerLink : public LinkInterface
{
    Q_OBJECT

public:
    explicit TCPServerLink(SharedLinkConfigurationPtr& config, QObject *parent = nullptr);
    virtual ~TCPServerLink();

    bool isConnected() const override;
    void disconnect() override;
    bool isSecureConnection() const override;

    void        signalBytesWritten  (void);

private slots:
    void _writeBytes(const QByteArray &bytes) override;
    void _onConnected();
    void _onDisconnected();
    void _onErrorOccurred(const QString &errorString);
    void _onDataReceived(const QByteArray &data);
    void _onDataSent(const QByteArray &data);

private:
    bool _connect(void) override;

#ifdef TCPLINK_READWRITE_DEBUG
    void _writeDebugBytes   (const QByteArray data);
#endif

    TCPServerConfiguration* _tcpServerConfig = nullptr;
    TCPServerWorker *_worker = nullptr;
    QThread *_workerThread = nullptr;
};

