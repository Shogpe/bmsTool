#ifndef UIDEMO08_H
#define UIDEMO08_H

#include <QWidget>
#include "bmsview.h"
#include "mb_cmu.h"
#include "models/ListView/ListView.h"
#include "myhelper.h"
#include "tftpserver.h"
#include "notifymanager.h"
#define EXIT_CODE_REBOOT 123456789
class QToolButton;

namespace Ui {
class MainUI;
}

class MainUI : public QWidget {  // public QFramelessWidget {
    Q_OBJECT

   public:
    explicit MainUI(QWidget *parent = nullptr);
    ~MainUI();

   protected:
    void closeEvent(QCloseEvent *event);

   private:
    Ui::MainUI *ui;
    QList<int> pixCharConfig;
    QList<QToolButton *> btnsConfig;
    QTimer *timer;
    // StringListModel list_model;
    QMenu *title_menu;
    QMenu *langue_menu;
    QMenu *theme_menu;

    QList<QAction *> updateActs;
    QActionGroup *langueGroup;
    QActionGroup *themeGroup;
    QAction *setBlue;
    QAction *setBlack;
    QAction *setWhite;

    TFTPServer *tftpd;
    NotifyManager *manager;
    //    bool eventFilter(QObject *obj, QEvent *event);
    //
    QSettings *settings;
   private slots:
    void initForm();
    void buttonClick();

    //    void on_btnMenu_Min_clicked();
    //    void on_btnMenu_Max_clicked();
    //    void on_btnMenu_Close_clicked();

    void menuClick();
    void changeTheme();
};

#endif  // UIDEMO08_H
