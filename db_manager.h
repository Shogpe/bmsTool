#ifndef DB_SQLITE_H
#define DB_SQLITE_H
#include <QDebug>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#ifdef Q_OS_IOS
#include <QtPlugin>

Q_IMPORT_PLUGIN(SqliteCipherDriverPlugin)
#endif

class db_manager {
   public:
    static db_manager* Instance();
    typedef struct {
        int evt_code;
        QString evt_txt;
        QString evt_id;
        QString evt_dt;
        QString evt_threshold;
        QString code;
    } ST_DB_SOE;
    bool start();
    bool getNode();
    bool getSOE(QMap<int, ST_DB_SOE> &soe_map, int tag);
    bool getUser(QString name, QString password, int&);
    void closed();

   protected:
   private:
    static db_manager* self;
};

#endif  // DB_SQLITE_H
