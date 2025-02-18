#include "taisyncromotehandler.h"
#include <QDebug>
#include "QGCApplication.h"
#include "AppSettings.h"
#include "SettingsManager.h"
#ifdef __android__
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <QMessageBox>
#include <poll.h>
#include <string.h>
#endif

static const QString logDir = "/storage/emulated/0/Android/data/QGCLogs";

TaisyncRomoteHandler::TaisyncRomoteHandler()
{
    connect(&workTimer,&QTimer::timeout,this, &TaisyncRomoteHandler::runTask);
#ifdef TAISYNC_FLY_CAL
    Fact* taisyncFlyViewShowFact = qgcApp()->toolbox()->settingsManager()->appSettings()->taisyncFlyViewShow();
    connect(taisyncFlyViewShowFact, &Fact::valueChanged, this, &TaisyncRomoteHandler::_onEnableChanged);

    Fact *saveDataFact = qgcApp()->toolbox()->settingsManager()->appSettings()->taisyncFlyDataSave();
    if (saveDataFact) {
        connect(saveDataFact, &Fact::valueChanged, this, &TaisyncRomoteHandler::onAutoSaveChanged);
    }

    onAutoSaveChanged();
#endif
     qDebug() << "TaisyncRomoteHandler()";
}

bool TaisyncRomoteHandler::connectToDev()
{
#ifdef __android__
    QString ttys = "/dev/ttyMSM0";
    fd = open(ttys.toUtf8().data(),O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd < 0)
        return false;
    /******* 115200 *********/
    struct termios opt;
    //bzero(&opt, sizeof(opt));
    tcgetattr(fd,&opt);       //
    cfsetispeed(&opt,B115200);//

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
#endif
    return true;
}

TaisyncRomoteHandler::~TaisyncRomoteHandler()
{
    qDebug() << "~TaisyncRomoteHandler()";
}

int   TaisyncRomoteHandler::setOnline(const bool &a)
{
      _online = a;
      emit onlineChanged();
      return 0;
}

int   TaisyncRomoteHandler::setAirRssi0(const int &a)
{
       _airRssi0 = a;
       emit airRssi0Changed();
       return 0;
}

int   TaisyncRomoteHandler::setAirRssi1(const int &a)
{
      _airRssi1 = a;
      emit airRssi1Changed();
      return 0;
}

int   TaisyncRomoteHandler::setAirSnr(const int &a)
{
      _airSnr = a;
      emit airSnrChanged();
      return 0;
}

int   TaisyncRomoteHandler::setAirDistance(const int &a)
{
       _airDistance = a;
       emit airDistanceChanged();
       return 0;
}

int   TaisyncRomoteHandler::setAirLDPCFailed(const int &a)
{
      _airLDPCFailed  = a;
      emit airLDPCFailedChanged();
      return 0;
}

int   TaisyncRomoteHandler::setAirLinkQuaity(const int &a)
{
      _airLinkQuaity = a;
      emit airLinkQuaityChanged();
      return 0;
}

int   TaisyncRomoteHandler::setGndRssi0(const int &a)
{
      _gndRssi0 = a;
      emit gndRssi0Changed();
      return 0;
}

int   TaisyncRomoteHandler::setGndRssi1(const int &a)
{
      _gndRssi1 = a;
      emit gndRssi1Changed();
      return 0;
}

int   TaisyncRomoteHandler::setGndSnr(const int &a)
{
      _gndSnr = a;
      emit gndSnrChanged();
      return 0;
}

int   TaisyncRomoteHandler::setGndDistance(const int &a)
{
     _gndDistance = a;
     emit gndDistanceChanged();
     return 0;
}

 int   TaisyncRomoteHandler::setGndLDPCFailed(const int &a)
 {
     _gndLDPCFailed = a;
     emit gndLDPCFailedChanged();
     return 0;
 }

int   TaisyncRomoteHandler::setGndLinkQuaity(const int &a)
{
  _gndLinkQuaity = a;
  emit gndLinkQuaityChanged();
  return 0;
}

