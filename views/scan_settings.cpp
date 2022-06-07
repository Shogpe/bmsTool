#include "scan_settings.h"
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtGlobal>
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
    QMap<QString, double> setMap;
    while (!node.isNull())  //如果节点不空
    {
        if (node.isElement())  //如果节点是元素
        {
            QDomElement e = node.toElement();  //转换为元素，注意元素和节点是两个数据结构，其实差不多
            if (e.attribute("name") != nullptr) {
                setMap[e.attribute("name")] = e.attribute("value").toDouble();
            }
        }
        node = node.nextSibling();  //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
    doc.clear();
    //
    if (m_mbtcp == nullptr) {
        m_mbtcp = new mb_tcp(ui->connectIP->text(), ui->spinBoxPort->value());
    }
    vector<MB_NODE> tab_config;
    tab_config.clear();
    MB_NODE* node_table = cmu_v4_config;
    int node_table_size = cmu_v4_config_len;
    for (int i = 0; i < node_table_size; i++) {
        if (node_table[i].val_type != 129) continue;
        node_table[i].index = tab_config.size();
        tab_config.push_back(node_table[i]);
    }
    m_mbtcp->init_config(tab_config);
    m_mbtcp->Connect();
    vector<ST_NODE_DATA> data = m_mbtcp->ReadALL();
    for (int i = 0; i < data.size(); i++) {
        if (setMap.contains(tab_config.at(i).name)) {
            qDebug() << i << tab_config.at(i).name << data.at(i).sysData.val.f64 << setMap.value(tab_config.at(i).name)
                     << qFuzzyCompare(data.at(i).sysData.val.f64, setMap.value(tab_config.at(i).name));
        }
    };
    m_mbtcp->close();
    m_mbtcp->deleteLater();
    m_mbtcp = nullptr;
}
