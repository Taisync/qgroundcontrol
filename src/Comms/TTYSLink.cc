#ifdef QGC_TTYS_LINK
#include "TTYSLink.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(TTYSLinkLog, "qgc.comms.ttyslink")

TTYSConfiguration::TTYSConfiguration(const QString& name, QObject *parent)
    : LinkConfiguration(name, parent)
{

}

TTYSConfiguration::TTYSConfiguration(const TTYSConfiguration* source, QObject *parent)
    : LinkConfiguration(source, parent)
    , _devFile(source->devFile())
    , _baudRate(source->baudRate())
{

}

void  TTYSConfiguration::copyFrom(const LinkConfiguration* source)
{
    Q_ASSERT(source);
    LinkConfiguration::copyFrom(source);

    const TTYSConfiguration* const ttysSource = qobject_cast<const TTYSConfiguration*>(source);
    Q_ASSERT(ttysSource);

    setDevFile(ttysSource->devFile());
    setBaudRate(ttysSource->baudRate());
}

void  TTYSConfiguration::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    _devFile= settings.value("TTYSFile").toString();
    _baudRate = settings.value("baudRate").toString();
    settings.endGroup();
}

void  TTYSConfiguration::saveSettings(QSettings& settings, const QString& root) const
{
    settings.beginGroup(root);
    settings.setValue("TTYSFile",_devFile);
    settings.setValue("baudRate",_baudRate);
    settings.endGroup();
}

void TTYSConfiguration::setDevFile (const QString& devFile)
{
    QString dfile = devFile.trimmed();
    if (!dfile.isEmpty() && dfile != _devFile) {
       _devFile = dfile;
        emit devFileChanged();
    }
}

void TTYSConfiguration::setBaudRate(const QString& baudRate)
{
    QString _bRate = baudRate.trimmed();
    if (!_bRate.isEmpty() && _baudRate != _bRate) {
        _baudRate = _bRate;
        emit baudRateChanged();
    }
}

//  Q_INVOKABLE void TTYSConfiguration::changDevFile(QString devFile)
// {
//     _devFile = devFile;
// }

// Q_INVOKABLE void TTYSConfiguration::changBaudRate(QString baudRate)
// {
//     _baudRate = baudRate;
// }

/*===========================================================================*/
TTYSAsyncReader::TTYSAsyncReader()
{
    _thread = new QThread;
    moveToThread(_thread);
    _thread->start();

    connect(this, &TTYSAsyncReader::_startSignal, this, &TTYSAsyncReader::_onStart);
}

void TTYSAsyncReader::start(TTYSWorker &handler)
{
    _running = true;
    _handler = &handler;

    emit _startSignal();
}

void TTYSAsyncReader::stop()
{
    _running = false;
}

void TTYSAsyncReader::_onStart()
{
    while (_running) {
        QByteArray tmp = _handler->read();
        if (tmp.size() > 0) {
            emit onReadData(tmp);
        }
        // QThread::msleep(1);
    }
}

/*===========================================================================*/

TTYSWorker::TTYSWorker(const TTYSConfiguration *config, QObject *parent)
    : QObject(parent)
    , _config(config)
    , _asyncReader(new TTYSAsyncReader())
{
    // qCDebug(TTYSLinkLog) << Q_FUNC_INFO << this;
}

TTYSWorker::~TTYSWorker()
{
    disconnectFromDevice();
    delete _asyncReader;
    _asyncReader = nullptr;
    // qCDebug(TTYSLinkLog) << Q_FUNC_INFO << this;
}

bool TTYSWorker::isConnected() const
{
    if(fd > 0)
        return true;
    else
        return false;
}

QByteArray TTYSWorker::read() const
{
    struct pollfd pfd;
    memset(&pfd,0,sizeof(pfd));
    pfd.fd = fd;
    pfd.events = POLLIN;
    if(poll(&pfd,1,1000)<0) return {};
    if(!(pfd.revents & POLLIN))  return {};

    char rBuffer[1024] = {0};
    memset(rBuffer,0,sizeof(rBuffer));
    int byteCount = ::read(fd,rBuffer,sizeof(rBuffer));
    QByteArray data;
    if (byteCount > 0) data.append(rBuffer,byteCount);
    return data;
}

