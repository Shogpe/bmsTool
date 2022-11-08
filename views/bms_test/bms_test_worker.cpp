/**
 * 测试任务执行
 *
 *
 *
 */
#include "bms_test_worker.h"
#include <QtNetwork/QNetworkInterface.h>
#include <QDateTime>
#include <QHostAddress>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QTimer>
#include <QtDebug>
#include <QtEndian>
#include <QtGlobal>
#include <QtXml>
#include "Toast.h"
#include "mb_cmu.h"
#include "myhelper.h"
/**
 * @brief 检查是否有均衡故障状态
 * @param ip
 * @param setMap
 *
 */
void bmsTestWorker::doTest(QString ip, const QMap<QString, double> setMap) {
    QStringList ip_list = ip.split(":");
    int port = 502;
    if (ip_list.size() > 1) {
        port = ip_list.at(1).toInt();
    }

    mb_tcp* m_mbtcp = new mb_tcp(ip_list.at(0), port);
    if (m_mbtcp->Connect() == -1) {
        m_mbtcp->close();
        m_mbtcp->deleteLater();
        m_mbtcp = nullptr;
        emit workFinished(0, "Connect Err");
        return;
    }
    vector<MB_NODE> tab_config;
    tab_config.clear();
    MB_NODE* node_table = cmu_v4_config;
    int node_table_size = cmu_v4_config_len;

    for (int i = 0; i < node_table_size; i++) {
        if (node_table[i].val_type != 129) continue;
        if (!setMap.contains(node_table[i].name)) continue;
        node_table[i].index = tab_config.size();
        tab_config.push_back(node_table[i]);
    }
    m_mbtcp->init_config(tab_config);

    vector<ST_NODE_DATA> data = m_mbtcp->ReadALL();
    int ok_count = 0;
    for (int i = 0; i < data.size(); i++) {
        if (setMap.contains(tab_config.at(i).name)) {
            qDebug() << i << tab_config.at(i).name << data.at(i).sysData.val.f64 << setMap.value(tab_config.at(i).name)
                     << qFuzzyCompare(data.at(i).sysData.val.f64, setMap.value(tab_config.at(i).name));
            //            if (qFuzzyCompare(data.at(i).sysData.val.f64, setMap.value(tab_config.at(i).name)))
            //            ok_count++;
            if (QString("%1").arg(data.at(i).sysData.val.f64) ==
                QString("%1").arg(setMap.value(tab_config.at(i).name))) {
                ok_count++;
            } else {
                qWarning() << QString("%1").arg(data.at(i).sysData.val.f64)
                           << QString("%1").arg(setMap.value(tab_config.at(i).name));
            }
        }
    };
    if (ok_count == setMap.size()) {
        emit workFinished(1, "OK");
    } else {
        emit workFinished(0, "FAIL");
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
void bmsTestWorker::doSetIp(QString ip, QString serverIp) {
    QStringList ip_list = ip.split(":");
    int port = 502;
    if (ip_list.size() > 1) {
        port = ip_list.at(1).toInt();
    }
    quint32 ip_addr = QHostAddress(serverIp).toIPv4Address();
    mb_tcp* m_mbtcp = new mb_tcp(ip_list.at(0), port);
    if (m_mbtcp->Connect() == -1) {
        m_mbtcp->close();
        m_mbtcp->deleteLater();
        m_mbtcp = nullptr;
        emit workFinished(0, "Connect Err");
        return;
    }
    int ok_count = 0;
    ip_addr = qFromBigEndian(ip_addr);  // 转换为小端模式
    if (m_mbtcp->write_ao(5420, 2, (uint16_t*)&ip_addr) > 0) {
        ok_count++;
    } else {
        qWarning() << QString("set serverIp %1 failed.").arg(serverIp);
    }
    if (ok_count == 1) {
        emit workFinished(1, "Set OK");
    } else {
        emit workFinished(0, "Set FAIL");
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
void bmsTestWorker::doSetData(QString ip, const QMap<QString, double> setMap) {
    QStringList ip_list = ip.split(":");
    int port = 502;
    if (ip_list.size() > 1) {
        port = ip_list.at(1).toInt();
    }

    mb_tcp* m_mbtcp = new mb_tcp(ip_list.at(0), port);
    if (m_mbtcp->Connect() == -1) {
        m_mbtcp->close();
        m_mbtcp->deleteLater();
        m_mbtcp = nullptr;
        emit workFinished(0, "Connect Err");
        return;
    }
    vector<MB_NODE> tab_config;
    tab_config.clear();
    MB_NODE* node_table = cmu_v4_config;
    int node_table_size = cmu_v4_config_len;

    for (int i = 0; i < node_table_size; i++) {
        if (node_table[i].val_type != 129) continue;
        if (!setMap.contains(node_table[i].name)) continue;
        node_table[i].index = tab_config.size();
        tab_config.push_back(node_table[i]);
    }
    int ok_count = 0;
    for (uint i = 0; i < tab_config.size(); i++) {
        if (setMap.contains(tab_config.at(i).name)) {
            uint16_t addr = tab_config.at(i).reg_addr;
            double_t dval = setMap.value(tab_config.at(i).name);
            uint16_t val = tab_config.at(i).factor == 0
                               ? uint16_t(dval)
                               : static_cast<uint16_t>(std::round(dval / tab_config.at(i).factor));

            if (m_mbtcp->write_ao(addr, val) > 0) {
                ok_count++;
            } else {
                qWarning() << QString("%1(%2) set %3 failed.").arg(tab_config.at(i).name).arg(addr).arg(dval);
            }
        }
    };
    if (ok_count == setMap.size()) {
        emit workFinished(1, "Set OK");
    } else {
        emit workFinished(0, "Set FAIL");
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
void bmsTestWorker::doCommand(QString ip, uint command) {
    QStringList ip_list = ip.split(":");
    int port = 502;
    if (ip_list.size() > 1) {
        port = ip_list.at(1).toInt();
    }

    mb_tcp* m_mbtcp = new mb_tcp(ip_list.at(0), port);
    if (m_mbtcp->Connect() == -1) {
        m_mbtcp->close();
        m_mbtcp->deleteLater();
        m_mbtcp = nullptr;
        emit workFinished(0, "Connect Err");
        return;
    }
    uint16_t val = 0;
    qDebug() << command;
    switch (command) {
        default:
            emit workFinished(0, QString("未定义命令:%1").arg(command));
            break;
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
