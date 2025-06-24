#include "mb_cmu.h"
#include <QDebug>
#include <QJsonObject>
#include <QTimerEvent>
#include "myhelper.h"
#include "node_conf.h"
#include "utils.h"
#include <QElapsedTimer>

static uint16_t sec_cmd[10] = {0x1223, 0x3445, 0x5667, 0x7889, 0x9000U, 0x1122, 0x3344, 0x5566, 0x7788};
const QString recPath = "Rec";
const QString dataPath = "Data";
const QString errLogDataPath = "ErrLog";


#define EN_TRY_RECV_INFO        1

#define REG_DIFF_REFUSE_TO_INSERT       (10U)
mb_cmu::mb_cmu(BMS_PROTOCOL ver) : QObject(nullptr) {
    cmu = nullptr;
    csvfile = nullptr;
    csvfile_errLog = nullptr;
    stopDump = true;
    stopDumpErrLog = true;
    drv_status = 0;
    stop = false;
    mb_ip = "192.168.1.0";
    mb_port = 502;
    m_interval = 500;
    protocal_ver = ver;
    isDirExist(recPath);
    isDirExist(dataPath);
    isDirExist(errLogDataPath);
    //    pMq = MessageQueue::getInstance();
    //    pMq->registMsgQueue(0);
    cproVerList.clear();
    cproVerList << CMU_P_V0_0_01 << CMU_P_V1_0_02 << CMU_P_V2_0_03 << CMU_P_V3_0_04
                << CMU_A_FAN_MOS_V1_0_00 << CMU_A_FAN_MOS_V1_0_01 << CMU_A_FAN_MOS_V1_0_02
                << CMU_A_FAN_MOS_V1_0_03 << CMU_A_FAN_MOS_V1_0_04 << CMU_A_FAN_MOS_V1_3_00
                << CMU_A_FAN_PAL_V2_0_00
                << CMU_A_LIQ_MOS_V3_0_01 << CMU_A_LIQ_MOS_V3_0_02 << CMU_A_LIQ_MOS_V3_3_00;

    qRegisterMetaType<ST_SOE>("ST_SOE");
    qRegisterMetaType<TMsgData>("TMsgData");
    m_thread = new QThread();
    moveToThread(m_thread);
    m_thread->start();

    statusTimeInMs.start();//测试用
    Init();
}
QString mb_cmu::GetBitStatus(uint16_t status) {
    QStringList statusList;
    if ((((status >> 0) & 0x01) > 0)) statusList << "1";
    if ((((status >> 1) & 0x01) > 0)) statusList << "2";
    if ((((status >> 2) & 0x01) > 0)) statusList << "3";
    if ((((status >> 3) & 0x01) > 0)) statusList << "4";
    if ((((status >> 4) & 0x01) > 0)) statusList << "5";
    if ((((status >> 5) & 0x01) > 0)) statusList << "6";
    if ((((status >> 6) & 0x01) > 0)) statusList << "7";
    if ((((status >> 7) & 0x01) > 0)) statusList << "8";
    if ((((status >> 8) & 0x01) > 0)) statusList << "9";
    if ((((status >> 9) & 0x01) > 0)) statusList << "10";
    if ((((status >> 10) & 0x01) > 0)) statusList << "11";
    if ((((status >> 11) & 0x01) > 0)) statusList << "12";
    if ((((status >> 12) & 0x01) > 0)) statusList << "13";
    if ((((status >> 13) & 0x01) > 0)) statusList << "14";
    if ((((status >> 14) & 0x01) > 0)) statusList << "15";
    if ((((status >> 15) & 0x01) > 0)) statusList << "16";
    //    if (statusList.size() > 0) statusList.insert(0, QString::number(status, 16));
    return statusList.join("|");
}

QString mb_cmu::GetBitStatus(uint64_t status)
{
    QStringList statusList;
    for (int i = 0; i < 64; ++i) {
        if((status>>i)&0x01){
            statusList << QString("%1").arg(i+1);
        }
    }
    return statusList.join("|");
}
// 0:停止均衡;0x55:强制;0xAA:自动;0x88:手动

enum BALANCE_MODE {
    BALANCE_STOP = 0x0,
    BALANCE_FORCE = 0x55,
    BALANCE_MANUAL = 0x88,
    BALANCE_AUTO = 0xAA,
};
QString mb_cmu::GetBalanceValue(uint16_t status) {
    int mode = (status & 0xFF);
    double Ib = (int8_t)((status >> 8) & 0xFF);
    Ib *= 0.1;
    switch (mode) {
        case BALANCE_STOP:
            return "STOP";
            break;
        case BALANCE_FORCE:
            return QString("FORCE:%1 A").arg(QString::number(Ib));
            break;
        case BALANCE_AUTO:
            return QString("AUTO:%1 A").arg(QString::number(Ib));
            break;
        case BALANCE_MANUAL:
            return QString("MANUAL:%1 A").arg(QString::number(Ib));
            break;
        default:
            break;
    }
    return QString("ERR:%1").arg(QString::number(mode));
}

QString mb_cmu::GetBalanceValue(uint16_t status, int16_t cur)
{
    uint16_t mode = status;
    double Ib = cur;
    Ib *= 0.001;
    switch (mode) {
        case BALANCE_STOP:
            return "STOP";
            break;
        case BALANCE_FORCE:
            return QString("FORCE:%1 A").arg(QString::number(Ib));
            break;
        case BALANCE_AUTO:
            return QString("AUTO:%1 A").arg(QString::number(Ib));
            break;
        case BALANCE_MANUAL:
            return QString("MANUAL:%1 A").arg(QString::number(Ib));
            break;
        default:
            break;
    }
    return QString("ERR:%1").arg(QString::number(mode));
}