void TTYSWorker::setupTTYS()
{
    qCDebug(TTYSLinkLog) << "CONNECT CALLED";
    qDebug() << "*************************:TTYLink Connect********************";
    if (fd > 0) {
        qCWarning(TTYSLinkLog) << "TtysLink:connect called while already connected";
        return;
    }

    (void) connect(_asyncReader, &TTYSAsyncReader::onReadData, this, &TTYSWorker::_onTTYSReadData, Qt::QueuedConnection);
    qCWarning(TTYSLinkLog) << "TtysLink:Initialize the connection";
}

void TTYSWorker::connectToDevice()
{
    if (isConnected()) {
        qCWarning(TTYSLinkLog) << "Already connected to" << _config->devFile() << ":" << _config->baudRate();
        return;
    }

    _errorEmitted = false;

    if(fd < 0) {
        fd = open(_config->devFile().toUtf8().data(),O_RDWR | O_NOCTTY | O_NDELAY);
        if(fd < 0) {
            if (!_errorEmitted) {
                emit errorOccurred(tr("Connection Failed: %1").arg(tr("TTYS open failed")));
                _errorEmitted = true;
            }
            _onTTYSDisconnected();
            return;
        }
        /******* 115200 *********/
        struct termios opt;
        //bzero(&opt, sizeof(opt));
        tcgetattr(fd,&opt);       //

        switch(_config->baudRate().toInt())
        {
        case 115200:
            cfsetispeed(&opt,B115200);//
            break;
        case 9600:
            cfsetispeed(&opt,B9600);//
            break;
        case 57600:
            cfsetispeed(&opt,B57600);//
            break;
        case 230400:
            cfsetispeed(&opt,B230400);//
            break;
        }

        /*c_lflag */
        opt.c_cflag &=~INPCK; //
        opt.c_cflag |=(CLOCAL | CREAD);//
        //opt.c_lflag &= ~(ICANON | ECHO | ECHOE |  ISIG); //
        opt.c_lflag &= ~(ICANON |ISIG);
        /* c_oflag  */
        opt.c_oflag &= ~ OPOST;             //
        //opt.c_oflag &= ~(ONLCR | OCRNL);    //
        opt.c_oflag &= ~(ONLRET | ONOCR | OFILL| ONLCR| OCRNL|OFDEL);
        /* c_iflag  */
        opt.c_iflag &= ~(ICRNL | INLCR| IGNCR);          //
        opt.c_iflag &= ~(IXON | IXOFF | IXANY);    //
        opt.c_iflag &= ~(IGNBRK | BRKINT| PARMRK | INPCK | ISTRIP | IXON|ICANON);

        opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IUCLC);

        /* c_cflag  */
        opt.c_cflag &= ~ CSIZE;     //
        opt.c_cflag |=  CS8;        //
        opt.c_cflag &= ~ CSTOPB;    //
        opt.c_cflag &= ~ PARENB;    //

        opt.c_oflag &= ~OPOST;
        opt.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        opt.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                         | INLCR | IGNCR | ICRNL | IXON);

        opt.c_cc[VTIME] = 128;
        opt.c_cc[VMIN] = 1;
        tcflush(fd, TCIOFLUSH);         //
        tcsetattr(fd, TCSANOW, &opt);   //
        FD_ZERO(&rd);
        FD_SET(fd,&rd);
        FD_ISSET(fd,&rd);
        FD_ZERO(&wd);
        FD_SET(fd,&wd);
        FD_ISSET(fd,&wd);

        qCInfo(TTYSLinkLog) << "Connect Success:" << _config->devFile() << _config->baudRate();
    }

    emit connected();
}

void TTYSWorker::disconnectFromDevice()
{
    if (!isConnected()) {
        qCDebug(TTYSLinkLog) << "Already disconnected from host:" << _config->devFile() << "port:" << _config->baudRate();
        return;
    }

    qCDebug(TTYSLinkLog) << "Attempting to disconnect from host:" << _config->devFile() << "port:" << _config->baudRate();

    if(fd) close(fd);
    fd = -1;
    _asyncReader->stop();
    emit disconnected();
    qCInfo(TTYSLinkLog) << "###############TtysLink:disconnect success###########";
}

void TTYSWorker::writeData(const QByteArray &data)
{
    if (data.isEmpty()) {
        emit errorOccurred(tr("Data to Send is Empty"));
        return;
    }

    if (!isConnected()) {
        emit errorOccurred(tr("TTYS is not connected"));
        return;
    }

    const qint64 bytesWritten = write(fd, data.data(), data.size());
    emit dataSent(data);
}

