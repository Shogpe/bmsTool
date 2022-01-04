#ifndef FRMBALANCEBOX_H
#define FRMBALANCEBOX_H

#include <QDialog>

namespace Ui {
class frmBalanceBox;
}

class frmBalanceBox : public QDialog {
    Q_OBJECT

   public:
    explicit frmBalanceBox(QWidget *parent = 0);
    ~frmBalanceBox();

    void setMessage(const QString title);

    QByteArray getValue() const { return Value; }

   private slots:

    void on_btnManually_clicked();

    void on_btnForce_clicked();

   private:
    Ui::frmBalanceBox *ui;

    void initStyle();  //初始化无边框窗体

    QByteArray Value;
};

#endif  // FRMBALANCEBOX_H
