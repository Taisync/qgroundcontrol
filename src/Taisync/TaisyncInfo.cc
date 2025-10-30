/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#include "TaisyncInfo.h"
#include <QNetworkProxy>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QDateTime>
#include "SettingsManager.h"
#include "AppSettings.h"

TaisyncInfo::TaisyncInfo()
{
    QHostAddress host = QHostAddress::AnyIPv4;
    udpSocket = new QUdpSocket(this);
    udpSocket->setProxy(QNetworkProxy::NoProxy);
    _connectStatus = udpSocket->bind(host, 16789, QAbstractSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    if (_connectStatus)
    {
        udpSocket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption,     64 * 1024);
        udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,  128 * 1024);

        QObject::connect(udpSocket, &QUdpSocket::readyRead, this, &TaisyncInfo::readBytes);
        qDebug () << "udp create success\n";
    }
    _logName.clear();

    Fact *saveDataFact = SettingsManager::instance()->appSettings()->taisyncFlyDataSave();
    if (saveDataFact) {
        connect(saveDataFact, &Fact::valueChanged, this, &TaisyncInfo::onAutoSaveChanged);
        onAutoSaveChanged();
    }
}

TaisyncInfo::~TaisyncInfo(void)
{
    if (udpSocket)
    {
        udpSocket->close();
    }
}

void TaisyncInfo::readBytes()
{
    if (!udpSocket)
    {
        return;
    }

    QByteArray databuffer;

    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;

        datagram.resize(udpSocket->pendingDatagramSize());

        QHostAddress    sender;
        quint16         senderPort;

        qint64 slen = udpSocket->readDatagram(datagram.data(),datagram.size(), &sender, &senderPort);
        if (slen == -1)
        {
            break;
        }

        databuffer.append(datagram);
        if (databuffer.size() > 10 * 1024)
        {
            databuffer.clear();
        }
        else
        {
            receiveParse(databuffer);
        }
    }
}

void TaisyncInfo::deleteOldLogs()
{
    // only save N log file max
    // if txt file num > N, delete all the history files
    static int maxFileNumber = 30;
    QDir dir(SettingsManager::instance()->appSettings()->linkLogSavePath());
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
            QString filePath = dir.absolutePath();
            filePath.append("/").append(fileName);
            qDebug() << "auto delete logfile:" << filePath;
            QFile::remove(filePath);
        }
    }
}

void TaisyncInfo::receiveParse(QByteArray b)
{
    QString logData;
    QJsonParseError errJson;
    QJsonDocument jsonData = QJsonDocument::fromJson(b, &errJson);

    if (errJson.error != QJsonParseError::NoError)
    {
        return;
    }
    else
    {
        QJsonObject _dataObj = jsonData.object();

        if (_dataObj.contains("pass_a"))
        {
            _airLDPCPass = _dataObj.value("pass_a").toString().toInt();
            emit airLDPCPassChanged();
        }

        if (_dataObj.contains("failed_a"))
        {
            _airLDPCFailed = _dataObj.value("failed_a").toString().toInt();
            emit airLDPCFailedChanged();
        }

        if (_dataObj.contains("snr_a"))
        {
            _airSNR = _dataObj.value("snr_a").toString().toInt();
            emit airSNRChanged();
        }

        if (_dataObj.contains("rssi1_a"))
        {
            _airRSSI0 = _dataObj.value("rssi1_a").toString().toInt();
            emit airRSSI0Changed();
        }

        if (_dataObj.contains("rssi2_a"))
        {
            _airRSSI1 = _dataObj.value("rssi2_a").toString().toInt();
            emit airRSSI1Changed();
        }

        if (_dataObj.contains("pass_g"))
        {
            _gndLDPCPass = _dataObj.value("pass_g").toString().toInt();
            emit gndLDPCPassChanged();
        }

        if (_dataObj.contains("failed_g"))
        {
            _gndLDPCFailed = _dataObj.value("failed_g").toString().toInt();
            emit gndLDPCFailedChanged();
        }

        if (_dataObj.contains("snr_g"))
        {
            _gndSNR = _dataObj.value("snr_g").toString().toInt();
            emit gndSNRChanged();
        }

        if (_dataObj.contains("rssi1_g"))
        {
            _gndRSSI0 = _dataObj.value("rssi1_g").toString().toInt();
            emit gndRSSI0Changed();
        }

        if (_dataObj.contains("rssi2_g"))
        {
            _gndRSSI1 = _dataObj.value("rssi2_g").toString().toInt();
            emit gndRSSI1Changed();
        }

        if (_dataObj.contains("distance"))
        {
            _range = _dataObj.value("distance").toString().toInt();
            emit rangeChanged();
        }

        if (_dataObj.contains("ethTx"))
        {
            _dataRate = _dataObj.value("ethTx").toString().toInt();
            emit dataRateChanged();
        }

        if (_dataObj.contains("lockCnt"))
        {
            _lockCnt = _dataObj.value("lockCnt").toString().toInt();
            emit lockCntChanged();
        }

        if (_dataObj.contains("freq_rx"))
        {
            _currFreq = _dataObj.value("freq_rx").toString();
            emit currFreqChanged();
        }

        if (_dataObj.contains("ant_a"))
        {
            _ant = _dataObj.value("ant_a").toString();
            emit antChanged();
        }

        if (_dataObj.contains("ant_g"))
        {
            _antGnd = _dataObj.value("ant_g").toString();
            emit antGndChanged();
        }

        if (_dataObj.contains("mcs"))
        {
            _mcs = _dataObj.value("mcs").toString();
            emit mcsChanged();
        }

        if (_dataObj.contains("role"))
        {
            QString role = _dataObj.value("role").toString();
            SettingsManager::instance()->appSettings()->setIsPilot(role != "1");
        } else {
            SettingsManager::instance()->appSettings()->setIsPilot(true);
        }

        savelogInfo(b);
        logData.clear();
    }
}