QString mb_cmu::GetBalanceValue(uint64_t status, int16_t *cur)
{
    QStringList statusList = {"","","",""};
    QString str;
    double Cur;

    for (int i = 0; i < config.vol_num; ++i) {
        if((status>>i)&0x01){
            int index = i/(config.vol_num/4);
            Cur = cur[index] * 0.001;
            str = tr("CH:%1 Curr:").arg(i+1) + QString::number(Cur,'f',1) + "A";
            statusList[index] = str;
        }
    }
    return statusList.join("|");
}
void mb_cmu::Dump2CsvTitle() {
    if (stopDump) return;
    if ((rec & CSV) != CSV) return;
    fileTime = QDateTime::currentDateTime();
    QString currentip = (QString::fromStdString(mb_ip).split('.'))[3];
    QString fileName = currentip +"_"+ fileTime.toString("yyyyMMdd_hhmmss");
    fileName.append(".csv");
    if (csvfile) csvfile->close();
    csvfile = new QFile(dataPath + "/" + fileName);
    if (!csvfile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        delete csvfile;
        csvfile = nullptr;
        qDebug() << "Cannot open file for writing: " << qPrintable(csvfile->errorString());
        return;
    }
    QTextStream data_buf(csvfile);
    // 写入UTF-BOM头部
    data_buf << QChar(0xfeff);
    data_buf << "Time,";
    for (int i = 0; i < this->nodes_table.size(); i++) {
        if (this->nodes_table.at(i).val_type == 128) {
            data_buf << (this->nodes_table.at(i).node_name) << ",";
        }
    }
    for (int i = 0; i < config.bmu_num; i++) {
        if(is_pVer_a_fan_pal()){
            data_buf << (tr("BMU%1_模块温度1,").arg(i + 1));
            data_buf << (tr("BMU%1_模块温度2,").arg(i + 1));
        }
        for (int j = 0; j < config.vol_num; ++j) {
            data_buf << (QString("BMU%1_U%2,").arg(i + 1).arg(j + 1));
        }
        for (int j = 0; j < config.T_num; j++) {
            data_buf << (QString("BMU%1_T%2,").arg(i + 1).arg(j + 1));
        }
        for (int j = 0; j < config.Tp_num; j++) {
            data_buf << (tr("BMU%1_Tp%2,").arg(i + 1).arg(j + 1));
        }
        data_buf << (tr("BMU%1_电压断线,").arg(i + 1));
        data_buf << (tr("BMU%1_温度断线,").arg(i + 1));
        data_buf << (tr("BMU%1_运行状态,").arg(i + 1));
        data_buf << (tr("BMU%1_故障状态,").arg(i + 1));
        if(is_cpVer_match(CMU_A_FAN_MOS_V1_0_03)) {
            data_buf << (tr("BMU%1_风机转速,").arg(i + 1));
        }
        if (is_cpVer_Higher_than(CMU_P_V1_0_02)) {
            data_buf << (tr("BMU%1_CAN错误,").arg(i + 1));
        }
        if (is_cpVer_Higher_than(CMU_P_V2_0_03)) {
            data_buf << (tr("BMU%1_母线电压,").arg(i + 1));
            if(is_pVer_a_liq_mos()){
                data_buf << (tr("BMU%1_模组A均衡电流,").arg(i + 1));
                data_buf << (tr("BMU%1_模组B均衡电流,").arg(i + 1));
                data_buf << (tr("BMU%1_模组C均衡电流,").arg(i + 1));
                data_buf << (tr("BMU%1_模组D均衡电流,").arg(i + 1));
            }else{
                data_buf << (tr("BMU%1_均衡电流,").arg(i + 1));
            }
            data_buf << (tr("BMU%1_均衡故障,").arg(i + 1));
            data_buf << (tr("BMU%1_通道状态,").arg(i + 1));
            data_buf << (tr("BMU%1_均衡模式,").arg(i + 1));
            for (int j = 0; j < config.vol_num; ++j) {
                data_buf << (tr("BMU%1_%2充电Ah,").arg(i + 1).arg(j + 1));
                data_buf << (tr("BMU%1_%2放电Ah,").arg(i + 1).arg(j + 1));
            }
        }
    }

    data_buf << endl;
}
#define FILE_ROTATE_TIME 60 * 60 * 12
void mb_cmu::Dump2Csv() {
    if (stopDump) return;
    if (rec & 0x01) {
        if (!csvfile) {
            Dump2CsvTitle();
        } else {
            // 检查csv文件以便分割文件 12小时
            if (fileTime.secsTo(QDateTime::currentDateTime()) >= FILE_ROTATE_TIME) {
                Dump2CsvTitle();
            }
        }
        QTextStream data_buf(csvfile);
        data_buf << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << ",";
        for (int i = 0; i < this->nodes_table.size(); i++) {
            if (this->nodes_table.at(i).val_type == 128) {
                data_buf << QString::number(mapData.value(this->nodes_table.at(i).node_name, 0), 'g', 15) << ",";
            }
        }
        for (int i = 0; i < config.bmu_num; i++) {
            if(is_pVer_a_fan_pal()){
                double val = this->bmu_data[i].ModT1 / 10.0;
                data_buf << (QString("%1,").arg(val));
                val = this->bmu_data[i].ModT2 / 10.0;
                data_buf << (QString("%1,").arg(val));
            }

            for (int j = 0; j < config.vol_num; j++) {
                double val = this->bmu_data[i].Ucell[j] / 10000.0;
                data_buf << (QString("%1,").arg(val));
            }
            for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
                double val = this->bmu_data[i].Tcell[j] / 10.0;
                data_buf << (QString("%1,").arg(val));
            }

            // 20220115添加
            double val = 0;
            uint64_t u64val = 0;
            // 状态量个数，电压断线+温度断线+运行状态+故障状态
            if (is_pVer_a_fan_pal()){
                val = this->bmu_data[i].Ubreak;
            }else{
                val = this->bmu_data[i].U64break;
            }
            u64val = val;
            data_buf << (QString("0x%1,").arg(u64val,0,16));


            if (is_pVer_a_fan_pal()){
                val = this->bmu_data[i].Tbreak;
            }else{
                val = this->bmu_data[i].T64break;
            }
            u64val = val;
            data_buf << (QString("0x%1,").arg(u64val,0,16));

            val = this->bmu_data[i].RunStat;
            u64val = val;
            data_buf << (QString("0x%1,").arg(u64val,4,16,QChar('0')));
            val = this->bmu_data[i].ErrStat;
            u64val = val;
            data_buf << (QString("0x%1,").arg(u64val,4,16,QChar('0')));
            if(is_cpVer_match(CMU_A_FAN_MOS_V1_0_03)) {
                data_buf << (this->bmu_data[i].FanSpeed) << ",";
            }
            if (is_cpVer_Higher_than(CMU_P_V1_0_02)) {
                data_buf << (this->bmu_data[i].CanErr) << ",";
            }
            if (is_pVer_active()) {
                if(is_pVer_a_liq_mos())
                {
                    val = this->bmu_data[i].BalU24 / 1000.0;
                    data_buf << (QString("%1,").arg(val));

                    val = this->bmu_data[i].BalIdc[0] / 1000.0;
                    data_buf << (QString("%1,").arg(val));
                    val = this->bmu_data[i].BalIdc[1] / 1000.0;
                    data_buf << (QString("%1,").arg(val));
                    val = this->bmu_data[i].BalIdc[2] / 1000.0;
                    data_buf << (QString("%1,").arg(val));
                    val = this->bmu_data[i].BalIdc[3] / 1000.0;
                    data_buf << (QString("%1,").arg(val));

                    data_buf << GetBitStatus(this->bmu_data[i].U64BalErr) << ",";
                    data_buf << GetBitStatus(this->bmu_data[i].U64BalStat) << ",";
                    data_buf << GetBalanceValue(this->bmu_data[i].BalMode,this->bmu_data[i].BalCur) << ",";
                    for (int j = 0; j < config.vol_num; j++) {
                        data_buf << (this->bmu_data[i].BalChgAh[j]) << ",";
                        data_buf << (this->bmu_data[i].BalDischgAh[j]) << ",";
                    }
                }
                else
                {
                    val = this->bmu_data[i].BalU24 / 1000.0;
                    data_buf << (QString("%1,").arg(val));
                    val = this->bmu_data[i].BalIdc[0] / 1000.0;
                    data_buf << (QString("%1,").arg(val));
                    data_buf << GetBitStatus(this->bmu_data[i].BalErr) << ",";
                    data_buf << GetBitStatus(this->bmu_data[i].BalStat) << ",";
                    data_buf << GetBalanceValue(this->bmu_data[i].BalMode) << ",";
                    for (int j = 0; j < config.vol_num; j++) {
                        data_buf << (this->bmu_data[i].BalChgAh[j]) << ",";
                        data_buf << (this->bmu_data[i].BalDischgAh[j]) << ",";
                    }
                }
            }
        }
        data_buf << endl;
        csvfile->flush();
    }

    // dump bin文件
    if (rec & 0x02) {
        QJsonObject object;
        for (int i = 0; i < this->nodes_table.size(); i++) {
            if (this->nodes_table.at(i).val_type == 128) {
                object.insert(QString(this->nodes_table.at(i).node_name),
                              mapData.value(this->nodes_table.at(i).node_name, 0));
            }
        }
        double val = 0;
        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < config.vol_num; j++) {
                val = this->bmu_data[i].Ucell[j] / 10000.0;
                object.insert((QString("BMU%1_U%2").arg(i + 1).arg(j + 1)), val);
            }

            for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
                double val = this->bmu_data[i].Tcell[j] / 10.0;
                if (j < config.T_num) {
                    object.insert((QString("BMU%1_T%2").arg(i + 1).arg(j + 1)), val);
                } else {
                    object.insert((QString("BMU%1_Tp%2").arg(i + 1).arg(j + 1)), val);
                }
            }
            if (is_pVer_a_liq_mos()){
                val = this->bmu_data[i].U64break;
            }else{
                val = this->bmu_data[i].Ubreak;
            }
            object.insert((QString("BMU%1_Ubreak,").arg(i + 1)), val);
            if (is_pVer_a_liq_mos()){
                val = this->bmu_data[i].T64break;
            }else{
                val = this->bmu_data[i].Tbreak;
            }
            object.insert((QString("BMU%1_Tbreak,").arg(i + 1)), val);
            val = this->bmu_data[i].RunStat;
            object.insert((QString("BMU%1_Run,").arg(i + 1)), val);
            val = this->bmu_data[i].ErrStat;
            object.insert((QString("BMU%1_Err,").arg(i + 1)), val);

        }

        // 以读写方式打开主目录下的1.json文件，若该文件不存在则会自动创建
        QFile file(recPath + "/" + QDateTime::currentDateTime().toString("yyyyMMddThhmmss") + ".rec");
        if (!file.open(QIODevice::ReadWrite)) {
            qDebug() << "File open error";
        } /* else {
             qDebug() << "File open!";
         }*/
        // 使用QJsonDocument设置该json对象
        QJsonDocument jsonDoc;
        jsonDoc.setObject(object);
        // 将json以文本形式写入文件并关闭文件。
        QByteArray b = gzipCompress(jsonDoc.toBinaryData());
        file.write(b);
        file.close();
    }
}