void TTYSWorker::_onTTYSConnected()
{
    qCDebug(TTYSLinkLog) << "TTYS connected:" << _config->devFile() << _config->baudRate();
    _errorEmitted = false;
    emit connected();
}

void TTYSWorker::_onTTYSDisconnected()
{
    qCDebug(TTYSLinkLog) << "TTYS disconnected:" << _config->devFile() << _config->baudRate();
    _errorEmitted = false;
    emit disconnected();
}

void TTYSWorker::_onTTYSReadyRead()
{
    if (!_asyncReader->isRunning()) {
        _asyncReader->start(*this);
    }
}

void TTYSWorker::_onTTYSReadData(const QByteArray& data)
{
    emit dataReceived(data);
}

void TTYSWorker::_onTTYSBytesWritten(qint64 bytes)
{
    qCDebug(TTYSLinkLog) << _config->devFile() << "Wrote" << bytes << "bytes";
}

void TTYSWorker::_onTTYSErrorOccurred(const QString& errorMsg)
{
    qCWarning(TTYSLinkLog) << "TTYS error:" << errorMsg;

    if (!_errorEmitted) {
        emit errorOccurred(errorMsg);
        _errorEmitted = true;
    }
}

/*===========================================================================*/

TTYSLink::TTYSLink(SharedLinkConfigurationPtr& config, QObject *parent)
    : LinkInterface(config, parent)
    , _ttysConfig(qobject_cast<const TTYSConfiguration*>(config.get()))
    , _worker(new TTYSWorker(_ttysConfig))
    , _workerThread(new QThread(this))
{
    _workerThread->setObjectName(QStringLiteral("TTYS_%1").arg(_ttysConfig->name()));

    _worker->moveToThread(_workerThread);

    (void) connect(_workerThread, &QThread::started, _worker, &TTYSWorker::setupTTYS);
    (void) connect(_workerThread, &QThread::finished, _worker, &QObject::deleteLater);

    (void) connect(_worker, &TTYSWorker::connected, this, &TTYSLink::_onConnected, Qt::QueuedConnection);
    (void) connect(_worker, &TTYSWorker::disconnected, this, &TTYSLink::_onDisconnected, Qt::QueuedConnection);
    (void) connect(_worker, &TTYSWorker::errorOccurred, this, &TTYSLink::_onErrorOccurred, Qt::QueuedConnection);
    (void) connect(_worker, &TTYSWorker::dataReceived, this, &TTYSLink::_onDataReceived, Qt::QueuedConnection);
    (void) connect(_worker, &TTYSWorker::dataSent, this, &TTYSLink::_onDataSent, Qt::QueuedConnection);

    _workerThread->start();
}

TTYSLink::~TTYSLink() {
    TTYSLink::disconnect();

    _workerThread->quit();
    if (!_workerThread->wait()) {
        qCWarning(TTYSLinkLog) << "Failed to wait for TTYS Thread to close";
    }
}

bool TTYSLink::isConnected() const { return _worker->isConnected(); }

bool TTYSLink::_connect() {
    return QMetaObject::invokeMethod(_worker, "connectToDevice", Qt::QueuedConnection);
}

void TTYSLink::disconnect() { (void)QMetaObject::invokeMethod(_worker, "disconnectFromDevice", Qt::QueuedConnection); }

 void TTYSLink::_onConnected()
 {
     emit connected();

     (void) QMetaObject::invokeMethod(_worker, "_onTTYSReadyRead", Qt::QueuedConnection);
 }

 void TTYSLink::_onDisconnected()
 {
     emit disconnected();
 }

 void TTYSLink::_onErrorOccurred(const QString& errorString) {
     qCWarning(TTYSLinkLog) << "Communication error:" << errorString;
     emit communicationError(tr("TTYS Link Error"), tr("Link %1: (dev: %2 Port: %3) %4")
                                                        .arg(_ttysConfig->name(), _ttysConfig->devFile(),
                                                                                             _ttysConfig->baudRate(), errorString));
 }

void TTYSLink::_onDataReceived(const QByteArray &data)
{
    emit bytesReceived(this, data);
}

void TTYSLink::_onDataSent(const QByteArray &data)
{
    emit bytesSent(this, data);
}

void TTYSLink::_writeBytes(const QByteArray& bytes)
{
    (void) QMetaObject::invokeMethod(_worker, "writeData", Qt::QueuedConnection, Q_ARG(QByteArray, bytes));
}

#endif
