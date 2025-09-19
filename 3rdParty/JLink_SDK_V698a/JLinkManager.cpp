#include "JLinkManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QThread>

JLinkManager::JLinkManager(QObject *parent) : QObject(parent), _proc(this) {
    _proc.setProgram("JLink.exe");
    QObject::connect(&_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    QObject::connect(this, &JLinkManager::startScript, this, &JLinkManager::on_startScript);
    QObject::connect(this, &JLinkManager::establishConnection, this, &JLinkManager::on_establishConnection);
    clearErrorBuffer();
}

JLinkManager::~JLinkManager() {
    if (JLINKARM_IsOpen()) {
        JLINKARM_Close();
    }
}

void JLinkManager::clearErrorBuffer() { _errorBuffer = {256, '\0'}; }

void JLinkManager::setSN(const QString &serialNumber) { _SN = serialNumber; }

QString JLinkManager::getSN() const {
    JLINKARM_EMU_INFO jlink;

    int count = JLINKARM_EMU_GetNumDevices();
    qDebug() << count;

    for (int i = 0; i < count; i++) {
        JLINKARM_EMU_GetDeviceInfo(i, &jlink);
        qDebug() << jlink.USBAddr << jlink.SerialNo;
    }
    return _SN;
}
QByteArray JLinkManager::get_uid() const {
    char buf[1024];
    uint32_t ids[3];
    qDebug() << JLINKARM_ReadMemU32(0x0, 3, ids, (uint8_t *)buf);
    qDebug() << " status:" << buf << hex << ids[0] << ids[1] << ids[2];

    return QByteArray((const char *)ids, sizeof(ids));
}
bool JLinkManager::isConnected() const {
    if (_state == State::connectionTested) return true;

    return false;
}

void JLinkManager::selectByUSB() {
    if (JLINKARM_EMU_SelectByUSBSN(_SN.toUInt()) < 0) {
        _state = unknown;
        errorOut("No connection to JLink with S/N " + _SN);
    }
}

static void STDCALL _JLink_errorOutHandler(const char *text) { qCritical() << text; }

static void _JLinkARM_errorOutHandler(const char *text) { qCritical() << text; }

static void STDCALL _JLink_warnOutHandler(const char *text) { qWarning() << text; }

static void _JLinkARM_warnOutHandler(const char *text) { qWarning() << text; }

static int _JLink_hookUnsecureDialog(const char *sTitle, const char *sMsg, U32 Flags) {
    //    Q_UNUSED(sTitle);
    //    Q_UNUSED(sMsg);
    //    Q_UNUSED(Flags);
    qDebug() << sTitle << sMsg << Flags;
    return JLINK_DLG_BUTTON_YES;
}

void JLinkManager::open() {
    JLINK_SetErrorOutHandler(_JLink_errorOutHandler);
    JLINKARM_SetErrorOutHandler(_JLinkARM_errorOutHandler);

    JLINK_SetWarnOutHandler(_JLink_warnOutHandler);
    JLINKARM_SetWarnOutHandler(_JLinkARM_warnOutHandler);
    JLINK_SetHookUnsecureDialog(_JLink_hookUnsecureDialog);
    if (JLINKARM_Open())
        errorOut("JLINK: An error occured when opening JLink programmer.");
    else {
        JLINK_SetHookUnsecureDialog(_JLink_hookUnsecureDialog);
    }
}

void JLinkManager::setDevice(const QString &device) {
    clearErrorBuffer();
    char cmd[0x400];
    strcpy_s(cmd, "device = ");
    strcat_s(cmd, device.toLocal8Bit().data());
    JLINKARM_ExecCommand(cmd, _errorBuffer.data(), _errorBuffer.size());
    if (_errorBuffer.at(0) != 0) {
        errorOut("JLINK: " + _errorBuffer);
    }
}

void JLinkManager::select(int interface) {
    _targetInterface = interface;
    JLINKARM_TIF_Select(_targetInterface);
}

void JLinkManager::setSpeed(int speed) {
    _speed = speed;
    JLINKARM_SetSpeed(_speed);
}

void JLinkManager::connect() {
    if (JLINKARM_Connect()) {
        errorOut("JLINK: Could not connect to target.");
    }
}

int JLinkManager::erase() {
    int error = 0;

    error = JLINK_EraseChip();
    return error;
}

void JLinkManager::reset() { JLINKARM_Reset(); }

void JLinkManager::go() { JLINKARM_Go(); }

int JLinkManager::downloadFile(const QString &fileName, int adress) {
    int error = 0;
    JLINKARM_BeginDownload(0);  // Indicates start of flash download
    error = JLINK_DownloadFile(QString("bin/" + fileName).toLocal8Bit().data(),
                               adress);  // Load the application binary to address 0
    JLINKARM_EndDownload();
    return error;
}

void JLinkManager::close() { JLINKARM_Close(); }

void JLinkManager::on_establishConnection() {
    if (_SN.isEmpty()) {
        errorOut("No serial number for the JLink device provided.");
        return;
    }

    _state = waitingTestResponse;

    if (JLINKARM_EMU_SelectByUSBSN(_SN.toUInt()) < 0) {
        _state = unknown;
        errorOut("No connection to JLink with S/N " + _SN);
        logOut("No connection to JLink with S/N " + _SN);
    }

    else {
        _state = connectionTested;
        logOut("JLink with S/N: " + _SN + " connected");
    }
}

void JLinkManager::on_startScript(const QString &scriptFile) {
    _proc.setArguments({"-USB", _SN, "-CommanderScript", "Scripts/" + scriptFile});
    _proc.start();
    _proc.waitForStarted(2000);
}

void JLinkManager::readStandardOutput() {
    QByteArray data = _proc.readAllStandardOutput();

    data.replace('\0', ' ');
    QStringList lines = QString::fromLocal8Bit(data).split("\r\n");
    for (auto &line : lines) {
        if (line.size()) qDebug() << line;
    }
}
