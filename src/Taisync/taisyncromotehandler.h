#ifndef TAISYNCROMOTEHANDLER_H
#define TAISYNCROMOTEHANDLER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>

#define REMOTE_DEV ""
//#define TAISYNC_FLY_CAL

//////////////////////Remote ground command///////////////////////////
#define CBEG         "CBEG {\"cal\":\"start\"}\r\n"
#define ONLINE       "GREQ\r\n"
#define LOCAL_VERSION      "GDVR {\"source\":\"local\"}\r\n"
#define PEER_VERSION       "GDVR {\"source\":\"peer\"}\r\n"
#define LOCAL_INFO   "GLRS {\"source\":\"local\"}\r\n"
#define PEER_INFO    "GLRS {\"source\":\"peer\"}\r\n"
#define LOCAL_STATUS "GSTS\r\n"
#define BAND_INFO    "GBID\r\n"
#define FREQ_SCAN    "GRFS\r\n"  // Frequency scan

#define SET_BIND     "SBID\r\n"
#define RF_INFO      "GRFI\r\n"
#define RF_MODE      "SRFI {\"mode\":\"%1\"}\r\n"
#define RF_FREQ      "SRFI {\"mode\":\"manual\",\"freq\":\"%1\"}\r\n"
#define RF_AUTO      "SRFI {\"mode\":\"auto\",\"freq\":\"%1\"}\r\n"
#define RF_WREG      "SRFI {\"workRegion\":\"%1\"}\r\n"
#define SY_BAUDRATE  "SSCI {\"comBaudrate\":\"%1\"}\r\n"
#define RESET_STORE  "SSCI {\"recoverdefault\":\"enable\"}\r\n"

#define SET_MAX_POWER "SRFI {\"maxPower\":\"%1\"}\r\n"

