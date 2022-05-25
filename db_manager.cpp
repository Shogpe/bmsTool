#include "db_manager.h"
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
    QSqlDatabase dbconn = QSqlDatabase::addDatabase("SQLITECIPHER", "wxdb3");
    dbconn.setDatabaseName(file);
    dbconn.setPassword("994cd7f3625ca0083e80200e4b3f32de");
    dbconn.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; QSQLITE_ENABLE_REGEXP");
    if (!dbconn.open()) {
        qDebug() << "Can not open connection: " << dbconn.lastError().driverText();
        return false;
    }
    return true;
}
bool db_manager::getNode() {
    QSqlDatabase db = QSqlDatabase::database("wxdb3", false);
    QSqlQuery query(db);
    qDebug() << "-----TEST protocol query-----";
    QString str = QString(
                      "select node_id,node_name,reg_type,reg_addr,data_type,val_type,factor,offset from protocols "
                      "where proto_id=%1")
                      .arg(0);
    if (!query.exec(str)) {
        qDebug() << "exec failed: " << query.lastError().text();
    }
    while (query.next()) {
        qDebug() << query.value(0).toInt() << ": " << query.value(1).toString().trimmed();
    }
    return true;
}
bool db_manager::getUser(QString name, QString password) {
    bool flag = false;
    QSqlDatabase db = QSqlDatabase::database("wxdb3", false);
    qDebug() << db.isOpen() << db.isValid();
    QSqlQuery query(db);
    QString str = QString("SELECT user,level FROM user WHERE user='%1' AND password='%2';").arg(name, password);
    flag = query.exec(str);
    if (!flag) qDebug() << "exec failed: " << query.lastError().text();
//    while (query.next()) {
//        qDebug() << query.value(0).toString() << ": " << query.value(1).toInt();
//    }
    return flag;
}
void db_manager::closed() {
    QSqlDatabase db = QSqlDatabase::database("wxdb3", false);
    db.close();
}