int   TaisyncRomoteHandler::setGndFreq(const int &a)
{
   _gndFreq = a;
   emit gndFreqChanged();
   return 0;
}

int   TaisyncRomoteHandler::setGndPower(const int &a)
{
    _gndPower = a;
    emit gndPowerChanged();
    return 0;
}

int   TaisyncRomoteHandler::setEthTxRate(const quint64&a)
{
    _ethTxRate = a;
    emit ethTxRateChanged();
    return 0;
}

bool TaisyncRomoteHandler::ttysWrite(QByteArray data)
{
#ifdef __android__
    if(fd < 0)
    {
        return false;
    }
    else
    {
       struct pollfd pfd;
       memset(&pfd,0,sizeof(pfd));
       pfd.fd = fd;
       pfd.events = POLLOUT;//writeable
       if(poll(&pfd,1,1000)<0)
           return false;
       if(pfd.revents & POLLOUT)
       {
           return write(fd,data.data(),data.size()) > 0?true:false;
       }
       else {
           return false;
       }
    }
#endif
    return  false;
}

QByteArray TaisyncRomoteHandler::ttysRead()
{
#ifdef __android__
    readDone = false;
    char rBuffer[1024] = {0};
    if (fd > 0) {
          int byteCount = read(fd,rBuffer,sizeof(rBuffer));
          if(byteCount>0)
          {
              QByteArray buffer(rBuffer,byteCount);
              return buffer;
          }
    }
#endif
    QByteArray data;
    data.clear();
    return data;
}


bool TaisyncRomoteHandler::ttysReadDone(QByteArray &readData,int timeMs)
{
    int cnt  = timeMs;
    QByteArray buffer;
    buffer.clear();
    while (cnt--){
         QByteArray data = ttysRead();
         if(data.size() == 0)
         {
             QThread::msleep(1);
             continue;
         }
         buffer.append(data);
         if(buffer.contains("\r\n"))
         {
             data = buffer;
             buffer.clear();
             int index = data.indexOf(QByteArray(" {"));
             int lastIndex = data.indexOf(QByteArray("\r\n"));
             if(lastIndex > index)
             {
               QByteArray cmd = data.left(index);
               data = data.mid(index+1,lastIndex-index-1);
               readData = data;
               readDone = true;
               return true;
             }
         }
         QThread::msleep(1);
    }
    return false;
}

void TaisyncRomoteHandler::disconnectToDev()
{
#ifdef __android__
    close(fd);
#endif
}

bool TaisyncRomoteHandler::getLocalInfo()
{
    if(ttysWrite(LOCAL_INFO)>0)
    {
        QByteArray data;
        if(!ttysReadDone(data,1000))
            return false;
        //qDebug() << "LocalInfo: " << data;

        QJsonParseError errRpt;
        QJsonDocument  rootDoc = QJsonDocument::fromJson(data, &errRpt);//The string is formatted as JSON
        if(errRpt.error != QJsonParseError::NoError)
        {
            return false;
        }
        else
        {
            QJsonObject rootObj = rootDoc.object();
            if(rootObj.contains("pass"))
            {
                //ptpLocalInfo.pass = rootObj.value("pass").toString();
            }
            if(rootObj.contains("failed"))
            {
                //ptpLocalInfo.failed = rootObj.value("failed").toString();
                int ldpcFailed = rootObj.value("failed").toString().toInt();
                setGndLDPCFailed(ldpcFailed);
            }
            if(rootObj.contains("snr"))
            {
                //ptpLocalInfo.snr = rootObj.value("snr").toString();
                int snr = rootObj.value("snr").toString().toInt();
                setGndSnr(snr);
            }
            if(rootObj.contains("rssi0"))
            {
                //ptpLocalInfo.rssi0 = rootObj.value("rssi0").toString();
                int rssi0 = rootObj.value("rssi0").toString().toInt();
                setGndRssi0(rssi0);
            }
            if(rootObj.contains("rssi1"))
            {
                int rssi1 = rootObj.value("rssi1").toString().toInt();
                setGndRssi1(rssi1);
            }

            if(rootObj.contains("linkQuaity"))
            {
                int linkQuaity = rootObj.value("linkQuaity").toString().toInt();
                setGndLinkQuaity(linkQuaity);
                //qDebug()<<"linkQuaity: " << ptpLocalInfo.linkQuaity;
            }

            if(rootObj.contains("distance"))
            {
                int distance = rootObj.value("distance").toString().toInt();
                setGndDistance(distance);
            }
            //emit updateLocalInfo(ptpLocalInfo);
            return true;
        }
    }
    return false;
}