class TaisyncRomoteHandler :public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(bool  online       READ online      WRITE   setOnline        NOTIFY onlineChanged)
    //Air info
    Q_PROPERTY(int  airRssi0        READ airRssi0                 WRITE   setAirRssi0              NOTIFY airRssi0Changed)
    Q_PROPERTY(int  airRssi1        READ airRssi1                 WRITE   setAirRssi1              NOTIFY airRssi1Changed)
    Q_PROPERTY(int  airSnr          READ airSnr                   WRITE   setAirSnr                NOTIFY airSnrChanged)
    Q_PROPERTY(int  airDistance     READ airDistance              WRITE   setAirDistance           NOTIFY airDistanceChanged)
    Q_PROPERTY(int  airLDPCFailed   READ airLDPCFailed            WRITE   setAirLDPCFailed         NOTIFY airLDPCFailedChanged)
    Q_PROPERTY(int  airLinkQuaity   READ airLinkQuaity            WRITE   setAirLinkQuaity         NOTIFY airLinkQuaityChanged)

    //Gnd info
    Q_PROPERTY(int  gndRssi0        READ gndRssi0                 WRITE   setGndRssi0              NOTIFY gndRssi0Changed)
    Q_PROPERTY(int  gndRssi1        READ gndRssi1                 WRITE   setGndRssi1              NOTIFY gndRssi1Changed)
    Q_PROPERTY(int  gndSnr          READ gndSnr                   WRITE   setGndSnr                NOTIFY gndSnrChanged)
    Q_PROPERTY(int  gndDistance     READ gndDistance              WRITE   setGndDistance           NOTIFY gndDistanceChanged)
    Q_PROPERTY(int  gndLDPCFailed   READ gndLDPCFailed            WRITE   setGndLDPCFailed         NOTIFY gndLDPCFailedChanged)
    Q_PROPERTY(int  gndLinkQuaity   READ gndLinkQuaity            WRITE   setGndLinkQuaity         NOTIFY gndLinkQuaityChanged)
    Q_PROPERTY(int  gndFreq         READ gndFreq                  WRITE   setGndFreq               NOTIFY gndFreqChanged)
    Q_PROPERTY(int  gndPower        READ gndPower                 WRITE   setGndPower              NOTIFY gndPowerChanged)
    Q_PROPERTY(quint64  ethTxRate   READ ethTxRate                WRITE   setEthTxRate             NOTIFY ethTxRateChanged)

    // fly view param
    Q_PROPERTY(bool showFlyParam READ showFlyParam CONSTANT)

    TaisyncRomoteHandler();
    ~TaisyncRomoteHandler();
    bool  online()  const {return _online;}
    int   setOnline(const bool &a);

    int   airRssi0() const {return _airRssi0;}
    int   setAirRssi0(const int &a);

    int   airRssi1() const {return _airRssi1;}
    int   setAirRssi1(const int &a);

    int   airSnr() const {return _airSnr;}
    int   setAirSnr(const int &a);

    int   airDistance() const {return _airDistance;}
    int   setAirDistance(const int &a);

    int   airLDPCFailed() const {return _airLDPCFailed;}
    int   setAirLDPCFailed(const int &a);

    int   airLinkQuaity() const {return _airLinkQuaity;}
    int   setAirLinkQuaity(const int &a);

    int   gndRssi0() const {return _gndRssi0;}
    int   setGndRssi0(const int &a);

    int   gndRssi1() const {return _gndRssi1;}
    int   setGndRssi1(const int &a);

    int   gndSnr() const {return _gndSnr;}
    int   setGndSnr(const int &a);

    int   gndDistance() const {return _gndDistance;}
    int   setGndDistance(const int &a);

    int   gndLDPCFailed() const {return _gndLDPCFailed;}
    int   setGndLDPCFailed(const int &a);

    int   gndLinkQuaity() const {return _gndLinkQuaity;}
    int   setGndLinkQuaity(const int &a);

    int   gndFreq() const {return _gndFreq;}
    int   setGndFreq(const int &a);

    int   gndPower() const {return _gndPower;}
    int   setGndPower(const int &a);

    quint64  ethTxRate() const {return _ethTxRate;}
    int      setEthTxRate(const quint64&a);

    bool connectToDev();
    void disconnectToDev();
    bool  ttysWrite(QByteArray data);
    QByteArray ttysRead();
    bool ttysReadDone(QByteArray &readData,int timeMs);
    bool getLocalInfo();
    bool getLocalStatus();
    bool getPeerInfo();
    bool getRfInfo();

    bool showFlyParam() const {
#ifdef TAISYNC_FLY_CAL
        return true;
#else
        return false;
#endif
    }

    Q_INVOKABLE void startSaveLog();
    Q_INVOKABLE void stopSaveLog();

    quint64 getLogsSize(const QString);
    void saveLog(QString data);
signals:
    void onlineChanged();
    void airRssi0Changed();
    void airRssi1Changed();
    void airSnrChanged();
    void airDistanceChanged();
    void airLDPCFailedChanged();
    void airLinkQuaityChanged();

    //Gnd info
    void gndRssi0Changed();
    void gndRssi1Changed();
    void gndSnrChanged();
    void gndDistanceChanged();
    void gndLDPCFailedChanged();
    void gndLinkQuaityChanged();
    void gndFreqChanged();
    void gndPowerChanged();
    void ethTxRateChanged();
public slots:
    void runTask();
private slots:
    void onAutoSaveChanged(void);
    void _onEnableChanged();
private:
    void autoClearLog();
    bool _online = false;
    int  _airRssi0 = 0;
    int  _airRssi1 = 0;
    int  _airSnr   = 0;
    int  _airDistance = 0;
    int  _airLDPCFailed = 0;
    int  _airLinkQuaity = 0;

    int  _gndRssi0 = 0;
    int  _gndRssi1 = 0;
    int  _gndSnr   = 0;
    int  _gndDistance = 0;
    int  _gndLDPCFailed = 0;
    int  _gndLinkQuaity = 0;
    int  _gndFreq  = 0;
    int  _gndPower  = 0;
    quint64  _ethTxRate= 0;
    QTimer workTimer;

    int fd;
    fd_set rd;
    fd_set wd;
    bool readDone = false;
    QString curLogFileName;
    bool    isStart = false;
};

#endif // TAISYNCROMOTEHANDLER_H
