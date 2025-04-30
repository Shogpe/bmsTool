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
    enum SOE_TAG {
        SOE_BMS1 = 1,
        SOE_BMS2 = 2,
        SOE_BMS3 = 3,
//        SOE_RTU1 = 4,
        SOE_BMS4 = 4,
        SOE_RTU1 = 5,
    };
    enum USER_LEVEL{
        LEVEL_ERROR_L = 0,
        LEVEL_GUEST = 1,
        LEVEL_SUPER = 2,
        LEVEL_DEBUG = 3,
        LEVEL_ERROR_H = 31,
    };
    static bool isUserLevelValid()
    {
        return (db_manager::Instance()->userLevel() > LEVEL_ERROR_L && db_manager::Instance()->userLevel() != LEVEL_ERROR_H);
    }

    typedef struct {
        int node_id;
        QString node_name;
        uint16_t reg_type;
        uint16_t reg_addr;
        uint16_t data_type;
        uint16_t val_type;
        double factor;
        double offset;
        QString unit;
    } ST_DB_NODE;
    bool start();
    bool getNode(QList<ST_DB_NODE>& list, int proto_id);
    bool getSOE(QMap<int, ST_DB_SOE>& soe_map, SOE_TAG tag);
    bool getUser(QString name, QString password, int&);
    void closed();
    // 全局变量
    void setUserName(QString name) { usrName = name; }
    QString userName() { return usrName; }
    void setUserLevel(int level) { usrLevel = level; }
    int userLevel() { return usrLevel; }

   protected:
    QString usrName;
    int usrLevel;

   private:
    static db_manager* self;
};

#endif  // DB_SQLITE_H