bool TaisyncRomoteHandler::getLocalStatus()
{
    if(ttysWrite(LOCAL_STATUS)>0)
    {
        QByteArray data;
        if(!ttysReadDone(data,500))
            return false;
        QJsonParseError errRpt;
        QJsonDocument  rootDoc = QJsonDocument::fromJson(data, &errRpt);
        if(errRpt.error != QJsonParseError::NoError)
        {
            return false;
        }
        else
        {
            QJsonObject rootObj = rootDoc.object();
            if(rootObj.contains("ethtxDataRate"))
            {
                quint64 rate = rootObj.value("ethtxDataRate").toString().toULongLong();
                setEthTxRate(rate);
            }
            return true;
        }
    }
    return false;
}

bool TaisyncRomoteHandler::getPeerInfo()
{
    if(ttysWrite(QByteArray(PEER_INFO))>0)
    {
        QByteArray data;
        if(!ttysReadDone(data,1000))
            return false;
        //qDebug() << "PeerInfo: " << data;
        QJsonParseError errRpt;
        QJsonDocument  rootDoc = QJsonDocument::fromJson(data, &errRpt);
        if(errRpt.error != QJsonParseError::NoError)
        {
            return false;
        }
        else
        {
            QJsonObject rootObj = rootDoc.object();
            if(rootObj.contains("pass"))
            {
              //  ptpPeerInfo.pass = rootObj.value("pass").toString();
              //qDebug()<<"Peer pass: " << ptpPeerInfo.pass;
            }
            if(rootObj.contains("failed"))
            {
                int ldpcFailed = rootObj.value("failed").toString().toInt();
                setAirLDPCFailed(ldpcFailed);
                //qDebug()<<"Peer failed: " << ptpPeerInfo.failed;
            }
            if(rootObj.contains("snr"))
            {
                //ptpPeerInfo.snr = rootObj.value("snr").toString();
                int snr = rootObj.value("snr").toString().toInt();
                setAirSnr(snr);
                //qDebug()<<"Peer snr: " << ptpPeerInfo.snr;
            }
            if(rootObj.contains("rssi0"))
            {
                int rssi0 = rootObj.value("rssi0").toString().toInt();
                setAirRssi0(rssi0);
                //qDebug()<<"Peer rssi0: " << ptpPeerInfo.rssi0;
            }
            if(rootObj.contains("rssi1"))
            {
                int rssi1 = rootObj.value("rssi1").toString().toInt();
                setAirRssi1(rssi1);
                //qDebug()<<"Peer rssi1: " << ptpPeerInfo.rssi1;
            }

            if(rootObj.contains("status"))
            {
                //ptpPeerInfo.status = rootObj.value("status").toString();
                //qDebug()<<"Peer status: " << ptpPeerInfo.status;
            }
            if(rootObj.contains("linkQuaity"))
            {
                int linkQuaity = rootObj.value("linkQuaity").toString().toInt();
                setAirLinkQuaity(linkQuaity);
                //qDebug()<<"Peer linkQuaity: " << ptpPeerInfo.linkQuaity;
            }
            if(rootObj.contains("distance"))
            {
                int distance = rootObj.value("distance").toString().toInt();
                setAirDistance(distance);
                //qDebug()<<"Peer distance: " << ptpPeerInfo.distance;
            }
            //emit updateLocalInfo(ptpPeerInfo);
            return true;
        }
    }
    return false;
}


