#ifdef __android__
#include "TTYSLink.h"
#include "QGC.h"
#include "QGCLoggingCategory.h"
#include "QGCApplication.h"

QGC_LOGGING_CATEGORY(TtysLinkLog, "TtysLinkLog")


TtysConfiguration::TtysConfiguration(const QString& name) : LinkConfiguration(name)
{
    //_devFile = name;
}

TtysConfiguration::TtysConfiguration(TtysConfiguration* copy) : LinkConfiguration(copy)
{
    _devFile = copy->devFile();
    _baudRate = copy->baudRate();
}

void  TtysConfiguration::copyFrom(LinkConfiguration* source)
{
    LinkConfiguration::copyFrom(source);
    auto* ssource = qobject_cast<TtysConfiguration*>(source);
    if (ssource) {
        _devFile  = ssource->devFile();
        _baudRate = ssource->baudRate();
    } else {
        qWarning() << "Internal error";
    }
}

void  TtysConfiguration::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    _devFile= settings.value("TTYSFile").toString();
    _baudRate = settings.value("baudRate").toString();
    settings.endGroup();
}

void  TtysConfiguration::saveSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    settings.setValue("TTYSFile",_devFile);
    settings.setValue("baudRate",_baudRate);
    settings.endGroup();
}

void TtysConfiguration::setDevFile (const QString& devFile)
{
    QString dfile = devFile.trimmed();
    if (!dfile.isEmpty() && dfile != _devFile) {
       _devFile = dfile;
    }
}

void TtysConfiguration::setBaudRate(const QString& baudRate)
{
    QString _bRate = baudRate.trimmed();
    if (!_bRate.isEmpty() && _baudRate != _bRate) {
        _baudRate = _bRate;
    }
}

 Q_INVOKABLE void TtysConfiguration::changDevFile(QString devFile)
{
     _devFile = devFile;
}

Q_INVOKABLE void TtysConfiguration::changBaudRate(QString baudRate)
{
     _baudRate = baudRate;
}

TTYSLink::TTYSLink(SharedLinkConfigurationPtr& config)
    : LinkInterface(config)
    , _running(false)
    , _ttysConfig(qobject_cast<TtysConfiguration*>(config.get()))
{
    qWarning() <<"Create TtysLink(AIO3568J),devfile:" << _ttysConfig->devFile();
}

 TTYSLink::~TTYSLink()
 {
    disconnect();
 }

// LinkInterface overrides
bool TTYSLink::isConnected(void) const
{
    if(fd > 0 && m_notifier)
        return true;
    else
        return false;
}

void TTYSLink::disconnect (void)
{
#ifdef __android__
    if(_running)
    {
        // Tell the thread to exit
        _running = false;
        quit();
        // Wait for it to exit
        wait();
    }
    //QObject::disconnect(m_notifier,SIGNAL(activated(int)),this,SLOT(_readBytes()));
    if(fd)
        close(fd);
    if(m_notifier)
        delete m_notifier;
    fd = -1;
    m_notifier = nullptr;
    emit disconnected();
    qDebug() << "###############TtysLink:disconnect success###########";
#endif
}

void TTYSLink::run(void)
{
#ifdef __android__
    while(_running)
    {
        struct pollfd pfd;
        memset(&pfd,0,sizeof(pfd));
        pfd.fd = fd;
        pfd.events = POLLIN;
        if(poll(&pfd,1,1000)<0)
            continue;
        if(!(pfd.revents & POLLIN))
            continue;
        //QThread::msleep(1);
        _readBytes();
    }
#endif
}

void TTYSLink::_writeBytes(const QByteArray data)
{
#ifdef __android__
    if (m_notifier && fd > 0){
        emit bytesSent(this, data);
        write(fd,data.data(),data.size());
    } else {
        _emitLinkError(tr("Could not send data - link %1 is disconnected!").arg(_config->name()));
    }
#endif
}

