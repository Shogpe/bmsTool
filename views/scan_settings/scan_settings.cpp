#include "scan_settings.h"
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
#include "firmwareDialog.h"
#include "mb_cmu.h"
#include "myhelper.h"
#include "ui_scan_settings.h"
void testWorker::doTest(QString ip, const QMap<QString, double> setMap) {
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
        emit workFinished(0, tr("Connect Err"));
        return;
    }
    // 查询当前版本定制单
    QString protocol = QSettings("config.ini", QSettings::IniFormat).value("global/protocol", "CMU1.0").toString();
    QList<db_manager::ST_DB_NODE> nodes_table;
    uint protocal_ver = 3;
    if (g_proto_map.contains(protocol)) {
        protocal_ver = g_proto_map.value(protocol);
    }
    db_manager::Instance()->getNode(nodes_table, protocal_ver);
    vector<MB_NODE> tab_config;
    tab_config.clear();

    for (int i = 0; i < nodes_table.size(); i++) {
        if (nodes_table.at(i).val_type != 129) continue;
        if (!setMap.contains(nodes_table.at(i).node_name)) continue;
        MB_NODE tmp;
        tmp.index = tab_config.size();
        strncpy(tmp.name, nodes_table.at(i).node_name.toStdString().c_str(), 64);
        tmp.reg_type = nodes_table.at(i).reg_type;
        tmp.reg_addr = nodes_table.at(i).reg_addr;
        tmp.data_type = nodes_table.at(i).data_type;
        tmp.val_type = nodes_table.at(i).val_type;
        tmp.factor = nodes_table.at(i).factor;

        tab_config.push_back(tmp);
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
        emit workFinished(1, tr("OK"));
    } else {
        emit workFinished(0, tr("FAIL"));
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
void testWorker::doSetIp(QString ip, QString serverIp) {
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
        emit workFinished(0, tr("Connect Err"));
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
        emit workFinished(1, tr("Set OK"));
    } else {
        emit workFinished(0, tr("Set FAIL"));
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
void testWorker::doSetData(QString ip, const QMap<QString, double> setMap) {
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
        emit workFinished(0, tr("Connect Err"));
        return;
    }
    // 查询当前版本定制单
    QString protocol = QSettings("config.ini", QSettings::IniFormat).value("global/protocol", "CMU1.0").toString();
    QList<db_manager::ST_DB_NODE> nodes_table;
    uint protocal_ver = 3;
    if (g_proto_map.contains(protocol)) {
        protocal_ver = g_proto_map.value(protocol);
    }
    db_manager::Instance()->getNode(nodes_table, protocal_ver);
    vector<db_manager::ST_DB_NODE> tab_config;
    tab_config.clear();

    for (int i = 0; i < nodes_table.size(); i++) {
        db_manager::ST_DB_NODE tmp = nodes_table.at(i);
        if (tmp.val_type != 129) continue;
        if (!setMap.contains(tmp.node_name)) continue;
        tmp.node_id = tab_config.size();
        tab_config.push_back(tmp);
    }

    int ok_count = 0;
    for (uint i = 0; i < tab_config.size(); i++) {
        if (setMap.contains(tab_config.at(i).node_name)) {
            uint16_t addr = tab_config.at(i).reg_addr;
            double_t dval = setMap.value(tab_config.at(i).node_name);
            uint16_t val = tab_config.at(i).factor == 0
                               ? uint16_t(dval)
                               : static_cast<uint16_t>(std::round(dval / tab_config.at(i).factor));

            if (m_mbtcp->write_ao(addr, val) > 0) {
                ok_count++;
            } else {
                qWarning() << QString("%1(%2) set %3 failed.").arg(tab_config.at(i).node_name).arg(addr).arg(dval);
            }
        }
    };
    if (ok_count == setMap.size()) {
        emit workFinished(1, tr("Set OK"));
    } else {
        emit workFinished(0, tr("Set FAIL"));
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
void testWorker::doCommand(QString ip, uint command) {
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
        emit workFinished(0, tr("Connect Err"));
        return;
    }
    uint16_t val = 0;
    qDebug() << command;
    switch (command) {
        case CMD_UP_CMU: {
            val = 0x5A78;  // 升级CMU
            uint16_t sec_cmd[9] = {0x1223, 0x3445, 0x5667, 0x7889, 0x9000U, 0x1122, 0x3344, 0x5566};
            sec_cmd[8] = val;
            if (m_mbtcp->write_ao(0xFFD0, 9, sec_cmd) > 0) {
                emit workFinished(1, tr("发送升级CMU 成功"));
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("发送升级CMU失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_UP_BMU: {
            val = 0x5A33;  // 升级BMU
            uint16_t sec_cmd[9] = {0x1223, 0x3445, 0x5667, 0x7889, 0x9000U, 0x1122, 0x3344, 0x5566};
            sec_cmd[8] = val;
            if (m_mbtcp->write_ao(0xFFD0, 9, sec_cmd) > 0) {
                emit workFinished(1, tr("发送升级BMU OK"));
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("发送升级BMU 失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_UP_BMUBOOT: {
            val = 0x5A66;  // 升级BMUBoot
            uint16_t sec_cmd[9] = {0x1223, 0x3445, 0x5667, 0x7889, 0x9000U, 0x1122, 0x3344, 0x5566};
            sec_cmd[8] = val;
            if (m_mbtcp->write_ao(0xFFD0, 9, sec_cmd) > 0) {
                emit workFinished(1, tr("发送升级BMUBoot OK"));
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("发送升级BMUBoot 失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_UP_INS: {
            val = 0xA5B6;  // 升级绝缘板
            uint16_t sec_cmd[9] = {0x1223, 0x3445, 0x5667, 0x7889, 0x9000U, 0x1122, 0x3344, 0x5566};
            sec_cmd[8] = val;
            if (m_mbtcp->write_ao(0xFFD0, 9, sec_cmd) > 0) {
                emit workFinished(1, tr("发送升级绝缘板 成功"));
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("发送升级绝缘板 失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_RD_VER_BMS: {
            uint32_t v = 0;
            if (m_mbtcp->read_value(0x03, 1280, 2, (uint16_t*)&v) > 0) {
                emit workFinished(1, QString("%1:%2").arg(tr("读取BMS版本 成功"), myHelper::IntegerToHexString(v)));
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("读取BMS版本 失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_RD_VER_INS: {
            uint32_t v = 0;
            if (m_mbtcp->read_value(0x03, 1274, 2, (uint16_t*)&v) > 0) {
                emit workFinished(1, QString("%1:%2").arg(tr("读取绝缘版本 成功"), myHelper::IntegerToHexString(v)));
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("读取绝缘版本 失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_LOCK_BMU: {
            if (m_mbtcp->write_ao(0xFFF4, 0xA5B6) > 0) {
                if (m_mbtcp->write_ao(0xFFF1, MB_BMU_LOCK) > 0) {
                    emit workFinished(1, tr("BMU拨码锁定成功."));
                } else {
                    emit workFinished(0, QString("%1:%2").arg(tr("BMU拨码锁定 失败"), m_mbtcp->get_error_msg()));
                }
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("BMS解锁失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_UNLOCK_BMU: {
            if (m_mbtcp->write_ao(0xFFF4, 0xA5B6) > 0) {
                if (m_mbtcp->write_ao(0xFFF1, MB_BMU_UNLOCK) > 0) {
                    emit workFinished(1, tr("BMU拨码解锁成功."));
                } else {
                    emit workFinished(0, QString("%1:%2").arg(tr("BMU拨码解锁失败"), m_mbtcp->get_error_msg()));
                }
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("BMS解锁失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_RESET_ADJ: {
            if (m_mbtcp->write_ao(0xFFF4, 0xA5B6) > 0) {
                if (m_mbtcp->write_ao(0xFF0A, 0xAA55) > 0) {
                    emit workFinished(1, tr("恢复默认校准参数成功."));
                } else {
                    emit workFinished(0, QString("%1:%2").arg(tr("恢复默认校准参数失败"), m_mbtcp->get_error_msg()));
                }
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("BMS解锁失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_RESET_PAR: {
            if (m_mbtcp->write_ao(0xFFF4, 0xA5B6) > 0) {
                if (m_mbtcp->write_ao(0xFF0A, 0xBB66) > 0) {
                    emit workFinished(1, tr("恢复默认运行参数成功."));
                } else {
                    emit workFinished(0, QString("%1:%2").arg(tr("恢复默认运行参数失败"), m_mbtcp->get_error_msg()));
                }
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("BMS解锁失败"), m_mbtcp->get_error_msg()));
            }
        } break;
        case CMD_FixTime: {
            uint32_t unix_time = static_cast<uint32_t>(time(nullptr));
            if (m_mbtcp->write_ao(ADDR_TIME_ADJ, 2, (uint16_t*)(&unix_time)) > 0) {
                emit workFinished(1, tr("校时成功."));
            } else {
                emit workFinished(0, QString("%1:%2").arg(tr("校时失败."), m_mbtcp->get_error_msg()));
            }
        } break;
        default:
            emit workFinished(0, QString("%1:%2").arg(tr("未定义命令"), command));
            break;
    }
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
QString ipv4int_to_str(quint32 ipint) {
    return QString("%1.%2.%3.%4")
        .arg((ipint >> 24) & 0xff)
        .arg((ipint >> 16) & 0xff)
        .arg((ipint >> 8) & 0xff)
        .arg(ipint & 0xff);
}

quint32 ipv4str_to_int(const QString& ipstr) {
    QStringList ip4 = ipstr.split(".");
    if (ip4.size() == 4) {
        return ip4.at(3).toInt() | ip4.at(2).toInt() << 8 | ip4.at(1).toInt() << 16 | ip4.at(0).toInt() << 24;
    } else {
        return 0;
    }
}
scan_settings::scan_settings(QWidget* parent) : QWidget(parent), ui(new Ui::scan_settings) {
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    uiInit();
    m_mbtcp = nullptr;
    m_timer = new QTimer();
    //
    connect(ui->btnLoadXml, &QPushButton::released, this, &scan_settings::loadXml);
    connect(&qtftp, &Qtftp::fileSent, this, [this](int ret, QString file) {
        qDebug() << tr("文件:") << file << ((ret == 0) ? tr("传输成功") : tr("传输失败"));
        auto list = m_result_model->GetData();
        for (int i = 0; i < list.size(); i++) {
            if (file == QString::fromStdString(list.at(i).name))
                m_result_model->updateData(i, (ret == 0) ? tr("传输成功") : tr("传输失败"));
        }
    });
    {
        ui->srvStatus->setText(tr("服务检测中..."));
        m_timer->setInterval(1000);
        connect(&m_thread, &QThread::started, this->m_timer, static_cast<void (QTimer::*)()>(&QTimer::start));
        connect(m_timer, &QTimer::timeout, this, &scan_settings::checkServer, Qt::DirectConnection);
        connect(&m_thread, &QThread::finished, m_timer, &QTimer::stop);
        connect(&m_thread, &QThread::finished, m_timer, &QTimer::deleteLater);
        connect(this, &scan_settings::checkRespond, this, [this](int result) {
            switch (result) {
                case 1:
                    ui->srvStatus->setText(tr("远程服务在线"));
                    ui->srvStatus->setStyleSheet("color:darkGreen;");
                    ui->srvStatus->setToolTip(tr("可以上传固件至远程服务器"));
                    ui->btnUpload->setDisabled(false);
                    break;
                case 2:
                    ui->srvStatus->setText(tr("本机服务在线"));
                    ui->srvStatus->setStyleSheet("color:darkGreen;");
                    ui->srvStatus->setToolTip(tr("请查看下方状态栏，检查服务器是否启动成功"));
                    ui->btnUpload->setDisabled(true);
                    break;
                default:
                    ui->srvStatus->setText(tr("远程服务离线"));
                    ui->srvStatus->setStyleSheet("color:darkRed;text-decoration:underline;");
                    ui->srvStatus->setToolTip(tr("可以修改IP以启用本地服务器"));
                    ui->btnUpload->setDisabled(true);
                    break;
            }
        });
        m_timer->moveToThread(&m_thread);
        m_thread.start();
    }
}

scan_settings::~scan_settings() {
    m_thread.quit();
    m_thread.wait();
    qDebug() << "delete";
}
void scan_settings::checkServer() {
    m_timer->stop();
    QString ipAddr;
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            ipAddr = address.toString();
            if (ipAddr != "192.168.1.230") continue;
            break;
        }
    }
    if (ipAddr != "192.168.1.230") {
        QString network_cmd = "ping 192.168.1.230 -n 1 -w 1000";
        QString result;
        QProcess network_process;                                                  // 不要加this
        network_process.start(network_cmd);                                        // 调用ping 指令
        network_process.waitForFinished();                                         // 等待指令执行完毕
        result = network_process.readAll();                                        // 获取指令执行结果
                                                                                   //         qDebug() << result;
        if (result.contains(QString("TTL=")) || result.contains(QString("ttl=")))  // 若包含TTL=字符串则认为网络在线
        {
            emit checkRespond(1);
        } else {
            emit checkRespond(0);
        }
    } else {
        emit checkRespond(2);
    }

    m_timer->start(5000);
}
void scan_settings::setBusy(bool is_busy) {
    if (is_busy) {
        ui->btnWrite->setDisabled(true);
        ui->btnTest->setDisabled(true);
        ui->btnCtrl->setDisabled(true);
        ui->btnSetIp->setDisabled(true);
        this->setCursor(QCursor(Qt::WaitCursor));
    } else {
        ui->btnTest->setDisabled(false);
        ui->btnWrite->setDisabled(false);
        ui->btnCtrl->setDisabled(false);
        ui->btnSetIp->setDisabled(false);
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}
void scan_settings::ip_analyze() {
    QStringList host_list = ui->connectIP->text().split(",");
    //        QStringList target_ips;
    m_mutex.lock();
    target_ips.clear();
    target_count = 0;
    for (int i = 0; i < host_list.size(); ++i) {
        QStringList tmp_ips;
        QList<int> tmp_ports;
        QStringList tmp = host_list.at(i).split(":");
        if (tmp.size() > 0) {
            QStringList ips = tmp.at(0).split("-");
            if (ips.size() == 2) {
                if (myHelper::IsIP(ips.at(0)) && myHelper::IsIP(ips.at(1))) {
                    quint32 ip_start = ipv4str_to_int(ips.at(0));
                    quint32 ip_end = ipv4str_to_int(ips.at(1));
                    for (; ip_start <= ip_end; ip_start++) {
                        if (!ip_start) continue;
                        tmp_ips << ipv4int_to_str(ip_start);
                    }
                }
            } else if (ips.size() == 1) {
                if (myHelper::IsIP(tmp.at(0))) {
                    tmp_ips << tmp.at(0);
                }
            }
        }
        if (tmp.size() > 1) {
            QStringList ports = tmp.at(1).split("-");
            if (ports.size() == 2) {
                bool ok1 = false, ok2 = false;
                QString port_str1 = ports.at(0);
                QString port_str2 = ports.at(1);
                int port1 = port_str1.toInt(&ok1);
                int port2 = port_str2.toInt(&ok2);
                if (ok1 && ok2 && (port1 > 0 && port1 < 65535) && (port2 > 0 && port2 < 65535)) {
                    for (; port1 <= port2; port1++) {
                        if (!port1) continue;
                        tmp_ports << port1;
                    }
                }
            } else if (ports.size() == 1) {
                bool ok = false;
                QString port_str = ports.at(0);
                int port = port_str.toInt(&ok);
                if (ok && (port > 0 && port < 65535)) {
                    tmp_ports << port;
                }
            }
        }
        if (tmp_ports.size() == 0) {
            for (int j = 0; j < tmp_ips.size(); j++) {
                target_ips << QString("%1:502").arg(tmp_ips.at(j));
            }
        } else {
            for (int k = 0; k < tmp_ports.size(); k++) {
                for (int j = 0; j < tmp_ips.size(); j++) {
                    target_ips << QString("%1:%2").arg(tmp_ips.at(j)).arg(tmp_ports.at(k));
                }
            }
        }
    }
    qDebug() << target_ips;
    QList<ST_PARA> plist;
    for (int i = 0; i < target_ips.size(); i++) {
        ST_PARA p;
        p.index = i;
        QString name = target_ips.at(i);
        p.name = name.toStdString();
        p.name_cn = "";
        p.type = TP_STR;
        plist << p;
    }
    ui->testResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->testResult->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_result_model->updateData(plist);
    ui->testResult->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->testResult->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_mutex.unlock();
}

void scan_settings::uiInit() {
    /* 设置 tableWidget */
    //  tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    //  tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    this->setWindowTitle(tr("BMS维护工具"));
    m_para_model = new ParaModel(this);
    m_result_model = new ParaModel(this);
    QTableView* tableView = ui->stdSetting;
    tableView->setAlternatingRowColors(true);
    QStringList headerList;
    headerList << "No"
               << "Name"
               << "Value";
    m_para_model->setHorizontalHeaderLabels(headerList);
    tableView->verticalHeader()->setVisible(false);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);  // 选中行
    tableView->horizontalHeader()->setStretchLastSection(true);

    //
    ReadOnlyDelegate* readOnlyDelegate = new ReadOnlyDelegate(this);
    ValueDelegate* valDelegate = new ValueDelegate(this);
    tableView->setItemDelegateForColumn(0, readOnlyDelegate);
    tableView->setItemDelegateForColumn(1, readOnlyDelegate);
    tableView->setItemDelegateForColumn(2, valDelegate);

    tableView->setModel(m_para_model);

    // 结果显示
    tableView = ui->testResult;
    tableView->setAlternatingRowColors(true);
    QStringList resultList;
    resultList << "No"
               << "Name"
               << "Result";
    m_result_model->setHorizontalHeaderLabels(resultList);
    tableView->verticalHeader()->setVisible(false);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);  // 选中行
    tableView->horizontalHeader()->setStretchLastSection(true);

    //
    tableView->setItemDelegateForColumn(0, readOnlyDelegate);
    tableView->setItemDelegateForColumn(1, readOnlyDelegate);
    tableView->setItemDelegateForColumn(2, readOnlyDelegate);

    tableView->setModel(m_result_model);

    QMenu* update_menu = new QMenu;
    update_menu->addAction(tr("下载升级BMS"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("下载升级BMU"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("下载升级BMUBOOT"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("下载升级绝缘板"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("读取BMS版本号"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("读取绝缘版本号"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("BMU拨码锁定⚿"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("BMU拨码解锁"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("恢复校准参数"), this, &scan_settings::btnCtrlMenu);
    update_menu->addAction(tr("恢复运行参数"), this, &scan_settings::btnCtrlMenu);
    ui->btnCtrl->setMenu(update_menu);

    QMenu* btn_menu = new QMenu;
    btn_menu->addAction(tr("设置服务IP为本地"), this, &scan_settings::btnCtrlMenu);
    btn_menu->actions().constLast()->setObjectName("setServerIp");
    btn_menu->addAction(tr("恢复默认服务IP"), this, &scan_settings::btnCtrlMenu);
    btn_menu->actions().constLast()->setObjectName("resetServerIp");
    ui->btnSetIp->setMenu(btn_menu);
    ui->btnSetIp->setHidden(true);
    QString ipRange =
        QSettings("config.ini", QSettings::IniFormat).value("SCAN/iprange", "192.168.1.121-192.168.1.128").toString();
    ui->connectIP->setText(ipRange);
    ui->btnUpload->setDisabled(true);
}

void scan_settings::loadXml() {
    // 从xml加载配置
    QString filename = QFileDialog::getOpenFileName(this, "Open", "", "*.xml");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();
    QDomElement root = doc.documentElement();  // 返回根节点
    QDomNode node = root.firstChild();         // 获得第一个子节点
    QList<ST_PARA> plist;
    QMap<QString, ST_PARA> data_map;
    while (!node.isNull())  // 如果节点不空
    {
        if (node.isElement())  // 如果节点是元素
        {
            QDomElement e = node.toElement();  // 转换为元素，注意元素和节点是两个数据结构，其实差不多
            if ((e.attribute("name") != nullptr) && (e.attribute("name_cn") != nullptr)) {
                ST_PARA p;
                p.name = e.attribute("name_cn").toStdString();
                p.val = e.attribute("value").toDouble();
                p.name_cn = e.attribute("name");
                data_map[e.attribute("name")] = p;
            }
        }
        node = node.nextSibling();  // 下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
    doc.clear();
    // 排序数据点
    vector<MB_NODE> tab_config;
    tab_config.clear();

    QString protocol = QSettings("config.ini", QSettings::IniFormat).value("global/protocol", "CMU1.0").toString();
    uint protocal_ver = 3;
    if (g_proto_map.contains(protocol)) {
        protocal_ver = g_proto_map.value(protocol);
    }

#if CONFIG_METHOD_USE == CONFIG_METHOD_1
    MB_NODE* node_table;
    int node_table_size;


    if(protocal_ver == CMUV4_6){
        node_table = cmu_v4_6config;
        node_table_size = GetCMUConfigArrayLen(cmu_v4_6config);
    }else if(protocal_ver == CMUV4_10){
        node_table = cmu_v4_10config;
        node_table_size = GetCMUConfigArrayLen(cmu_v4_10config);
    }else if(protocal_ver == CMUV5_0){
        node_table = cmu_v5_0config;
        node_table_size = GetCMUConfigArrayLen(cmu_v5_0config);
    }else{
        node_table = cmu_v4_config;
        node_table_size = GetCMUConfigArrayLen(cmu_v4_config);
    }

    for (int i = 0; i < node_table_size; i++) {
        if (node_table[i].val_type != 129) continue;
        if (!data_map.contains(node_table[i].name)) continue;
        plist.append(data_map.value(node_table[i].name));
    }

#endif
#if CONFIG_METHOD_USE == CONFIG_METHOD_2
    QList<MB_NODE> node_table;
    int node_table_size = GetCMUConfigArrayLen();
    QList<db_manager::ST_DB_NODE> nodes_table;

    db_manager::Instance()->getNode(nodes_table, protocal_ver);

    for (int i = 0; i < node_table_size; i++) {

        if (!data_map.contains(cmu_config_name[i]/*.toUtf8().data()*/)) continue;

        foreach(auto val, nodes_table)
        {
            if(val.node_name == cmu_config_name[i])
            {
                if(val.val_type == 129)
                {
                    plist.append(data_map.value(cmu_config_name[i]));
                }
                else
                {
                    qDebug() << i << "<<<<<name " << cmu_config_name[i] << val.val_type;
                }
            }
        }
    }

#endif

    ui->stdSetting->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->stdSetting->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_para_model->updateData(plist);
    ui->stdSetting->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->stdSetting->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void scan_settings::on_btnWrite_released() {
    m_setMap.clear();
    foreach (auto val, m_para_model->GetData()) {
        m_setMap[val.name_cn] = val.val;
    }
    if (m_setMap.size() == 0) {
        myHelper::ShowMessageBoxError(tr("定值为空，放弃操作！"));
        return;
    }
    if (myHelper::ShowMessageBoxQuesion(tr("是否批量下发参数？")) != QDialog::Accepted) {
        return;
    }
    ip_analyze();
    if (target_ips.size()) {
        setBusy(true);
    }

    for (int i = 0; i < target_ips.size(); i++) {
        QThread* thread = new QThread();
        testWorker* task = new testWorker(target_ips.at(i), this->m_setMap, 1);
        task->moveToThread(thread);

        connect(thread, &QThread::started, task, &testWorker::doWork);
        connect(task, &testWorker::workFinished, thread, &QThread::quit);
        // automatically delete thread and task object when work is done:
        connect(task, &testWorker::workFinished, task, &testWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(task, &testWorker::workFinished, this, [this, i](int state, QString msg) {
            m_mutex.lock();
            target_count++;
            m_mutex.unlock();
            if (target_count == target_ips.size()) {
                setBusy(false);
                Toast::showTip(tr("写入完毕"));
            }
            qDebug() << i << state << msg;
            m_result_model->updateData(i, msg);
        });

        thread->start();
    }
}

void scan_settings::btnCtrlMenu() {
    QAction* b = (QAction*)sender();
    uint16_t val = 0;
    QString ip_addr;
    if (b->objectName() == "setServerIp") {
        QList<QHostAddress> list = QNetworkInterface::allAddresses();
        foreach (QHostAddress address, list) {
            // qDebug()<<address.toString();
            if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                if (address.toString().startsWith("192.168.1.")) {
                    qDebug() << address.toString();
                    val = CMD_SetIp;
                    ip_addr = address.toString();
                    break;
                }
            }
        }
        if (!val) return;
    } else if (b->objectName() == "resetServerIp") {
        val = CMD_SetIp;
        ip_addr = "192.168.1.230";
        if (!val) return;
    }
    auto actList = ui->btnCtrl->menu()->actions();
    for (int i = 0; i < actList.size(); i++) {
        QAction* action = actList.at(i);
        if (b->text() == action->text()) {
            val = CMD_UP_CMU + i;
        }
    }
    if (val == 0) {
        Toast::showTip(tr("未知命令"));
        return;
    } else if (val >= CMD_UP_CMU && val <= CMD_UP_INS) {
        if (myHelper::ShowMessageBoxQuesion(QString("%1 %2 ？").arg(tr("是否执行"), b->text())) != QDialog::Accepted) {
            return;
        }
    }
    ip_analyze();
    if (target_ips.size()) {
        setBusy(true);
    }
    m_setMap.clear();
    for (int i = 0; i < target_ips.size(); i++) {
        QThread* thread = new QThread();
        testWorker* task = new testWorker(target_ips.at(i), this->m_setMap, val);
        task->setServerIp(ip_addr);
        task->moveToThread(thread);

        connect(thread, &QThread::started, task, &testWorker::doWork);
        connect(task, &testWorker::workFinished, thread, &QThread::quit);
        // automatically delete thread and task object when work is done:
        connect(task, &testWorker::workFinished, task, &testWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(task, &testWorker::workFinished, this, [this, i](int state, QString msg) {
            m_mutex.lock();
            target_count++;
            m_mutex.unlock();
            if (target_count == target_ips.size()) {
                setBusy(false);
                Toast::showTip(tr("操作完毕"));
            }
            qDebug() << i << state << msg;
            m_result_model->updateData(i, msg);
        });
        thread->start();
        myHelper::Sleep(500);
    }
}

void scan_settings::on_connectIP_editingFinished() {
    QSettings("config.ini", QSettings::IniFormat).setValue("SCAN/iprange", ui->connectIP->text());
}

void scan_settings::on_btnUpload_released() {
    qDebug() << qtftp.isRunning();
    QStringList firmware_files;
    const QString ipAddr = "192.168.1.230";
    firmware_files << "CMU"
                   << "CMUV2"
                   << "bms"
                   << "bmu"
                   << "BMU"
                   << "INR"
                   << "BTB"
                   << "BTC";
    QList<ST_PARA> plist;
    for (int i = 0; i < firmware_files.size(); i++) {
        QString path = "firmware/" + firmware_files.at(i);
        if (QFileInfo(path).isFile()) {
            // 区分大小写
            QDir parent_dir = QFileInfo(path).dir();
            if (!parent_dir.entryList().contains(firmware_files.at(i), Qt::CaseSensitive)) continue;
            qtftp.put(path, ipAddr);
            ST_PARA p;
            p.index = i;
            p.name = firmware_files.at(i).toStdString();
            p.name_cn = tr("传输中...");
            p.type = TP_STR;
            plist << p;
        }
    }
    ui->testResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->testResult->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_result_model->updateData(plist);
    ui->testResult->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->testResult->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
/**
 * @brief 检查定值
 */
void scan_settings::on_btnTest_released() {
    m_setMap.clear();
    foreach (auto val, m_para_model->GetData()) {
        m_setMap[val.name_cn] = val.val;
    }
    if (m_setMap.size() == 0) {
        myHelper::ShowMessageBoxError(tr("定值为空，放弃操作！"));
        return;
    }
    ip_analyze();
    if (target_ips.size()) {
        setBusy(true);
    }
    for (int i = 0; i < target_ips.size(); i++) {
        QThread* thread = new QThread();
        testWorker* task = new testWorker(target_ips.at(i), this->m_setMap, 2);
        task->moveToThread(thread);

        connect(thread, &QThread::started, task, &testWorker::doWork);
        connect(task, &testWorker::workFinished, thread, &QThread::quit);
        // automatically delete thread and task object when work is done:
        connect(task, &testWorker::workFinished, task, &testWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(task, &testWorker::workFinished, this, [this, i](int state, QString msg) {
            m_mutex.lock();
            target_count++;
            m_mutex.unlock();
            if (target_count == target_ips.size()) {
                setBusy(false);
                Toast::showTip(tr("测试完毕"));
            }
            qDebug() << i << state << msg;
            m_result_model->updateData(i, msg);
        });

        thread->start();
    }
}

void scan_settings::on_btnFwCheck_released() {
    firmwareDialog* w = new firmwareDialog(nullptr);
    w->exec();
}

void scan_settings::on_btnFixTime_released()
{
    ip_analyze();
    if (target_ips.size()) {
        setBusy(true);
    }

    for (int i = 0; i < target_ips.size(); i++) {
        QThread* thread = new QThread();
        testWorker* task = new testWorker(target_ips.at(i), this->m_setMap, CMD_FixTime);
        task->moveToThread(thread);

        connect(thread, &QThread::started, task, &testWorker::doWork);
        connect(task, &testWorker::workFinished, thread, &QThread::quit);
        // automatically delete thread and task object when work is done:
        connect(task, &testWorker::workFinished, task, &testWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(task, &testWorker::workFinished, this, [this, i](int state, QString msg) {
            m_mutex.lock();
            target_count++;
            m_mutex.unlock();
            if (target_count == target_ips.size()) {
                setBusy(false);
                Toast::showTip(tr("操作完毕"));
            }
            qDebug() << i << state << msg;
            m_result_model->updateData(i, msg);
        });

        thread->start();
    }
}