void mb_cmu::DumpErrLog2CsvTitle()
{
    if (stopDumpErrLog) return;
    fileTime_errLog = QDateTime::currentDateTime();
    QString currentip = (QString::fromStdString(mb_ip).split('.'))[3];
    QString fileName = currentip +"_"+ fileTime_errLog.toString("yyyyMMdd_hhmmss");
    fileName.append(".csv");
    if (csvfile_errLog) csvfile_errLog->close();
    csvfile_errLog = new QFile(errLogDataPath + "/" + fileName);
    if (!csvfile_errLog->open(QIODevice::WriteOnly | QIODevice::Text)) {
        delete csvfile_errLog;
        csvfile_errLog = nullptr;
        qDebug() << "Cannot open file for writing: " << qPrintable(csvfile_errLog->errorString());
        return;
    }
    QTextStream data_buf(csvfile_errLog);
    // 写入UTF-BOM头部
    data_buf << QChar(0xfeff);
    data_buf << "Time,";
    data_buf << tr("故障ID,");
    data_buf << tr("运行状态,");
    data_buf << tr("故障状态,");
    data_buf << tr("电压采集异常通道及对应电压,");
    data_buf << tr("电池温度异常通道及对应温度,");
    data_buf << endl;
}

// id       : int      当前故障所在板子ID
// err_type : QString  当前故障类型
// val      : uint     当前故障值
bool mb_cmu::IsErrCanWrite(int id, QString err_type, uint val)
{
    uint *cnt = &(OldErrLogBufMap[id][err_type][val]);

    bool isWrite = false;
    (*cnt)++;

    if(logSaveMethod == ERRLOG_ONCE){
        if((*cnt) == 1){
            isWrite = true;            
        }else{
            (*cnt) = 1;
        }
    }else if(logSaveMethod == ERRLOG_ALWAYS){
        isWrite = true;
    }else if(logSaveMethod == ERRLOG_NTIMES){
        if((*cnt) >= errLogCount){
            isWrite = true;
            (*cnt) = 0;
        }
    }

    return isWrite;
}