void TaisyncInfo::startLogSave()
{
    // make sure log files under num limit
    deleteOldLogs();

    QString filePath,currDateTimeStr,currTimeStr;

    filePath = SettingsManager::instance()->appSettings()->linkLogSavePath();
    QDateTime currTime = QDateTime::currentDateTime();
    currDateTimeStr = currTime.toString("yyyyMMdd");
    currTimeStr = currTime.toString("hhmmss");
    QDir dir(filePath);

    if (!dir.exists())
    {
        bool folder = dir.mkdir(filePath);
        if (!folder)
        {
            qDebug()<<"log folder create failed:" << filePath;
            return;
        }
    }

    _logName = filePath.append("/").append(currDateTimeStr).append(currTimeStr).append(".txt");

    QFileInfo _info(_logName);
    if (_info.isFile())
    {
        qDebug()<<"file is exist";
    }
    else
    {
        QFile file(_logName);
        (void) file.open(QIODevice::ReadWrite|QIODevice::Append);

        if (file.isOpen())
        {
            QTextStream txOutput(&file);
            txOutput <<"time gndRssi0 gndRssi1 gndSnr gndLDPCPass gndLDPCFailed airRssi0 airRssi1 airSnr airLDPCPass airLDPCFailed rang rate lockCnt freq ant antGnd mcs"<<Qt::endl;
            file.close();
        }
    }
}

void TaisyncInfo::stopLogSave()
{
    QFile file(_logName);
    if (file.isOpen())
    {
        file.close();
        qDebug()<<"close file :"<<_logName;
    }
    else
    {
        qDebug()<<"log name"<<_logName;
    }

    _logName.clear();
}

void TaisyncInfo::savelogInfo(const QByteArray& log)
{
    if (!_logName.isNull())
    {
        QFile file(_logName);
        (void) file.open(QIODevice::ReadWrite|QIODevice::Append);

        QDateTime   currDate = QDateTime::currentDateTime();
        QString currTimeStr = currDate.toString("hh:mm:ss");

        if (file.isOpen())
        {
            QTextStream txOutput(&file);
            txOutput <<currTimeStr.append(" ").append(log)<<Qt::endl;
            file.close();
            currTimeStr.clear();
        }
    }
}

void TaisyncInfo::onAutoSaveChanged()
{
    Fact *saveDataFact = SettingsManager::instance()->appSettings()->taisyncFlyDataSave();
    if (saveDataFact) {
        if (saveDataFact->rawValue().toBool()) {
            startLogSave();
        } else {
            stopLogSave();
        }
    }
}
