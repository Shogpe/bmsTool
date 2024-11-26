#ifndef SWITCHPOWER_CONFIG_H
#define SWITCHPOWER_CONFIG_H

#include <QDialog>
#include "mb_cmu.h"

namespace Ui {
class SwitchPowerConfig;
}

class SwitchPowerConfig : public QDialog {
    Q_OBJECT

   public:
    explicit SwitchPowerConfig(QWidget *parent = 0);
    ~SwitchPowerConfig();

    void setMessage(const QString title,const QString title1, const QString title2);

    uint16_t getValue() const { return m_value; }
    bool setValue(uint16_t value);
    BMS_PROTOCOL protocal_ver;

   private slots:

    void on_btnManually_clicked();

   signals:
    void valueChange(QByteArray b);

   private:
    Ui::SwitchPowerConfig *ui;

    void initStyle();  //初始化无边框窗体
    uint16_t m_value;
    uint16_t loadValue();
    void saveValue(uint16_t value);
};

#endif  // FRM_BALANCEBOX_CONFIG_H