void mb_cmu::DumpErrLog2Csv()
{
    if (stopDumpErrLog) return;
    if (!csvfile_errLog) {
        DumpErrLog2CsvTitle();
    } else {
        // 检查csv文件以便分割文件 12小时
        if (fileTime_errLog.secsTo(QDateTime::currentDateTime()) >= FILE_ROTATE_TIME) {
            DumpErrLog2CsvTitle();
        }
    }
    QTextStream data_buf(csvfile_errLog);
    QMap<QString,QString>errDataBufMap;
    QList<uint>chlAvgList;

    errDataBufMap.clear();
    chlAvgList.clear();
    bool errisexist = false;

    for (int i = 0; i < config.vol_num; ++i) {
        uint32_t sum = 0;

       std::sort(ChlCellMap[i].begin(),ChlCellMap[i].end());
       if(ChlCellMap[i].count()>4){
           ChlCellMap[i].removeFirst();
           ChlCellMap[i].removeFirst();
           ChlCellMap[i].removeLast();
           ChlCellMap[i].removeLast();
       }else if(ChlCellMap[i].count()>2){
           ChlCellMap[i].removeFirst();
           ChlCellMap[i].removeLast();
       }

       for (int j = 0; j < ChlCellMap[i].count(); ++j) {
           sum += ChlCellMap[i].at(j);
       }

       chlAvgList << sum / ChlCellMap[i].count();
    }

    for (int i = 0; i < config.bmu_num; i++) {
        // 获取故障时间
        errDataBufMap["Time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        // 获取故障ID
        errDataBufMap["ID"] = QString("%1").arg(i,2,10,QLatin1Char('0'));
        // 获取运行状态
        errDataBufMap["RunStat"] = QString("0x%1").arg(this->bmu_data[i].RunStat, 4, 16, QLatin1Char('0'));
        // 获取故障状态
        errDataBufMap["ErrStat"] = "";
        if(this->bmu_data[i].ErrStat !=0){
            if(IsErrCanWrite(i,"ErrStat",this->bmu_data[i].ErrStat)){
                errDataBufMap["ErrStat"] = QString("0x%1").arg(this->bmu_data[i].ErrStat, 4, 16, QLatin1Char('0'));
            }
        }

        // 获取故障电压
        errDataBufMap["ErrUcell"] = "";
        for (int k = 0; k < config.vol_num; ++k) {
            if( (abs((int)(this->bmu_data[i].Ucell[k] - chlAvgList[k])) >= errLogUcellLimitValue ) ||
                (abs((int)(chlAvgList[k] - 33000)) >= errLogStdLimitValue)                         ){
                if(IsErrCanWrite(i,"ErrUcell",k)){
                    errDataBufMap["ErrUcell"] += QString("%1[%2]  ").arg(k+1,2,10,QLatin1Char('0')).arg(this->bmu_data[i].Ucell[k]/10000.0,5,'f', 3,'0');
                }
            }
        }

        // 获取故障温度
        errDataBufMap["ErrTcell"] = "";
        for (int k = 0; k < config.T_num+config.Tp_num; ++k) {
            if(abs(this->bmu_data[i].Tcell[k] - 250) >= errLogTempLimitValue){
                if(IsErrCanWrite(i,"ErrTcell", k)){
                    if(k>=config.T_num){
                        errDataBufMap["ErrTcell"] += QString("P%1[%2]  ").arg(k+1-config.T_num,2,10,QLatin1Char('0')).arg(this->bmu_data[i].Tcell[k] / 10.0,6,'f',1,' ');
                    }else{
                        errDataBufMap["ErrTcell"] += QString("T%1[%2]  ").arg(k+1,2,10,QLatin1Char('0')).arg(this->bmu_data[i].Tcell[k] / 10.0,6,'f',1,' ');
                    }
                }
            }
        }

        if( errDataBufMap["ErrStat"]  != "" || errDataBufMap["ErrUcell"] != "" || errDataBufMap["ErrTcell"] != "" ){
            data_buf<<errDataBufMap["Time"]<<","
                    <<errDataBufMap["ID"]<<","
                    <<errDataBufMap["RunStat"]<<","
                    <<errDataBufMap["ErrStat"]<<","
                    <<errDataBufMap["ErrUcell"]<<","
                    <<errDataBufMap["ErrTcell"]<<",";
            data_buf << endl;
        }
    }
    csvfile_errLog->flush();
}

mb_cmu::~mb_cmu() {
    if (this->cmu) this->Close();
    if (csvfile) csvfile->close();
    csvfile = nullptr;
    if (csvfile_errLog) csvfile_errLog->close();
    csvfile_errLog = nullptr;
    stop = true;
    m_thread->quit();
    m_thread->wait();
    m_thread->deleteLater();
}

int mb_cmu::Close() {
    modbus_close(this->cmu);
    modbus_free(this->cmu);
    this->cmu = nullptr;
    return 0;
}
NodeReg mb_cmu::GetNodeAddr(QString name) {
    QMutexLocker locker(&mutex);
    return mapConfig.value(name, NodeReg{});
}
int mb_cmu::Init() {
    QMutexLocker locker(&mutex);
    qDebug() << "init config";
    NodeReg node_reg_tmp;
    reg_list_.clear();
    wr_list_.clear();
    int index = -1;
    config = {0, 0, 0, 0, 0};
    memset(&sys_para, 0, sizeof(sys_para));
    if(cproVerList.contains(compound_protocol_ver())&&exVerChangedFlag)
    {
        db_manager::Instance()->getExternNode(this->nodes_table, protocal_ver, ex_ver);
    }
    else
    {
        clearExVer();
        db_manager::Instance()->getOriginNode(this->nodes_table, protocal_ver);
    }
    exVerChangedFlag = false;

    qDebug()<<"nodes_table.size:"<< this->nodes_table.size();
    mapData.clear();
    mapConfig.clear();
    for (int i = 0; i < this->nodes_table.size(); i++) {
        node_reg_tmp.default_val = 0;
        if (nodes_table.at(i).reg_type > NONE_REG) {
            node_reg_tmp.reg_type = nodes_table.at(i).reg_type;
            node_reg_tmp.reg_addr = nodes_table.at(i).reg_addr;
            node_reg_tmp.data_type = nodes_table.at(i).data_type;
            node_reg_tmp.index = i;
            node_reg_tmp.factor = nodes_table.at(i).factor;
            if ((index = JudgeReg(node_reg_tmp)) != -1) {
                InsertReg(node_reg_tmp, index);
            } else {
                NewReg(node_reg_tmp);
            }
        }
        mapData.insert(nodes_table.at(i).node_name, 0);
        mapConfig.insert(nodes_table.at(i).node_name, node_reg_tmp);
    }
    emit bmsDataReady(1, mapData);
    emit signal_message("init complete.");
    return 0;
}


#define MAX_LEN 120
/*
 * 读取数据
 **/
int mb_cmu::ReadData(uint8_t type, int start_addr, int reg_num, uint16_t* dest) {
    int status = 0;
    int read_len = 0;
    int test_start_addr = start_addr;
    int test_reg_num = reg_num;
    int rc = 0;
#if EN_TRY_RECV_INFO
    QElapsedTimer readOneTimeCostMs;
    readOneTimeCostMs.start();
#endif
    memset(dest, 0, reg_num * sizeof(uint16_t));
    if (!reg_num) return status;
    switch (type) {
        case MODBUS_FC_READ_HOLDING_REGISTERS: {
            do {
                read_len = reg_num > MAX_LEN ? MAX_LEN : reg_num;
                rc = modbus_read_registers(this->cmu, start_addr, read_len, dest);
                if (rc > 0) {
                    status += rc;
//                    qWarning() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc << "√";
                } else {
                    //qWarning() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc << "X";
                }
                reg_num -= read_len;
                dest += read_len;
                start_addr += read_len;
            } while (reg_num);
        } break;
        case MODBUS_FC_READ_INPUT_REGISTERS: {
            do {
                read_len = reg_num > MAX_LEN ? MAX_LEN : reg_num;
                rc = modbus_read_input_registers(this->cmu, start_addr, read_len, dest);
                if (rc > 0) {
                    status += rc;
//                    qWarning() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc << "√";
                } else {
                    //qWarning() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc << "X";
                }
                reg_num -= read_len;
                dest += read_len;
                start_addr += read_len;
            } while (reg_num);
        } break;
        default:
            break;
    }
    if (status <= 0) {
        drv_status &= ~(0x01U << CMU_ONLINE);
        err_counter++;
    } else {
        err_counter = 0;
        drv_status |= (0x01 << CMU_ONLINE);
    }

#if EN_TRY_RECV_INFO
    QString res = "";
    if(status == test_reg_num){
        res = "-√-";
    }else{
        res = "-X-";
    }
    qInfo()<< "==<try to recv:"  << res
                                 << "Type:" << type
                                 << "Addr:" << test_start_addr
                                 << "Num:" << test_reg_num
                                 << "recv:" << status
                                 << "cost:" << readOneTimeCostMs.elapsed() << "ms";
#endif
    return status;
}
int mb_cmu::ReadCapData() {
    /* Read 5 registers from the address 0 */
    unsigned int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;
    if (config.bmu_num > 0) {
        // BMU 均衡电量
        if (is_pVer_a_fan_pal()) {
            reg_num = config.bmu_num * 2 * config.vol_num;
            status += ReadData(0x04, 0xA00 + config.bmu_num * 1, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                for (int j = 0; j < config.vol_num; j++) {
                    bmu_data[i].BalChgAh[j] = *(p + i * 2 * config.vol_num + 2 * j);
                    bmu_data[i].BalDischgAh[j] = *(p + i * 2 * config.vol_num + 2 * j + 1);
                }
            }
        } else {
            reg_num = config.bmu_num * 2 * config.vol_num;
            status += ReadData(0x03, 0xA00 + config.bmu_num * 1, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                for (int j = 0; j < config.vol_num; j++) {
                    bmu_data[i].BalChgAh[j] = *(p + i * 2 * config.vol_num + 2 * j);
                    bmu_data[i].BalDischgAh[j] = *(p + i * 2 * config.vol_num + 2 * j + 1);
                }
            }
        }
    }
    return status;
}
// CMU4.0主动均衡版本
#define STAT_NUM    4
#define BALANCE_NUM 5
int mb_cmu::ReadALL() {
    int status = 0;
    status += ReadCmuData();
    status += ReadBmuData();
    return status;
}


int mb_cmu::ReadCmuData() {

    unsigned int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;
    status += ReadAI();

    if (status > 0) emit bmsDataReady(0, mapData);

    return status;
}

int mb_cmu::ReadBmuData() {

    unsigned int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;

    // 版本号,优先读取,下文根据版本号采集不同地址
    reg_num = config.bmu_num * 2 + 2;
    status += ReadData(0x03, 0x500, reg_num, p);
    this->cmu_ver = *(uint32_t*)(p);
    for (int i = 0; i < config.bmu_num; i++) {
        bmu_data[i].Version = *(uint32_t*)(p + 2 + i * 2);
    }

    if (config.bmu_num > 0) {
        if(is_pVer_a_fan_pal())
        {
            uint16_t *starAddr = (uint16_t *)(0x500 + 2 + config.bmu_num * 2);
            uint16_t *pt;
            reg_num = config.bmu_num * 4;
            status += ReadData(0x03, (int)starAddr, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                // 读硬件版本号
                pt = p;
                bmu_data[i].HVersion = *(pt + i);
                // 读bmuboot版本号
                pt = p + config.bmu_num;
                bmu_data[i].BMUBootVersion  = (*(pt + i*2));
                bmu_data[i].BMUBootVersion |= *(pt + i*2+1)<<16;
                // 读bmu生产流水号
                pt = p + config.bmu_num*3;
                bmu_data[i].BMUSN = *(pt + i);
            }

            reg_num = config.bmu_num * 2;
            status += ReadData(0x04, 0x157C, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                // 读模块1温度
                bmu_data[i].ModT1 = *(p + i*2);
                // 读模块2温度
                bmu_data[i].ModT2 = *(p + i*2 + 1);
            }
        }else if(is_cpVer_a_fan_mos_with_boot_ver()){
            uint16_t *starAddr = (uint16_t *)(0x500 + 2 + config.bmu_num * 2);
            uint16_t *pt;
            reg_num = config.bmu_num * 3;
            status += ReadData(0x03, (int)starAddr, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                // 读硬件版本号
                pt = p;
                bmu_data[i].HVersion = *(pt + i);
                // 读bmuboot版本号
                pt = p + config.bmu_num;
                bmu_data[i].BMUBootVersion  = (*(pt + i*2));
                bmu_data[i].BMUBootVersion |= *(pt + i*2+1)<<16;
            }
        }else if(is_pVer_a_liq_mos()){
            uint16_t *starAddr = (uint16_t *)(0x500 + 2 + config.bmu_num * 2);
            uint16_t *pt;
            reg_num = config.bmu_num * 3;
            status += ReadData(0x03, (int)starAddr, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                // 读硬件版本号
                pt = p;
                bmu_data[i].HVersion = *(pt + i);
                // 读bmuboot版本号
                pt = p + config.bmu_num;
                bmu_data[i].BMUBootVersion  = (*(pt + i*2));
                bmu_data[i].BMUBootVersion |= *(pt + i*2+1)<<16;
            }
        }


        reg_num = config.bmu_num * config.vol_num;
        status += ReadData(0x04, 0x01, reg_num, p);
        int maxId = 0;
        int minId = 0;
        int maxBmuId = 0;
        int minBmuId = 0;
        ChlCellMap.clear();

        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < config.vol_num; j++) {
                bmu_data[i].Ucell[j] = *(p + i * config.vol_num + j);
                ChlCellMap[j]<<bmu_data[i].Ucell[j];
                if (bmu_data[i].Ucell[j] > bmu_data[i].Ucell[maxId]) maxId = j;
                if (bmu_data[i].Ucell[j] < bmu_data[i].Ucell[minId]) minId = j;
            }
            bmu_data[i].MaxUcellId = maxId;
            bmu_data[i].MinUcellId = minId;
            if (bmu_data[maxBmuId].Ucell[bmu_data[maxBmuId].MaxUcellId] < bmu_data[i].Ucell[maxId]) {
                maxBmuId = i;
            }
            if (bmu_data[minBmuId].Ucell[bmu_data[minBmuId].MinUcellId] > bmu_data[i].Ucell[minId]) {
                minBmuId = i;
            }
        }
        bms_data.MaxUbmuId = maxBmuId;
        bms_data.MinUbmuId = minBmuId;
        // 温度
        reg_num = config.bmu_num * (config.T_num + config.Tp_num);
        status += ReadData(0x04, 0x1000, reg_num, p);
        maxId = 0;
        minId = 0;
        maxBmuId = 0;
        minBmuId = 0;
        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
                if (j < (config.T_num)) {
                    if (bmu_data[i].Tcell[j] > bmu_data[i].Tcell[maxId]) maxId = j;
                    if (bmu_data[i].Tcell[j] < bmu_data[i].Tcell[minId]) minId = j;
                }
                bmu_data[i].Tcell[j] = *(p + i * (config.T_num + config.Tp_num) + j);
            }
            bmu_data[i].MaxTcellId = maxId;
            bmu_data[i].MinTcellId = minId;
            if (bmu_data[maxBmuId].Tcell[bmu_data[maxBmuId].MaxTcellId] < bmu_data[i].Tcell[maxId]) maxBmuId = i;
            if (bmu_data[minBmuId].Tcell[bmu_data[minBmuId].MinTcellId] > bmu_data[i].Tcell[minId]) minBmuId = i;
        }
        bms_data.MaxTbmuId = maxBmuId;
        bms_data.MinTbmuId = minBmuId;
        // 状态
        if(is_pVer_a_liq_mos()){
            uint16_t *pt;
            reg_num = config.bmu_num * 10;
            status += ReadData(0x03, 0x100, reg_num, p);

            for (int i = 0; i < config.bmu_num; i++){

                // 读电压断线
                pt = p+i*4;
                bmu_data[i].U64break  =  *pt;
                bmu_data[i].U64break |= (uint64_t)(*(pt + 1))<<(16);
                bmu_data[i].U64break |= (uint64_t)(*(pt + 2))<<(32);
                bmu_data[i].U64break |= (uint64_t)(*(pt + 3))<<(48);

                // 读温度断线
                pt = (p+config.bmu_num*4)+i*4;
                bmu_data[i].T64break  =  *pt;
                bmu_data[i].T64break |= (uint64_t)(*(pt + 1))<<(16);
                bmu_data[i].T64break |= (uint64_t)(*(pt + 2))<<(32);
                bmu_data[i].T64break |= (uint64_t)(*(pt + 3))<<(48);

                // 读运行状态
                pt = (p+config.bmu_num*8)+i;
                bmu_data[i].RunStat = *pt;
                // 读故障状态
                pt = (p+config.bmu_num*9)+i;
                bmu_data[i].ErrStat = *pt;

                //qDebug()<<tr("BMU%1UU64break = 0x%2:").arg(i).arg(bmu_data[i].U64break,16,16,QChar('0'));
                //qDebug()<<tr("BMU%1UT64break = 0x%2:").arg(i).arg(bmu_data[i].T64break,16,16,QChar('0'));
            }
        }else{
            reg_num = config.bmu_num * 4;
            status += ReadData(0x03, 0x100, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                bmu_data[i].Ubreak =  *(p + i);
                bmu_data[i].Tbreak =  *(p + 1 * config.bmu_num + i);
                bmu_data[i].RunStat = *(p + 2 * config.bmu_num + i);
                bmu_data[i].ErrStat = *(p + 3 * config.bmu_num + i);
            }
        }

        if (is_pVer_active()) {
            if (is_pVer_a_fan_pal()) {
                // 并充项目
                reg_num = config.bmu_num * (4 + config.vol_num);  // 均衡状态等
                status += ReadData(0x03, 0x900, reg_num, p);
                for (int i = 0; i < config.bmu_num; i++) {
                    for (int j = 0; j < config.vol_num; j++) {
                        bmu_data[i].BalIdc[j] = *(p++);
                    }
                    bmu_data[i].BalU24 = *(p++);
                    bmu_data[i].BalErr = *(p++);
                    bmu_data[i].BalStat = *(p++);
                    bmu_data[i].BalMode = *(p++);
                }
            }
            else if (is_pVer_a_liq_mos()){
                // 液冷项目
                reg_num = config.bmu_num * 15;  // 均衡状态等
                status += ReadData(0x03, 0x900, reg_num, p);
                for (int i = 0; i < config.bmu_num; i++) {
                    for (int j = 0; j < 4; j++) {
                        bmu_data[i].BalIdc[j] = *(p++);
                    }
                    bmu_data[i].BalU24 = *(p++);

                    bmu_data[i].U64BalErr = *(p++);
                    bmu_data[i].U64BalErr |= (uint64_t)(*(p++))<<16;
                    bmu_data[i].U64BalErr |= (uint64_t)(*(p++))<<32;
                    bmu_data[i].U64BalErr |= (uint64_t)(*(p++))<<48;



                    //bmu_data[i].U64BalErr = 0x0001000100010001;

                    bmu_data[i].U64BalStat = *(p++);
                    bmu_data[i].U64BalStat |= (uint64_t)(*(p++))<<16;
                    bmu_data[i].U64BalStat |= (uint64_t)(*(p++))<<32;
                    bmu_data[i].U64BalStat |= (uint64_t)(*(p++))<<48;

                    //bmu_data[i].U64BalStat = 0x0001000100010001;

                    bmu_data[i].BalMode = *(p++);
                    bmu_data[i].BalCur = (int16_t)(*(p++));
                }
            }
            else {
                reg_num = config.bmu_num * 5;  // 均衡状态等
                status += ReadData(0x03, 0x900, reg_num, p);
                for (int i = 0; i < config.bmu_num; i++) {
                    bmu_data[i].BalIdc[0] = *(p + i * BALANCE_NUM);
                    bmu_data[i].BalU24 = *(p + i * BALANCE_NUM + 1);
                    bmu_data[i].BalErr = *(p + i * BALANCE_NUM + 2);
                    bmu_data[i].BalStat = *(p + i * BALANCE_NUM + 3);
                    bmu_data[i].BalMode = *(p + i * BALANCE_NUM + 4);
                }
            }

            if (is_pVer_a_fan_pal()) {
                reg_num = config.bmu_num;  // 均衡母线电流
                status += ReadData(0x03, 0xBB8, reg_num, p);
                for (int i = 0; i < config.bmu_num; i++) {
                    bmu_data[i].BalI48 = *(p + i);
                }
            }
            if (is_pVer_a_fan_pal()||is_cpVer_match(CMU_A_FAN_MOS_V1_0_03)) {
                reg_num = config.bmu_num / 2 + config.bmu_num % 2;
                status += ReadData(0x04, 0x156A, reg_num, p);
                for (int i = 0; i < config.bmu_num; i++) {
                    // 获取奇数bmu风扇转速（从1计数）
                    if ((i + 1) % 2 == 1) {
                        bmu_data[i].FanSpeed = *(p + (i + 1 + 1) / 2 - 1);
                    }
                    // 获取偶数bmu风扇转速（从2计数）
                    else {
                        bmu_data[i].FanSpeed = (*(p + (i + 1) / 2 - 1)) >> 8;
                    }
                }
            }
        }

        if (is_cpVer_match(CMU_P_V2_0_03)) {
            reg_num = config.bmu_num * 2;
            status += ReadData(0x03, 0x900, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                bmu_data[i].CanErr = *(p + i * 2 + 1);
            }
        } else if (is_cpVer_Higher_than(CMU_P_V2_0_03)) {
            // 通信计数
            if (is_pVer_a_fan_pal()) {
                reg_num = config.bmu_num * 1;
                status += ReadData(0x04, 0xA00, reg_num, p);
                for (int i = 0; i < config.bmu_num; i++) {
                    bmu_data[i].CanErr = *(p + i);
                }
            } else {
                reg_num = config.bmu_num * 1;
                status += ReadData(0x03, 0xA00, reg_num, p);
                for (int i = 0; i < config.bmu_num; i++) {
                    bmu_data[i].CanErr = *(p + i);
                }
            }
        }
    }

    emit bmuDataReady();
    return status;
}
void mb_cmu::msg_deal(TMsgData MsgCmd) {
    QString str = "";
    for (int i = 0; i < MsgCmd.data.size(); ++i) {
        str += QString("0x%1 ").arg((uint8_t)MsgCmd.data[i],2,16,QLatin1Char('0'));
    }

    qInfo() << "deal msg:" << MsgCmd.msg_type << ",len:" << MsgCmd.data.size()<<str;

    DealCMD(MsgCmd);
}
void mb_cmu::timerEvent(QTimerEvent* event) {
    killTimer(event->timerId());
    //    qDebug() << time(nullptr);
    if (time(nullptr) > (myHelper::cvt_TIME(__DATE__) + TIME_OUTOFDATE)) {
        qWarning() << "software out of date exit..";
        this->stop = true;
        drv_status |= (0x01 << CMU_OUTOFDATE);
        return;
    }
    int rc = -1;
    //    TMsgData MsgCmd;
    uint32_t counter = 0;
    //    qDebug() << " run thread:" << QThread::currentThreadId() << m_interval << state << err_counter;

    do {
        if (this->stop) break;
        //        while (pMq->readMsg(0, MsgCmd)) {
        //            qDebug() << "recv:" << MsgCmd.msg_type << ",len:" << MsgCmd.data.size() << "," <<
        //            MsgCmd.data.toHex(); DealCMD(MsgCmd);
        //        }
        // 状态机
        if (err_counter++ >= 10) {
            qDebug() << "reconnect ip:" << this->mb_ip.c_str() << "port:" << this->mb_port;
            err_counter = 0;
            state = SM_CONNECT;
        }
        m_interval = 500;
        QElapsedTimer readTimeCostMs;
        //测每次进入离上次进入的时间差
#if EN_TRY_RECV_INFO
        qDebug() << ">>>>>" << state << currentReadGroup << "time in" << statusTimeInMs.elapsed() <<"ms";
        statusTimeInMs.start();
#endif
        switch (state) {
            case SM_READ:

                readTimeCostMs.start();
#if 0
                if (ReadALL()) {
                    if (is_pVer_active()) {
                        if (counter % (60 * 5) == 0) {
                            ReadCapData();
                        }
                        counter++;
                    }
                    state = SM_INIT;
                    Dump2Csv();
                    DumpErrLog2Csv();
                }
#else

                if(currentReadGroup == RD_GROUP1)
                {
                    if(ReadCmuData())
                    {
                        currentReadGroup = RD_GROUP2;
                    }
                }
                else if(currentReadGroup == RD_GROUP2)
                {
                    if(ReadBmuData())
                    {
                        if (is_pVer_active()) {
                            if (counter % (60 * 5) == 0) {
                                ReadCapData();
                            }
                            counter++;
                        }
                        state = SM_INIT;
                        Dump2Csv();
                        DumpErrLog2Csv();
                    }

                }
                else
                {
                    state = SM_INIT;
                }

#endif
#if EN_TRY_RECV_INFO
                qDebug() << "<><><><>read all cost time" << readTimeCostMs.elapsed() << "ms";
#endif
                break;
            case SM_CONNECT: {
                drv_status &= ~(0x01U << CMU_ONLINE);
                if (cmu != nullptr) this->Close();
                cmu = modbus_new_tcp(this->mb_ip.c_str(), this->mb_port);
                modbus_set_slave(cmu, 1);
                modbus_set_response_timeout(cmu, 3, 0);
                if (cmu) rc = modbus_connect(this->cmu);
                if (rc < 0) qWarning() << "can not connect" << rc << this->mb_ip.c_str() << ":"<< this->mb_port;
                if (rc == 0) state = SM_INIT;
                memset(tab_reg, 0, sizeof(tab_reg));
                counter = 0;
                emit connectChanged(QString("%1:%2").arg(QString::fromStdString(mb_ip)).arg(mb_port));
                break;
            }
            case SM_INIT: {
                rc = ReadData(0x03, 5411, sizeof(sys_para) / 2, sys_para.array);
                if (rc == sizeof(sys_para) / 2) {
                    state = SM_READ;
                    currentReadGroup = RD_GROUP1;
                    mapData["LocalIP"] = bswap_32(sys_para.Name.u32LocalIP);
                    mapData["ServIP"] = bswap_32(sys_para.Name.u32TftpServIP);
                    isWrLocked = (sys_para.Name.uFunCtrReg & (0x01 << WR_LOCK_BIT)) > 0 ? true : false;
                    if (config.bmu_num != sys_para.Name.u16ClusterBmuNum ||
                        config.vol_num != sys_para.Name.u16BmuCellNum || config.T_num != sys_para.Name.u16BmuPackTNum ||
                        config.Tp_num != sys_para.Name.u16BmuPoleTNum) {
                        config.bmu_num =
                            sys_para.Name.u16ClusterBmuNum > MAX_BMU ? MAX_BMU : sys_para.Name.u16ClusterBmuNum;
                        config.vol_num = sys_para.Name.u16BmuCellNum > MAX_U ? MAX_U : sys_para.Name.u16BmuCellNum;
                        config.T_num = sys_para.Name.u16BmuPackTNum > MAX_T ? MAX_T : sys_para.Name.u16BmuPackTNum;
                        config.Tp_num = sys_para.Name.u16BmuPoleTNum > MAX_T ? MAX_T : sys_para.Name.u16BmuPoleTNum;
                        config.status_num = 4;
                        Dump2CsvTitle();
                        DumpErrLog2CsvTitle();
                        qDebug() << "table changed!";
                        mapData["bmu_num"] = config.bmu_num;
                        mapData["vol_num"] = config.vol_num;
                        mapData["T_num"] = config.T_num;
                        mapData["Tp_num"] = config.Tp_num;
                        mapData["status_num"] = config.status_num;
                        emit bmsDataReady(1, mapData);
                    }
                } else {
                    m_interval = 1000;
                }
                break;
            }
            case SM_NONE:
            default:
                m_interval = 1000;
                break;
        }
    } while (0);
    startTimer(m_interval);
}