bool TaisyncRomoteHandler::getRfInfo()
{
    if(ttysWrite(RF_INFO)>0)
    {
        QByteArray data;
        if(!ttysReadDone(data,1000))
            return false;

        QJsonParseError errRpt;
        QJsonDocument  rootDoc = QJsonDocument::fromJson(data, &errRpt);
        if(errRpt.error != QJsonParseError::NoError)
        {
            return false;
        }
        else
        {
            QJsonObject rootObj = rootDoc.object();

            if(rootObj.contains("freq"))
            {
                setGndFreq(rootObj.value("freq").toString().toInt());
                //ptpLocalInfo.freq = rootObj.value("freq").toString();
            }
            if(rootObj.contains("Power"))
            {
                setGndPower(rootObj.value("Power").toString().toInt());
                //ptpLocalInfo.freq = rootObj.value("freq").toString();
            }
            return true;
        }
    }
    return false;
}

Q_INVOKABLE void TaisyncRomoteHandler::startSaveLog()
{
    autoClearLog();

    QString filePath = logDir;
    QDateTime currDateTime = QDateTime::currentDateTime();
    QString curDateTimeStr = currDateTime.toString("yyyyMMdd");
    QString curTimeStr     = currDateTime.toString("hhmmss");
    QString fileDirPath    = filePath.append("/").append(curDateTimeStr).append(curTimeStr).append(".txt");
    curLogFileName = fileDirPath;
    isStart = true;
}

Q_INVOKABLE void TaisyncRomoteHandler::stopSaveLog() {
    isStart = false;
}

quint64 TaisyncRomoteHandler::getLogsSize(const QString filePath)
{
    QDir tmpDir(filePath);
    quint64 size = 0;
    /*Get the file list statistics file size*/
    foreach(QFileInfo fileInfo, tmpDir.entryInfoList(QDir::Files))
    {
        size += fileInfo.size();
    }
    return size/1024/1024;
}


void TaisyncRomoteHandler::autoClearLog()
{
    // only save N log file max
    // if txt file num > N, delete all the history files
    static int maxFileNumber = 30;
    QDir dir(logDir);
    if (!dir.exists()) return;
    QStringList filters;
    filters << "*.txt";
    dir.setFilter(QDir::Files | QDir::Readable | QDir::NoSymLinks | QDir::Writable);
    dir.setNameFilters(filters);
    QStringList fileList = dir.entryList();
    if (fileList.size() >= maxFileNumber) {
        // sort by filename
        const auto getNum = [](const QString&s){
            bool ok;
            QString numString = s.section('.',0,0);
            qulonglong num = numString.toULongLong(&ok);
            return num;
        };
        std::sort(fileList.begin(), fileList.end(), [&getNum](const QString& a, const QString& b) {
            return getNum(a) < getNum(b);
        });
        while(fileList.size() >= maxFileNumber) {
            QString fileName = fileList.first();
            fileList.pop_front();
            QString filePath = logDir;
            filePath.append("/").append(fileName);
            qDebug() << "auto delete logfile:" << filePath;
            QFile::remove(filePath);
        }
    }
}


void TaisyncRomoteHandler::saveLog(QString data)
{
    QString filePath = logDir;
    QDir dir(filePath);
    if(!dir.exists())
    {
        bool rt = dir.mkdir(filePath);
        if(!rt)
            return;
    }
    if(!isStart)
        return;
      QDateTime currDateTime = QDateTime::currentDateTime();
//    QString curDateTimeStr = currDateTime.toString("yyyy-MM-dd");
      QString curTimeStr     = currDateTime.toString("hh:mm:ss");
//    QString fileDirPath    = filePath.append("/").append(curDateTimeStr).append(".txt");
    /*To determine if a file exists, you need to add a header to the file*/
    bool addFileHead = false;
    QFileInfo _info(curLogFileName);
    if(!_info.isFile())
        addFileHead = true;
    QString headTitle;
    headTitle=QString("time,distance,gndRssi0,gndRssi1,gndSnr,gndFreq,gndPower,airRssi0,airRssi1,airSnr,gndLDPCFailed,airLDPCFailed");

    QFile file(curLogFileName);
    file.open(QIODevice::ReadWrite|QIODevice::Append);
    QTextStream txtOutput(&file);
    if(addFileHead)
         txtOutput << headTitle << Qt::endl;
    txtOutput << curTimeStr.append(",").append(data) << Qt::endl;
    file.close();
}

