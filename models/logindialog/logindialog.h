#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class logindialog;
}

class logindialog : public QDialog
{
    Q_OBJECT

public:
    explicit logindialog(QWidget *parent = nullptr);
    ~logindialog();

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_exit_clicked();

    void on_isRemember_stateChanged(int arg1);

   private:
    Ui::logindialog *ui;
};

#endif // LOGINDIALOG_H