void mb_cmu::DealCMD(TMsgData& Msg) {
    qDebug() << " deal thread:" << QThread::currentThreadId();
    int ret = -1;
    switch (Msg.msg_type) {
        case CONFIG_IP: {
            if (mb_ip != Msg.data.toStdString()) {
                mb_ip = Msg.data.toStdString();
                state = SM_CONNECT;
                qDebug() << "ip config:" << mb_ip.c_str();
            }
            return Msg.data.clear();
        } break;
        case CONFIG_PORT: {
            uint16_t port = 0;
            memcpy(&port, Msg.data.data(), sizeof(uint16_t));
            // if (mb_port == port) break;
            if (port != 0) {
                mb_port = port;
            }
            qDebug() << "port config:" << port;
            return Msg.data.clear();
        } break;
        case THREAD_EXIT:
            stop = true;
            qDebug() << "recv stop flag.";
            break;
        case CONFIG_INIT:
            drv_status = 0;
            state = SM_CONNECT;
            this->startTimer(m_interval);
            ret = 0;
            break;
        case CTRL_DO: {
            if (Msg.data.size() == 2 * sizeof(uint16_t)) {
                uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
                uint16_t addr = p[0];
                uint16_t value = p[1];
                ret = modbus_write_bit(cmu, addr, value);
            }
        } break;
        case CTRL_CMD_REBOOT: {
            uint16_t nb = Msg.data.size();
            uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
            if (nb == 2 * sizeof(uint16_t)) {
                ret = write_ao(p[0], p[1]);
            }
            ret = 0;
            break;
        }

        case CTRL_AO: {
            uint16_t nb = Msg.data.size();
            if (nb < 2) break;
            uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
            if (p[0] > this->nodes_table.size()) break;
            if (nb == 2 * sizeof(uint16_t)) {
                uint16_t addr = this->nodes_table.at(p[0]).reg_addr;
                uint16_t value = p[1];
                ret = write_ao(addr, value);
            } else {
                uint16_t addr = this->nodes_table.at(p[0]).reg_addr;
                uint16_t* pv = (uint16_t*)&p[1];
                ret = write_ao(addr, (nb - 1) / 2, pv);
            }

            break;
        }
        case CTRL_SEC_AO: {
            uint16_t nb = Msg.data.size();
            if (nb == 2 * sizeof(uint16_t)){
                uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
                ret = sec_ctrl(p[0], p[1]);
            }else if(nb == 3 * sizeof(uint16_t)){
                uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
                ret = sec_ctrl(p[0], p[1],p[2]);
            }
        } break;
        case CTRL_CMD_UNLOCK: {
            ret = write_ao(ADDR_WR_LOCK, MB_UNLOCK);
        } break;
        case CTRL_CMD_CLR_ALL_SOE: {
            ret = write_ao(ADDR_CLEAR_SOE, MB_CLR_ALL_SOE);
        } break;

        case CERT_CMD_TIME_ADJ: {
            uint32_t unix_time = static_cast<uint32_t>(time(nullptr));
            ret = write_ao(ADDR_TIME_ADJ, 2, (uint16_t*)(&unix_time));
        } break;
        case CERT_CMD_READ_SOE: {
            ret = ReadSOE();
        } break;
        case CTRL_AO_ADDR: {
            uint16_t nb = Msg.data.size();
            if (nb < 2) break;
            uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
            if (nb == 2 * sizeof(uint16_t)) {
                ret = write_ao(p[0], p[1]);
            } else {
                uint16_t addr = p[0];
                uint16_t* pv = (uint16_t*)&p[1];
                ret = write_ao(addr, (nb - 1) / 2, pv);
            }
            break;
        }
        case CTRL_DUMP: {
            uint16_t nb = Msg.data.size();
            stopDump = (nb > 0);
            QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
            rec = settings->value("global/rec", "1").toInt();
            settings->deleteLater();
            qDebug() << "stop storage:" << stopDump;
            if (stopDump && csvfile) {
                qDebug() << "close old data file";
                csvfile->close();
                delete csvfile;
                csvfile = nullptr;
            }

            ret = 0;
        } break;
        case CTRL_DUMPERRLOG: {
            uint16_t nb = Msg.data.size();
            stopDumpErrLog = (nb > 0);
            if(stopDumpErrLog == false){
                OldErrLogBufMap.clear();
            }
            if (stopDumpErrLog && csvfile_errLog) {
                qDebug() << "close ErrLog data file";
                csvfile_errLog->close();
                delete csvfile_errLog;
                csvfile_errLog = nullptr;
            }

            ret = 0;
        } break;
        case CTRL_SET_PRO: {
            uint16_t nb = Msg.data.size();
            if (nb >= 1) {
                int pVer = BMS_PROTOCOL(Msg.data.toInt());
                setCompoundProtocolVer(pVer*1000 + CMU_ILIGAL_EXVER);
//                Init();
//                Dump2CsvTitle();
//                DumpErrLog2CsvTitle();
            }
            ret = 0;
        } break;
        case CTRL_SET_EXPRO: {
            uint16_t nb = Msg.data.size();
            if (nb >= 1) {
//                setCompoundProtocolVer(Msg.data.toUInt());
                qDebug() << "set CMU Ex Ver:" << compound_protocol_ver();
                QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
                settings->setValue("global/protocol", QString("CMU_V%1").arg(GetProtocalVer()));
                settings->setValue("global/ex_Ver", GetExProtocalVer());
                Init();
                Dump2CsvTitle();
                DumpErrLog2CsvTitle();
            }
            ret = 0;
        } break;

        case CTRL_SET_ERRLOG_ULIMIT: {
            uint16_t nb = Msg.data.size();
            if (nb != 0) {
                errLogUcellLimitValue = Msg.data.toInt()*10;
                qDebug() << "errLogUcellLimitValue:" << errLogUcellLimitValue;
            }
            ret = 0;
        } break;
        case CTRL_SET_ERRLOG_TLIMIT: {
            uint16_t nb = Msg.data.size();
            if (nb != 0) {
                errLogTempLimitValue = Msg.data.toInt()*10;
                qDebug() << "errLogTempLimitValue:" << errLogTempLimitValue;
            }
            ret = 0;
        } break;
        case CTRL_SET_ERRLOG_STDVAL: {
            uint16_t nb = Msg.data.size();
            if (nb != 0) {
                errLogStdLimitValue = Msg.data.toInt()*10;
                qDebug() << "errLogStdLimitValue:" << errLogStdLimitValue;
            }
            ret = 0;
        } break;
        case CTRL_SET_ERRLOG_METHOD: {
            uint16_t nb = Msg.data.size();
            OldErrLogBufMap.clear();
            if(nb == 1){
                logSaveMethod = (ERRLOG_METHOD)Msg.data.toInt();
                qDebug()<<" errlogSaveMethod:" << logSaveMethod;
            }else if(nb == 4){
                uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
                logSaveMethod = (ERRLOG_METHOD)p[0];
                errLogCount = p[1];
                qDebug()<<" errlogSaveMethod:" << logSaveMethod <<" errLogCount:"<<errLogCount;
            }
            ret = 0;
        } break;
        default:
            break;
    }
    Msg.data.clear();
    if (ret < 0) {
        qWarning() << tr("操作失败");
        emit signal_message(QString(tr("操作失败")));
    } else {
        qDebug() << tr("操作成功");
        emit signal_message(QString(tr("操作成功")));
    }
}
int mb_cmu::write_ao(uint16_t addr, uint16_t len, uint16_t* pv) {
    int ret = -1;
    if (!pv) return ret;
    if (len < 1) return ret;
    if (len == 1) {
        ret = write_ao(addr, *pv);
        return ret;
    }
    if (isWrLocked) modbus_write_register(cmu, ADDR_WR_LOCK, MB_UNLOCK);
    ret = modbus_write_registers(cmu, addr, len, pv);
    if (ret < 0)
        qWarning() << "wr aos failed" << addr << ":" << ret;
    else
        qDebug() << "wr aos " << addr << ":" << len;
    return ret;
}
int mb_cmu::write_ao(uint16_t addr, uint16_t v) {
    int ret = -1;
    if (isWrLocked) modbus_write_register(cmu, ADDR_WR_LOCK, MB_UNLOCK);
    ret = modbus_write_register(cmu, addr, v);
    if (ret < 0)
        qWarning() << "wr ao failed" << addr << ":" << ret;
    else
        qDebug() << "wr ao " << addr << ":" << v;
    return ret;
}
int mb_cmu::sec_ctrl(uint16_t addr, uint16_t type) {
    sec_cmd[8] = type;
    return write_ao(addr, 9, sec_cmd);
}
int mb_cmu::sec_ctrl(uint16_t addr, uint16_t type,uint16_t value) {
    sec_cmd[8] = type;
    sec_cmd[9] = value;
    return write_ao(addr, 10, sec_cmd);
}
int mb_cmu::ParseData() {
    int ret = -1;

    return ret;
}
/**< 判别寄存器插入条件，返回可用报文序号，否则返回-1 */
int mb_cmu::JudgeReg(NodeReg& node_reg) {
    int index = reg_list_.size();
    const int max_frame_len = 229;
    int shortnum = (max_frame_len - 9) / 2;
    int bitnum = shortnum * 16;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) / 2;
    reg_len = (reg_len) > 1 ? reg_len : 1;
    uint8_t reg_type = node_reg.reg_type;

    //超过一定范围无相邻的寄存器不能形成批量读列表
    bool refuseToInsert = true;
    foreach(db_manager::ST_DB_NODE nodeTemp, this->nodes_table)
    {
        if(nodeTemp.reg_type == node_reg.reg_type
                &&nodeTemp.reg_addr != node_reg.reg_addr)
        {
            int diff = int(node_reg.reg_addr) - int(nodeTemp.reg_addr);
            if(diff < REG_DIFF_REFUSE_TO_INSERT && diff > 0)
            {
                refuseToInsert = false;
                break;
            }
        }
    }
    if(refuseToInsert)
    {
        return -1;
    }

    while (index--) {
        DataReg data_reg = reg_list_.at(index);
        if (reg_type == DO_REG || reg_type == DI_REG) {
            if ((uint16_t)(node_reg.reg_addr - data_reg.reg_start) < bitnum
                    && data_reg.reg_type == reg_type) {
                return index;
            }
        }
        if (reg_type == AO_REG || reg_type == AI_REG) {
            if ((uint16_t)(node_reg.reg_addr + reg_len - data_reg.reg_start) < shortnum
                    && data_reg.reg_type == reg_type) {
                return index;
            }
        }
    }
    return -1;
}
void mb_cmu::NewReg(NodeReg& node_reg) {
    DataReg data_reg;
    DatabaseIO data_io_tmp;
    data_reg.data_io.clear();
    data_reg.reg_type = node_reg.reg_type;
    data_reg.reg_start = node_reg.reg_addr;
    data_reg.data_num = 1;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) >> 1;
    data_reg.reg_num = (reg_len) > 1 ? reg_len : 1;
    data_io_tmp.index = node_reg.index;
    data_io_tmp.factor = node_reg.factor;
    data_io_tmp.data_type = node_reg.data_type;
    data_io_tmp.offset = 0;
    data_reg.data_io.push_back(data_io_tmp);
    reg_list_.push_back(data_reg);
}
void mb_cmu::InsertReg(NodeReg& node_reg, int index) {
    DataReg* pdata_reg;
    pdata_reg = &reg_list_.at(index);
    DatabaseIO data_io_tmp;
    pdata_reg->data_num++;
    data_io_tmp.index = node_reg.index;
    data_io_tmp.factor = node_reg.factor;
    data_io_tmp.data_type = node_reg.data_type;
    data_io_tmp.offset = node_reg.reg_addr - pdata_reg->reg_start;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) >> 1;
    reg_len = (reg_len) > 1 ? reg_len : 1;
    int len = data_io_tmp.offset + reg_len;
    if (pdata_reg->reg_num < len) pdata_reg->reg_num = len;
    pdata_reg->data_io.push_back(data_io_tmp);
}