void TTYSLink::_readBytes(void)
{
#ifdef __android__
    char rBuffer[1024] = {0};
    if (m_notifier && fd > 0) {
          memset(rBuffer,0,sizeof(rBuffer));
          int byteCount = read(fd,rBuffer,sizeof(rBuffer));
          QByteArray data;
          data.append(rBuffer,byteCount);
          emit bytesReceived(this,data);
#if 0
          if(byteCount>0)
          {
              buf.append(rBuffer,byteCount);
              for(int i = 0; i < buf.count();i++)
              {
                  if(buf.data()[i] == 0xFe)
                  {
                      buf = buf.right(buf.count()-i);
                      if(buf.count()<= 2)
                          return;
                      if(buf.data()[1]+8 <= buf.count())
                      {
                          QByteArray packet =  buf.left(buf.data()[1]+8);
                          buf = buf.right(buf.count() - buf.data()[1]-8);
                          emit bytesReceived(this, packet);
                          i = -1;
                          continue;
                      }
                      else {
                          return;
                      }
                  }
                  if(buf.data()[i] == 0xFD)
                  {
                      buf = buf.right(buf.count()-i);
                      if(buf.count()<= 2)
                          return;
                      if(buf.data()[1]+12 <= buf.count())
                      {
                          QByteArray packet =  buf.left(buf.data()[1]+12);
                          buf = buf.right(buf.count() - buf.data()[1]-12);
                          emit bytesReceived(this, packet);
                          i = -1;
                          continue;
                      }
                      else {
                          return;
                      }
                  }
              }
              QByteArray buffer(rBuffer,byteCount);
              emit bytesReceived(this, buffer);
          }
          if(buf.count() > 1024*10)
              buf.clear();
#endif
    } else {
        qWarning() << "TTYS not readable";
    }
#endif
}

bool TTYSLink::_connect(void)
{
    qCDebug(TtysLinkLog) << "CONNECT CALLED";
    qDebug() << "*************************:TTYLink Connect********************";
    if (fd  >  0 && m_notifier) {
        qCWarning(TtysLinkLog) << "TtysLink:connect called while already connected";
        return true;
    }
    qCWarning(TtysLinkLog) << "TtysLink:Initialize the connection";
    // Initialize the connection
    if (!_hardwareConnect()) {
        _emitLinkError(tr("Error connecting: Could not open devFile. %1").arg(_ttysConfig->devFile()));
        return false;
    }
    /* TTYS,Mavlink2.0 */
    mavlink_status_t* mavlinkStatus = mavlink_get_channel_status(mavlinkChannel());
    mavlinkStatus->flags &= ~MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
    return true;
}

bool TTYSLink::_hardwareConnect   (void)
{
    if(fd < 0 && !m_notifier)
    {
#ifdef __android__
        if(_ttysConfig->devFile().isEmpty()
        ||_ttysConfig->devFile().isNull()
        ||_ttysConfig->devFile()=="")
        {
            _ttysConfig->setDevFile("/dev/ttyHS2");
        }

        fd = open(_ttysConfig->devFile().toUtf8().data(),O_RDWR | O_NOCTTY | O_NDELAY);
        if(fd < 0)
            return false;
        /******* 115200 *********/
        struct termios opt;
        //bzero(&opt, sizeof(opt));
        tcgetattr(fd,&opt);       //

        switch(_ttysConfig->baudRate().toInt())
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

        m_notifier = new QSocketNotifier(fd,QSocketNotifier::Read,this);
        _running = true;
        this->start(NormalPriority);
        //connect(m_notifier,SIGNAL(activated(int)),this,SLOT(_readBytes()));
        qDebug() << "###########TtysLink:connect success###########" << _ttysConfig->devFile();
        qDebug() << "###########TtysLink:connect success###########" << _ttysConfig->baudRate();
#endif
    }

    emit connected();
    return true;
}

void TTYSLink::_emitLinkError     (const QString& errorMsg)
{
    qWarning() << "TtysLinkLog::linkError:" << errorMsg;
}
#endif
