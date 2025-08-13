#pragma once

#ifdef QGC_TTYS_LINK

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QString>
#include <QMetaType>
#include <QLoggingCategory>
#include "LinkConfiguration.h"
#include "LinkInterface.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>

Q_DECLARE_LOGGING_CATEGORY(TTYSLinkLog)

/// SerialLink configuration
class TTYSConfiguration : public LinkConfiguration
{
    Q_OBJECT

    Q_PROPERTY(QString devFile READ devFile WRITE setDevFile NOTIFY devFileChanged)
    Q_PROPERTY(QString baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)

public:
    explicit TTYSConfiguration(const QString& name, QObject *parent = nullptr);
    explicit TTYSConfiguration(const TTYSConfiguration* source, QObject *parent = nullptr);

    LinkType type(void) const override { return LinkConfiguration::TypeTtys; }
    void copyFrom(const LinkConfiguration *source) override;
    void loadSettings(QSettings &settings, const QString &root) override;
    void saveSettings(QSettings &settings, const QString &root) const override;
    QString settingsURL() const override { return QStringLiteral("TtysSettings.qml"); }
    QString settingsTitle() const override { return tr("TTYS Link Settings"); }

    QString devFile() const { return _devFile; }
    QString baudRate() const { return _baudRate; }
    void setDevFile(const QString &devFile);
    void setBaudRate(const QString &baudRate);

signals:
    void devFileChanged();
    void baudRateChanged();

private:
    QString _devFile;
    QString _baudRate;
};

/*===========================================================================*/
class TTYSWorker;
class TTYSAsyncReader : public QObject
{
    Q_OBJECT
public:
    explicit TTYSAsyncReader();

    void start(TTYSWorker& handler);
    void stop();

    bool isRunning() { return _running; }

signals:
    void onReadData(const QByteArray &data);

private:
    Q_SIGNAL void _startSignal();

private slots:
    void _onStart();

private:
    QThread* _thread;
    TTYSWorker *_handler;
    std::atomic<bool> _running{false};
};


/*===========================================================================*/

class TTYSWorker : public QObject
{
    Q_OBJECT

public:
    explicit TTYSWorker(const TTYSConfiguration *config, QObject *parent = nullptr);
    ~TTYSWorker();

    bool isConnected() const;

    QByteArray read() const;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &errorString);
    void dataReceived(const QByteArray &data);
    void dataSent(const QByteArray &data);

public slots:
    void setupTTYS();
    void connectToDevice();
    void disconnectFromDevice();
    void writeData(const QByteArray &data);

private slots:
    void _onTTYSConnected();
    void _onTTYSDisconnected();
    void _onTTYSReadyRead();
    void _onTTYSReadData(const QByteArray& data);
    void _onTTYSBytesWritten(qint64 bytes);
    void _onTTYSErrorOccurred(const QString& errorMsg);

private:
    const TTYSConfiguration *_config = nullptr;
    int                      fd = -1;
    fd_set rd;
    fd_set wd;
    bool _errorEmitted = false;
    TTYSAsyncReader *_asyncReader = nullptr;
};

class TTYSLink: public LinkInterface
{
    Q_OBJECT

public:
    explicit TTYSLink(SharedLinkConfigurationPtr& config, QObject *parent = nullptr);
    virtual ~TTYSLink();

    bool isConnected() const override;
    void disconnect () override;

private slots:
    void _writeBytes(const QByteArray &bytes) override;
    void _onConnected();
    void _onDisconnected();
    void _onErrorOccurred(const QString &errorString);
    void _onDataReceived(const QByteArray &data);
    void _onDataSent(const QByteArray &data);

private:
    bool _connect() override;

    const TTYSConfiguration *_ttysConfig = nullptr;
    TTYSWorker *_worker = nullptr;
    QThread *_workerThread = nullptr;
};
#endif