int mb_cmu::ReadAI() {
    int res = -1;
    uint16_t tab_buf[128];

    for (vector<DataReg>::iterator iter = reg_list_.begin(); iter != reg_list_.end(); iter++) {
        res = ReadData(iter->reg_type, iter->reg_start, iter->reg_num, tab_buf);

        if (res == iter->reg_num) {

//            qWarning()<< "<<<<<<<<<<<<<<<<<<" <<iter->reg_start << iter->reg_num;
            for (vector<DatabaseIO>::iterator data_iter = iter->data_io.begin(); data_iter != iter->data_io.end();
                 data_iter++) {
                if (this->nodes_table.size() > data_iter->index) {
                    qreal value = 0;
//                    qWarning() << iter->reg_start + data_iter->offset << tab_buf[data_iter->offset];
                    if (data_iter->data_type == 514) { // 0x202
                        value = tab_buf[data_iter->offset] * data_iter->factor;
                    } else if (data_iter->data_type == 513) { // 0x201
                        value = (int16_t)tab_buf[data_iter->offset] * data_iter->factor;
                    } else if (data_iter->data_type == 17410) { // 0x4402
                        value = ((uint32_t)MODBUS_GET_INT32_FROM_INT16_SWAP(tab_buf, data_iter->offset)) * data_iter->factor;
                    } else if (data_iter->data_type == 17409) { // 0x4401
                        value = ((int32_t)MODBUS_GET_INT32_FROM_INT16_SWAP(tab_buf, data_iter->offset)) * data_iter->factor;
                    } else {
                        value = tab_buf[data_iter->offset] * data_iter->factor;
                    }
                    mapData[this->nodes_table.at(data_iter->index).node_name] = value;
                }

            }
        }
        else
        {
//            qDebug() << "<<<<<<< reg_num not right" << res << iter->reg_num;
        }
    }

    return res;
}
#define MAX_SOE_COUNT 2000
#define SOE_REG_LEN   8

