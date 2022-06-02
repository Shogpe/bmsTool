#include "scan_settings.h"
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>
#include "Toast.h"
#include "myhelper.h"
#include "socImporter.h"
#include "ui_scan_settings.h"
scan_settings::scan_settings(QWidget* parent) : QWidget(parent), ui(new Ui::scan_settings) {
    ui->setupUi(this);
    m_mbtcp = nullptr;
    //
    connect(ui->tbtnConnect, &QPushButton::released, this, &scan_settings::flushData);
}

scan_settings::~scan_settings() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    delete ui;
}

void scan_settings::uiInit() {
    /* 设置 tableWidget */
    //  tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    //  tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    QStringList hdr_list;

    ui->tableBMU->setColumnCount(hdr_list.size());
    ui->tableBMU->setHorizontalHeaderLabels(hdr_list);
    ui->tableBMU->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableBMU->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个

    //定值显示和隐藏
    //    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
    //    foreach (QDoubleSpinBox* dspbox, dspboxs) {
    //        dspbox->hide();
    //        map<string, NodeReg>::iterator iter1;
    //        iter1 = mycmu->name_map.find(dspbox->objectName().toStdString());
    //        if (iter1 != mycmu->name_map.end()) {
    //            try {
    //                dspbox->show();
    //            } catch (exception& e) {
    //                qDebug() << e.what();
    //            }
    //        }
    //    }
}
void scan_settings::flushData() {
    if (m_mbtcp == nullptr) {
        m_mbtcp = new mb_tcp(ui->connectIP->text(), ui->spinBoxPort->value());
    }
    vector<MB_NODE> tab_config;
    tab_config.clear();
    MB_NODE* node_table = cmu_v4_config;
    int node_table_size = cmu_v4_config_len;
    for (int i = 0; i < node_table_size; i++) {
        tab_config.push_back(node_table[i]);
    }
    m_mbtcp->init_config(tab_config);
    m_mbtcp->Connect();
    vector<ST_NODE_DATA> data = m_mbtcp->ReadALL();
    for (int i = 0; i < data.size(); i++) {
        qDebug() << i << cmu_v4_config[i].name << data.at(i).sysData.val.f64;
    };
    m_mbtcp->close();
}
