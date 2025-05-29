#include "db_manager.h"
#include <QThread>
#include "myhelper.h"
QMutex mutex;
db_manager *db_manager::self = nullptr;
db_manager *db_manager::Instance() {
    if (!self) {
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new db_manager;
            self->start();
        }
    }
    return self;
}

bool db_manager::start() {
    QString file = "data.db3";
    if (QFile(file).size() <= 4) {
        myHelper::ShowMessageBoxError(QObject::tr("数据库文件不存在!请联系软件提供商协助处理。"));
        abort();
        return false;
    }
    QSqlDatabase dbconn = QSqlDatabase::addDatabase("SQLITECIPHER", "wxdb3");
    dbconn.setDatabaseName(file);
    dbconn.setPassword("994cd7f3625ca0083e80200e4b3f32de");
    dbconn.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; QSQLITE_ENABLE_REGEXP");
    if (!dbconn.open()) {
        qCritical() << "Can not open connection: " << dbconn.lastError().driverText();
        myHelper::ShowMessageBoxError(QObject::tr("数据无法读取:") + dbconn.lastError().driverText());
        return false;
    }
    return true;
}
bool db_manager::getOriginNode(QList<ST_DB_NODE> &list, int proto_id) {
    list.clear();
    QString connect_name = QString("conn_%1").arg(int(QThread::currentThreadId()));
    if (!QSqlDatabase::contains(connect_name)) {
        QString file = "data.db3";
        QSqlDatabase dbconn = QSqlDatabase::addDatabase("SQLITECIPHER", connect_name);
        dbconn.setDatabaseName(file);
        dbconn.setPassword("994cd7f3625ca0083e80200e4b3f32de");
        dbconn.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; QSQLITE_ENABLE_REGEXP");
        if (!dbconn.open()) {
            qDebug() << "Can not open connection: " << dbconn.lastError().driverText();
            return false;
        }
    }
    QSqlDatabase db = QSqlDatabase::database(connect_name, false);
    QSqlQuery query(db);
    QString str =
        QString(
            "SELECT node_id,node_name,reg_type,reg_addr,data_type,val_type,factor,[offset],unit FROM protocols "
            "WHERE (proto_id=%1 and ex_ver=0)")
            .arg(proto_id);
    qDebug() << "-----加载基础点表-----";
    qInfo() << str;
    if (!query.exec(str)) {
        qDebug() << "exec failed: " << query.lastError().text();
        return false;
    }
    ST_DB_NODE node;
    while (query.next()) {
        node.node_id = query.value(0).toInt();
        node.node_name = query.value(1).toString().trimmed();
        node.reg_type = query.value(2).toUInt();
        node.reg_addr = query.value(3).toUInt();
        node.data_type = query.value(4).toUInt();
        node.val_type = query.value(5).toUInt();
        node.factor = query.value(6).toDouble();
        node.offset = query.value(7).toDouble();
        node.unit = query.value(8).toString().trimmed();
        list.append(node);
    }
    return true;
}

