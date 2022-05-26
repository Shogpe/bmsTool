#ifndef FRM_BALANCEBOX_CONFIG_H
#define FRM_BALANCEBOX_CONFIG_H

#include <QDialog>

namespace Ui {
class frmbalanceConfig;
}

class frmbalanceConfig : public QDialog {
    Q_OBJECT

   public:
    explicit frmbalanceConfig(QWidget *parent = 0);
    ~frmbalanceConfig();

    void setMessage(const QString title);

    uint16_t getValue() const { return m_value; }
    bool setValue(uint16_t value);

   private slots:

    void on_btnManually_clicked();

   signals:
    void valueChange(QByteArray b);

   private:
    Ui::frmbalanceConfig *ui;

    void initStyle();  //初始化无边框窗体
    uint16_t m_value;
    uint16_t loadValue();
    void saveValue(uint16_t value);
};

#endif  // FRM_BALANCEBOX_CONFIG_H
