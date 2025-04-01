#ifndef SWITCHPOWER_CONFIG_H
#define SWITCHPOWER_CONFIG_H

#include <QDialog>
#include "mb_cmu.h"

namespace Ui {
class SwitchPowerConfig;
}
#define BMU_POWER_SET   0x0101
#define PCS_POWER_SET   0x0202


class SwitchPowerConfig : public QDialog {
    Q_OBJECT

   public:
    explicit SwitchPowerConfig(QWidget *parent = 0,uint16_t m = BMU_POWER_SET);
    ~SwitchPowerConfig();

    void setMessage(const QString title,const QString title1, const QString title2);

    uint16_t getValueBmu() const { return m_value; }
    QVector<uint16_t> getValuePcs() const{ return m_data; }
    bool setValue(QVector<uint16_t> data);
    bool setValue(uint16_t value);
    BMS_PROTOCOL protocal_ver;
    uint16_t mode = 0x0000;
   private slots:

    void on_btnManually_clicked();

   signals:
    void valueChange(QByteArray b);

   private:
    Ui::SwitchPowerConfig *ui;

    void initStyle();  //初始化无边框窗体
    uint16_t m_value;
    QVector<uint16_t> m_data;
    void loadValue();
    void saveValue(uint16_t value);
};

#endif  // FRM_BALANCEBOX_CONFIG_H
