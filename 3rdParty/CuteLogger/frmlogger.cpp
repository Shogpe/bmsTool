#pragma execution_character_set("utf-8")

#include "frmlogger.h"
#include <QSettings>
#include "logmanager.h"
#include "qdatetime.h"
#include "qtimer.h"
#include "ui_frmlogger.h"

frmLogger::frmLogger(QWidget *parent) : QWidget(parent), ui(new Ui::frmLogger) {
    ui->setupUi(this);
    setWindowFlag(Qt::Dialog);
    //    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    this->initForm();

    rdbList.clear();
    rdbList << ui->rb_LogtoFile << ui->rb_LogtoForm << ui->rb_LogCancel;
    foreach (QRadioButton *btn, rdbList) {
        connect(btn, &QRadioButton::clicked, this, &frmLogger::getLogDirection);
    }

    LoadLogSettings();

    // 软件没有保存重定向的方向，这样每次打开软件需要手动使能重定向,方便开发
    ui->rb_LogCancel->setChecked(true);

    // 设置是否开启日志上下文打印比如行号、函数等 此功能还未实现
    // SaveLog::Instance()->setUseContext(true);

    // 设置文件存储目录
}

frmLogger::~frmLogger() {
    this->SaveSettings();
    delete ui;
}

void frmLogger::LoadLogSettings() {
    // 设置默认的文件存储行数与上面的文件存储大小会产生优先级，行数限制优先级比文件大小的优先级高
    // 如果文件行数与文件大小都不设置初值，则会在日期改变后才会创建新文件
    QSettings *settings = new QSettings("config.ini", QSettings::IniFormat);
    ui->cboxViewRows->setCurrentIndex(settings->value("log/ViewRows", 1).toInt());

    delete settings;
}

void frmLogger::SaveSettings() {
    QSettings *settings = new QSettings("config.ini", QSettings::IniFormat);
    settings->setValue("log/ViewRows", ui->cboxViewRows->currentIndex());

    delete settings;
}

void frmLogger::initForm() {
    // 创建拦截信息与窗体追加报文的连接

    this->setWindowTitle("日志显示");

    // 添加消息类型
    QStringList types, datas;
    types << "Debug"
          << "Info"
          << "Warning"
          << "Error"
          << "Fatal";
    datas << "1"
          << "2"
          << "3"
          << "4"
          << "5";

    // 添加消息类型到列表用于勾选设置哪些类型需要重定向
    int count = types.count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(types.at(i));
        item->setData(Qt::UserRole, datas.at(i));
        item->setCheckState(Qt::Checked);
        ui->listType->addItem(item);
    }
    connect(ui->listType, &QListWidget::itemChanged, this, &frmLogger::checkItemChanged);
    // 添加日志条数

    ui->cboxViewRows->addItem("100条", 100);
    ui->cboxViewRows->addItem("300条", 300);
    ui->cboxViewRows->addItem("700条", 700);
    ui->cboxViewRows->addItem("1000条", 1000);
    ui->cboxViewRows->addItem("2000条", 2000);
    ui->cboxViewRows->addItem("10000条", 10000);
}

// 此append函数仅用于调试所用

void frmLogger::append(const QString &flag) {
    Q_UNUSED(flag);
#if 0
    if (count >= 100) {
        count = 0;
        ui->txtMain->clear();
    }

    QString str1;
    int type = ui->cboxType->currentIndex();
    if (!ui->ckSave->isChecked()) {
        if (type == 0) {
            str1 = "[D]";
        } else if (type == 1) {
            str1 = "[I]";
        } else if (type == 2) {
            str1 = "[W]";
        } else if (type == 3) {
            str1 = "[E]";
        } else if (type == 4) {
            str1 = "[F]";
        }
    }

    QString str2 = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString str3 = flag.isEmpty() ? "自动插入消息" : flag;
    QString msg = QString("%1当前时间: %2 %3").arg(str1).arg(str2).arg(str3);

    //开启网络重定向换成英文方便接收解析不乱码
    //对方接收解析的工具未必是utf8
    if (ui->ckNet->isChecked()) {
        msg = QString("%1time: %2 %3").arg(str1).arg(str2).arg("(QQ: 517216493 WX: feiyangqingyun)");
    }

    count++;
    ui->txtMain->append(msg);

    //根据不同的类型打印
    //TMD转换要分两部走不然msvc的debug版本会乱码(英文也一样)
    //char *data = msg.toUtf8().data();
    QByteArray buffer = msg.toUtf8();
    const char *data = buffer.constData();
    if (type == 0) {
        qDebug(data);
    } else if (type == 1) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        qInfo(data);
#endif
    } else if (type == 2) {
        qWarning(data);
    } else if (type == 3) {
        qCritical(data);
    } else if (type == 4) {
        //调用下面这个打印完会直接退出程序
        qFatal(data);
    }
#endif
}

void frmLogger::getLogDirection() {
    QString str;
    foreach (QRadioButton *var, rdbList) {
        if (var->isChecked()) {
            str = var->objectName();
        }
    }

    if (str == "rb_LogtoFile") {
        // logger_m::Instance()->setDirection(RedirectionType::To_File);
    } else if (str == "rb_LogtoNet") {
        // SaveLog::Instance()->setListenPort(ui->txtPort->text().toInt());
        // SaveLog::Instance()->setDirection(RedirectionType::To_Net);
    } else if (str == "rb_LogtoForm") {
        qDebug() << m_log_filter;
        LogManager::instance()->initSignalAppender();
        connect(LogManager::instance()->m_signalAppender, &SignalAppender::logMessage, this, &frmLogger::getstr, Qt::UniqueConnection);
    } else if (str == "rb_LogCancel") {
        LogManager::instance()->removeSignalAppender();
    }
}

void frmLogger::checkItemChanged(QListWidgetItem *item) {
    // 找到所有勾选的类型进行设置
    quint8 types = 0;
    int count = ui->listType->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = ui->listType->item(i);
        if (item->checkState() == Qt::Checked) {
            types |= (0x01 << item->data(Qt::UserRole).toInt());
        }
    }
    m_log_filter = types;
}

void frmLogger::getstr(const int level, const QString &content) {
    if (!(m_log_filter & (0x01 << level))) return;
    if (!ui->lineEditSearch->text().isEmpty()) {
        if (!content.contains(ui->lineEditSearch->text())) return;
    }
    QTextEdit *log = ui->txtMain;
    QString color;
    switch (level) {
        case Logger::Debug:
            color = "black";
            break;
        case Logger::Info:
            color = "#5dbe8a";
            break;
        case Logger::Warning:
            color = "#ff8c00";
            break;
        case Logger::Error:
        case Logger::Fatal:
            color = "#d80000";
            break;
        default:
            break;
    }
    log->append(QString("<font color=\"%1\">%2</font>").arg(color, content));
}

void frmLogger::on_cboxViewRows_currentIndexChanged(int index) {
    bool ok;
    int size = ui->cboxViewRows->itemData(index).toInt(&ok);

    if (ok) {
        ui->txtMain->document()->setMaximumBlockCount(size);
    }
}

void frmLogger::on_listType_itemPressed(QListWidgetItem *item) {
    // 切换选中行状态
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}