bool db_manager::getExternNode(QList<ST_DB_NODE> &list,int proto_id, int ex_ver) {
    list.clear();
    QString connect_name = QString("conn_%1").arg(int(QThread::currentThreadId()));
    if (!QSqlDatabase::contains(connect_name)) {
        QString file = "data.db3";
        QSqlDatabase dbconn = QSqlDatabase::addDatabase("SQLITECIPHER", connect_name);
        dbconn.setDatabaseName(file);
        dbconn.setPassword("994cd7f3625ca0083e80200e4b3f32de");
        dbconn.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; QSQLITE_ENABLE_REGEXP");
        if (!dbconn.open()) {
            qDebug() << "Can not open connection: " << dbconn.lastError().driverText();
            return false;
        }
    }
    QSqlDatabase db = QSqlDatabase::database(connect_name, false);
    QSqlQuery query(db);
    QString str =
        QString(
            "SELECT node_id,node_name,reg_type,reg_addr,data_type,val_type,factor,[offset],unit FROM protocols "
            "WHERE (proto_id=%1 and (ex_ver=%2 or ex_ver=0))")
            .arg(proto_id).arg(ex_ver);
    qDebug() << "-----加载扩展点表-----";
    qInfo() << str;
    if (!query.exec(str)) {
        qDebug() << "exec failed: " << query.lastError().text();
        return false;
    }
    ST_DB_NODE node;
    while (query.next()) {
        node.node_id = query.value(0).toInt();
        node.node_name = query.value(1).toString().trimmed();
        node.reg_type = query.value(2).toUInt();
        node.reg_addr = query.value(3).toUInt();
        node.data_type = query.value(4).toUInt();
        node.val_type = query.value(5).toUInt();
        node.factor = query.value(6).toDouble();
        node.offset = query.value(7).toDouble();
        node.unit = query.value(8).toString().trimmed();
        list.append(node);
    }
    return true;
}
bool db_manager::getUser(QString name, QString password, int &level) {
    bool flag = false;
    qDebug() << name << level;
    QString connect_name = QString("conn_%1").arg(int(QThread::currentThreadId()));
    if (!QSqlDatabase::contains(connect_name)) {
        QString file = "data.db3";
        QSqlDatabase dbconn = QSqlDatabase::addDatabase("SQLITECIPHER", connect_name);
        dbconn.setDatabaseName(file);
        dbconn.setPassword("994cd7f3625ca0083e80200e4b3f32de");
        dbconn.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; QSQLITE_ENABLE_REGEXP");
        if (!dbconn.open()) {
            qDebug() << "Can not open connection: " << dbconn.lastError().driverText();
            return false;
        }
    }
    QSqlDatabase db = QSqlDatabase::database(connect_name, true);
    qDebug() << db.isOpen() << db.isValid();
    if ((!db.isValid()) && (!db.isOpen())) {
        if (!db.open()) {
            qDebug().noquote() << "Create connection error:" << db.lastError().text();
        }
    }
    if ((!db.isValid()) || (!db.isOpen())) return false;
    //    qDebug() << db.isOpen() << db.isValid();
    QSqlQuery query(db);
    QString str = QString("SELECT user,level FROM user WHERE user='%1' AND password='%2'").arg(name, password);
    flag = query.exec(str);
    //    qDebug() << str << flag;
    if (!flag) qDebug() << "exec failed: " << query.lastError().text();
    while (query.next()) {
        qDebug() << query.value(0).toString() << ": " << query.value(1).toInt();
        level = query.value(1).toInt();
        return true;
    }
    return false;
}

bool db_manager::getSOE(QMap<int, ST_DB_SOE> &soe_map, SOE_TAG tag) {
    bool flag = false;
    soe_map.clear();
    QString connect_name = QString("conn_%1").arg(int(QThread::currentThreadId()));
    QSqlDatabase db = QSqlDatabase::database(connect_name, true);
    //    qDebug() << db.isOpen() << db.isValid();
    QString locale = myHelper::GetAppValue("locale", "zh_CN").toString();
    locale = "soe_codec_" + locale;
    QSqlQuery query(db);
    QString str =
        QString("SELECT evt_code,evt_txt,evt_id,evt_dt,evt_threshold,code FROM %2 WHERE tag=%1").arg(tag).arg(locale);
    flag = query.exec(str);
    //    qDebug() << str << flag;
    if (!flag) qDebug() << "exec failed: " << query.lastError().text();
    while (query.next()) {
        ST_DB_SOE one;
        one.evt_code = query.value(0).toInt();
        one.evt_txt = query.value(1).toString();
        one.evt_id = query.value(2).toString();
        one.evt_dt = query.value(3).toString();
        one.evt_threshold = query.value(4).toString();
        one.code = query.value(5).toString();
        qDebug() << one.evt_id;
        soe_map[one.evt_code] = one;
    }
    return false;
}
void db_manager::closed() {
    //    QSqlDatabase db = QSqlDatabase::database("wxdb3", false);
    //    db.close();
    QSqlDatabase::removeDatabase("wxdb3");
}
