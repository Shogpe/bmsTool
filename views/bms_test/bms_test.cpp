#include "bms_test.h"
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
#include "ui_bms_test.h"

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
    uiInit();
    m_mbtcp = nullptr;
    m_timer = new QTimer();
    //
    connect(ui->btnLoadXml, &QPushButton::released, this, &scan_settings::loadXml);
    connect(&qtftp, &Qtftp::fileSent, this, [this](int ret, QString file) {
        qDebug() << "文件:" << file << ((ret == 0) ? " 传输成功" : " 传输失败");
        auto list = m_result_model->GetData();
        for (int i = 0; i < list.size(); i++) {
            if (file == QString::fromStdString(list.at(i).name))
                m_result_model->updateData(i, (ret == 0) ? "传输成功" : "传输失败");
        }
    });
    ui->srvStatus->setText(tr("服务检测中..."));
    m_timer->setInterval(1000);
    connect(&m_thread, &QThread::started, this->m_timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_timer, &QTimer::timeout, this, &scan_settings::checkServer, Qt::DirectConnection);
    connect(&m_thread, &QThread::finished, m_timer, &QTimer::stop);
    connect(&m_thread, &QThread::finished, m_timer, &QTimer::deleteLater);
    connect(this, &scan_settings::checkRespond, this, [this](int result) {
        switch (result) {
            case 1:
                ui->srvStatus->setText("远程服务在线");
                ui->srvStatus->setStyleSheet("color:green;");
                ui->srvStatus->setToolTip("可以上传固件至远程服务器");
                ui->btnUpload->setDisabled(false);
                break;
            case 2:
                ui->srvStatus->setText("本机服务在线");
                ui->srvStatus->setStyleSheet("color:green;");
                ui->srvStatus->setToolTip("请查看下方状态栏，检查服务器是否启动成功");
                ui->btnUpload->setDisabled(true);
                break;
            default:
                ui->srvStatus->setText("远程服务离线");
                ui->srvStatus->setStyleSheet("color:red;text-decoration:underline;");
                ui->srvStatus->setToolTip("可以修改IP以启用本地服务器");
                ui->btnUpload->setDisabled(true);
                break;
        }
    });
    m_timer->moveToThread(&m_thread);
    m_thread.start();
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
        QString network_cmd = "ping 192.168.1.230 -n 1";
        QString result;
        QProcess network_process;                                                  //不要加this
        network_process.start(network_cmd);                                        //调用ping 指令
        network_process.waitForFinished();                                         //等待指令执行完毕
        result = network_process.readAll();                                        //获取指令执行结果
                                                                                   //        qDebug() << result;
        if (result.contains(QString("TTL=")) || result.contains(QString("ttl=")))  //若包含TTL=字符串则认为网络在线
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
    this->setWindowTitle("BMS维护工具");
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
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);  //选中行
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
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);  //选中行
    tableView->horizontalHeader()->setStretchLastSection(true);

    //
    tableView->setItemDelegateForColumn(0, readOnlyDelegate);
    tableView->setItemDelegateForColumn(1, readOnlyDelegate);
    tableView->setItemDelegateForColumn(2, readOnlyDelegate);

    tableView->setModel(m_result_model);

    QMenu* update_menu = new QMenu;
    update_menu->addAction("下载升级BMS", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("下载升级BMU", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("下载升级绝缘板", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("读取BMS版本号", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("读取绝缘版本号", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("BMU拨码锁定⚿", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("BMU拨码解锁", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("恢复校准参数", this, &scan_settings::btnCtrlMenu);
    update_menu->addAction("恢复运行参数", this, &scan_settings::btnCtrlMenu);
    ui->btnCtrl->setMenu(update_menu);

    QMenu* btn_menu = new QMenu;
    btn_menu->addAction("设置服务IP为本地", this, &scan_settings::btnCtrlMenu);
    btn_menu->actions().constLast()->setObjectName("setServerIp");
    btn_menu->addAction("恢复默认服务IP", this, &scan_settings::btnCtrlMenu);
    btn_menu->actions().constLast()->setObjectName("resetServerIp");
    ui->btnSetIp->setMenu(btn_menu);
    ui->btnSetIp->setHidden(true);
    QString ipRange =
        QSettings("config.ini", QSettings::IniFormat).value("SCAN/iprange", "192.168.1.121-192.168.1.128").toString();
    ui->connectIP->setText(ipRange);
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
    QDomElement root = doc.documentElement();  //返回根节点
    QDomNode node = root.firstChild();         //获得第一个子节点
    QList<ST_PARA> plist;
    QMap<QString, ST_PARA> data_map;
    while (!node.isNull())  //如果节点不空
    {
        if (node.isElement())  //如果节点是元素
        {
            QDomElement e = node.toElement();  //转换为元素，注意元素和节点是两个数据结构，其实差不多
            if ((e.attribute("name") != nullptr) && (e.attribute("name_cn") != nullptr)) {
                ST_PARA p;
                p.name = e.attribute("name_cn").toStdString();
                p.val = e.attribute("value").toDouble();
                p.name_cn = e.attribute("name").toStdString();
                data_map[e.attribute("name")] = p;
            }
        }
        node = node.nextSibling();  //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
    doc.clear();
    // 排序数据点
    vector<MB_NODE> tab_config;
    tab_config.clear();
    MB_NODE* node_table = cmu_v4_config;
    int node_table_size = cmu_v4_config_len;
    for (int i = 0; i < node_table_size; i++) {
        if (node_table[i].val_type != 129) continue;
        if (!data_map.contains(node_table[i].name)) continue;
        plist.append(data_map.value(node_table[i].name));
    }
    ui->stdSetting->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->stdSetting->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_para_model->updateData(plist);
    ui->stdSetting->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->stdSetting->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void scan_settings::on_btnWrite_released() {
    if (myHelper::ShowMessageBoxQuesion("是否批量下发参数？") != QDialog::Accepted) {
        return;
    }
    ip_analyze();
    if (target_ips.size()) {
        setBusy(true);
    }
    m_setMap.clear();
    foreach (auto val, m_para_model->GetData()) { m_setMap[QString().fromStdString(val.name_cn)] = val.val; }
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
                Toast::showTip("写入完毕");
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
        Toast::showTip("未知命令");
        return;
    } else if (val >= CMD_UP_CMU && val <= CMD_UP_INS) {
        if (myHelper::ShowMessageBoxQuesion(QString("是否执行 %1 ？").arg(b->text())) != QDialog::Accepted) {
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
                Toast::showTip("操作完毕");
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
            p.name_cn = "传输中...";
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
    ip_analyze();
    if (target_ips.size()) {
        setBusy(true);
    }
    m_setMap.clear();
    foreach (auto val, m_para_model->GetData()) { m_setMap[QString().fromStdString(val.name_cn)] = val.val; }
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
                Toast::showTip("测试完毕");
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