static uint16_t get_data(const uint16_t* src, int index) {
    uint16_t val = src[index];
    return static_cast<uint16_t>((val >> 8) | (val << 8));
}
int mb_cmu::ReadSOE() {
    int res = -1;
    uint16_t tab_buf[128] = {0};
    res = ReadData(0x03, 0x2000, 2, tab_buf);
    if (res != 2) return -1;
    cmu_soe.list_soe.clear();
    if (((is_pVer_active()) && (this->cmu_ver >= 0x00000402)) ||
        ((is_pVer_passive()) && (this->cmu_ver >= 0x00000407))) {
        if(is_exVer_3levels_alarm()){
            cmu_soe.type = db_manager::SOE_BMS4;
        }else if(is_pVer_a_fan_pal()){
            cmu_soe.type = db_manager::SOE_BMS3;
        }else{
            cmu_soe.type = db_manager::SOE_BMS2;
        }
    } else {
        cmu_soe.type = db_manager::SOE_BMS1;
    }
    cmu_soe.new_soe_count = tab_buf[0];
    cmu_soe.soe_count = tab_buf[1];
    int start = 0x2002;
    int len = cmu_soe.soe_count > MAX_SOE_COUNT ? MAX_SOE_COUNT : cmu_soe.soe_count;
    int soe_index = 0;
    do {
        int soe_len = len > 15 ? 15 : len;
        len -= soe_len;
        res = modbus_read_registers(cmu, start, soe_len * SOE_REG_LEN, tab_buf);
        // qDebug() << start << "->" << start + soe_len * SOE_REG_LEN<<","<<soe_index;
        if (res == soe_len * SOE_REG_LEN) {
            start += (soe_len * SOE_REG_LEN);
            for (int i = soe_len; --i >= 0;) {
                uint64_t u64time = static_cast<uint32_t>((get_data(tab_buf, SOE_REG_LEN * i + 1) << 16) |
                                                         get_data(tab_buf, SOE_REG_LEN * i));
                //                if (u64time > 0xFFFFFFFF) {
                //                    soe_index++;
                //                    continue;
                //                }
                u64time = u64time * 1000 + get_data(tab_buf, SOE_REG_LEN * i + 2);
                CMU_SOE soe;
                soe.soe_time = u64time;
                soe.soe_stat = get_data(tab_buf, SOE_REG_LEN * i + 3);
                soe.soe_type = get_data(tab_buf, SOE_REG_LEN * i + 4);
                soe.soe_id = get_data(tab_buf, SOE_REG_LEN * i + 5);
                soe.soe_val = get_data(tab_buf, SOE_REG_LEN * i + 6);
                soe.soe_limit = get_data(tab_buf, SOE_REG_LEN * i + 7);
                cmu_soe.list_soe.append(soe);
                soe_index++;
            }
        } else {
            break;
        }
    } while (len);
    qDebug() << "read soe succeed.";
    emit bmsSOEReady(cmu_soe);
    return res;
}

