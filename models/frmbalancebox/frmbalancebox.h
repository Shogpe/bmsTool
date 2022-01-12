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
    uint8_t getMode() const { return curMode; }
    bool setMode(uint8_t mode);

   private slots:

    void on_btnManually_clicked();

    void on_btnForce_clicked();

    void on_btnMode_clicked();

   private:
    Ui::frmBalanceBox *ui;

    void initStyle();  //初始化无边框窗体
    uint8_t curMode;
    QByteArray Value;
    void loadValue();
    void saveValue();
};

#endif  // FRMBALANCEBOX_H
