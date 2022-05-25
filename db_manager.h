#ifndef DB_SQLITE_H
#define DB_SQLITE_H
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMutex>
#ifdef Q_OS_IOS
#include <QtPlugin>

Q_IMPORT_PLUGIN(SqliteCipherDriverPlugin)
#endif

class db_manager {
 public:
  static db_manager* Instance();

  bool start();
  bool getNode();
  bool getUser(QString name, QString password);
  void closed();

 protected:
 private:
  static db_manager* self;

};


#endif // DB_SQLITE_H