void TaisyncRomoteHandler::runTask()
{
    workTimer.stop();
    if(!online())
    {
        disconnectToDev();
        setOnline(connectToDev());
    }
    else {
        getLocalInfo();
        getPeerInfo();
        getRfInfo();
        getLocalStatus();
        //headTitle=QString("distance  gndRssi0  gndRssi1  gndSnr  gndFreq gndPower airRssi0  airRssi1  airSnr  gndLDPCFailed airLDPCFailed");
        if (isStart) {
            QString logData=QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11")
            .arg(_gndDistance,8,10,QLatin1Char(' '))
                .arg(_gndRssi0,8,10,QLatin1Char(' '))
                .arg(_gndRssi1,8,10,QLatin1Char(' '))
                .arg(_gndSnr,6,10,QLatin1Char(' '))
                .arg(_gndFreq,7,10,QLatin1Char(' '))
                .arg(_gndPower,7,10,QLatin1Char(' '))
                .arg(_airRssi0,8,10,QLatin1Char(' '))
                .arg(_airRssi1,8,10,QLatin1Char(' '))
                .arg(_airSnr,6,10,QLatin1Char(' '))
                .arg(_gndLDPCFailed,13,10,QLatin1Char(' '))
                .arg(_airLDPCFailed,13,10,QLatin1Char(' '));

            saveLog(logData);
        }
        //qDebug() << "distance  gndRssi0  gndRssi1  gndSnr  gndFreq gndPower  airRssi0  airRssi1  airSnr";
        // for (int var = 0; var < 50; ++var) {
        //     QString data = QString("%1/%2/%3/%5/%6/%7/%8/%9/%10")
        //     .arg(_gndDistance)
        //         .arg(_gndRssi0)
        //         .arg(_gndRssi1)
        //         .arg(_gndSnr)
        //         .arg(_gndFreq)
        //         .arg(_gndPower)
        //         .arg(_airRssi0)
        //         .arg(_airRssi1)
        //         .arg(_airSnr)
        //         .arg(_gndFreq)
        //         .arg(_ethTxRate);
        // }
        // qDebug() << QString().append("QGC_DATA ").append(data);
    }
    workTimer.start(1000);
}

void TaisyncRomoteHandler::onAutoSaveChanged()
{
    Fact *saveDataFact = qgcApp()->toolbox()->settingsManager()->appSettings()->taisyncFlyDataSave();
    if (saveDataFact) {
        if (saveDataFact->rawValue().toBool()) {
            startSaveLog();
        } else {
            stopSaveLog();
        }
    }
    _onEnableChanged();
}

void TaisyncRomoteHandler::_onEnableChanged()
{
    Fact* taisyncFlyViewShowFact = qgcApp()->toolbox()->settingsManager()->appSettings()->taisyncFlyViewShow();
    Fact *saveDataFact = qgcApp()->toolbox()->settingsManager()->appSettings()->taisyncFlyDataSave();

    // true if anyone opened
    bool enable = taisyncFlyViewShowFact->rawValue().toBool() || saveDataFact->rawValue().toBool();
    if (enable && !workTimer.isActive()) {
        qDebug() << __FUNCTION__ << "start work";
        workTimer.start(1000);
    } else if (!enable && workTimer.isActive()) {
        workTimer.stop();
        qDebug() << __FUNCTION__ << "stop work";
    }
}
