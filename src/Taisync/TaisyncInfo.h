/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#ifndef __TAISYNCINFO_H__
#define __TAISYNCINFO_H__
#include <QObject>
#include <QUdpSocket>

class TaisyncInfo : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(bool online          READ online             NOTIFY  onlineChanged)

    Q_PROPERTY(int airRSSI0         READ airRSSI0           NOTIFY airRSSI0Changed)
    Q_PROPERTY(int airRSSI1         READ airRSSI1           NOTIFY airRSSI1Changed)
    Q_PROPERTY(int airSNR           READ airSNR             NOTIFY airSNRChanged)
    Q_PROPERTY(int airLDPCPass      READ airLDPCPass        NOTIFY airLDPCPassChanged)
    Q_PROPERTY(int airLDPCFailed    READ airLDPCFailed      NOTIFY airLDPCFailedChanged)
    Q_PROPERTY(int gndRSSI0         READ gndRSSI0           NOTIFY gndRSSI0Changed)
    Q_PROPERTY(int gndRSSI1         READ gndRSSI1           NOTIFY gndRSSI1Changed)
    Q_PROPERTY(int gndSNR           READ gndSNR             NOTIFY gndSNRChanged)
    Q_PROPERTY(int gndLDPCPass      READ gndLDPCPass        NOTIFY gndLDPCPassChanged)
    Q_PROPERTY(int gndLDPCFailed    READ gndLDPCFailed      NOTIFY gndLDPCFailedChanged)
    Q_PROPERTY(int range            READ range              NOTIFY rangeChanged)
    Q_PROPERTY(int dataRate         READ dataRate           NOTIFY dataRateChanged)
    Q_PROPERTY(QString currFreq     READ currFreq           NOTIFY currFreqChanged)
    Q_PROPERTY(QString ant          READ ant                NOTIFY antChanged)
    Q_PROPERTY(QString antGnd       READ antGnd             NOTIFY antGndChanged)
    Q_PROPERTY(QString mcs          READ mcs                NOTIFY mcsChanged)
    Q_PROPERTY(int lockCnt          READ lockCnt            NOTIFY lockCntChanged)

    Q_PROPERTY(QVariantList noiseTitles          READ noiseTitles            NOTIFY noiseChanged)
    Q_PROPERTY(QVariantList noiseValuesA          READ noiseValuesA            NOTIFY noiseChanged)
    Q_PROPERTY(QVariantList noiseValuesG          READ noiseValuesG            NOTIFY noiseChanged)

    TaisyncInfo();
    ~TaisyncInfo();

    void receiveParse(QByteArray b);

    bool online() const {return _online;}

    int airRSSI0() const {return _airRSSI0;}
    int airRSSI1() const {return _airRSSI1;}
    int airSNR() const {return _airSNR;}
    int airLDPCPass() const {return _airLDPCPass;}
    int airLDPCFailed() const {return _airLDPCFailed;}

    int gndRSSI0() const {return _gndRSSI0;}
    int gndRSSI1() const {return _gndRSSI1;}
    int gndSNR() const {return _gndSNR;}
    int gndLDPCPass() const {return _gndLDPCPass;}
    int gndLDPCFailed() const {return _gndLDPCFailed;}
    int range() const {return _range;}
    int dataRate() const {return _dataRate;}
    int lockCnt() const {return _lockCnt;}
    QString currFreq() {return _currFreq;}
    QString ant() {return _ant;}
    QString antGnd() {return _antGnd;}
    QString mcs() {return _mcs;}

    QVariantList noiseTitles() {return _noiseTitles;}
    QVariantList noiseValuesA() {return _noiseValuesA;}
    QVariantList noiseValuesG() {return _noiseValuesG;}

    void savelogInfo(const QByteArray& log);

    Q_INVOKABLE void startLogSave();
    Q_INVOKABLE void stopLogSave();

public slots:
    void readBytes(void);

private slots:
    void onAutoSaveChanged(void);

signals:
    void onlineChanged();
    void airRSSI0Changed();
    void airRSSI1Changed();
    void airSNRChanged();
    void airLDPCPassChanged();
    void airLDPCFailedChanged();
    void gndRSSI0Changed();
    void gndRSSI1Changed();
    void gndSNRChanged();
    void gndLDPCPassChanged();
    void gndLDPCFailedChanged();
    void rangeChanged();
    void dataRateChanged();
    void lockCntChanged();
    void currFreqChanged();
    void antChanged();
    void antGndChanged();
    void mcsChanged();
    void noiseChanged();

private:

    // auto delete old logs's file
    void deleteOldLogs();

    QUdpSocket  *udpSocket;
    bool        _connectStatus;
    bool        _online = 0;
    int         _airRSSI0 = 100;
    int         _airRSSI1 = 100;
    int         _airSNR = 0;
    int         _airLDPCPass = 0;
    int         _airLDPCFailed = 0;
    int         _gndRSSI0 = 100;
    int         _gndRSSI1 = 100;
    int         _gndSNR = 0;
    int         _gndLDPCPass = 0;
    int         _gndLDPCFailed = 0;
    int         _range = 0;
    int         _dataRate = 0;
    int         _lockCnt = 0;
    QString     _currFreq;
    QString     _ant;
    QString     _antGnd;
    QString     _mcs;
    QString     _logName;

    QVariantList _noiseTitles;
    QVariantList _noiseValuesA;
    QVariantList _noiseValuesG;
};
#endif
