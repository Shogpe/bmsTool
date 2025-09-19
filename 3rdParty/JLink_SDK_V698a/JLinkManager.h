#pragma once
#include <stdint.h>
#include <JLinkARMDLL.h>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include <QString>
class JLinkManager : public QObject {
    Q_OBJECT

   public:
    enum State { unknown, waitingTestResponse, connectionTested };

    explicit JLinkManager(QObject* parent = Q_NULLPTR);
    ~JLinkManager() Q_DECL_OVERRIDE;

    void setSN(const QString& serialNumber);
    QString getSN() const;
    QByteArray get_uid() const;
   public slots:

    State state() const { return _state; }
    bool isConnected() const;
    void selectByUSB();
    void open();
    void setDevice(const QString& device);
    void select(int interface = JLINKARM_TIF_SWD);
    void setSpeed(int speed);
    void connect();
    int erase();
    void reset();
    void go();
    int downloadFile(const QString& fileName, int adress);
    void close();

    void on_establishConnection();
    void on_startScript(const QString& scriptFile);
    void readStandardOutput();

   signals:

    void establishConnection();
    void startScript(const QString& scriptFile);

   private:
    void clearErrorBuffer();

    void logOut(QString log) { qDebug() << (log); }
    void errorOut(QString log) { qDebug() << (QString("JLINK ERROR: %1").arg(log)); }

    State _state = unknown;

    QString _device;
    int _targetInterface = JLINKARM_TIF_SWD;
    int _speed = 5000;
    int _hostInterface = JLINKARM_HOSTIF_USB;
    QString _SN;  // JLink serial number
    QByteArray _errorBuffer;

    QProcess _proc;
};
