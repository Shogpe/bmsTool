#include "SwitchPowerConfig.h"
#include "ui_SwitchPowerConfig.h"
SwitchPowerConfig::SwitchPowerConfig(QWidget *parent) : QDialog(parent), ui(new Ui::SwitchPowerConfig) {
    ui->setupUi(this);
    this->initStyle();
    //    myHelper::moveFormToCenter(this);
}

SwitchPowerConfig::~SwitchPowerConfig() { delete ui; }

void SwitchPowerConfig::setMessage(const QString title,const QString title1, const QString title2)
{
    this->setWindowTitle(title);
    ui->lable_name1->setText(title1);
    ui->lable_name2->setText(title2);
}

void SwitchPowerConfig::initStyle() {

    //设置窗体标题栏隐藏
    //    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    this->setWindowFlags(Qt::WindowCloseButtonHint);
    //设置图形字体
    //    IconHelper::Instance()->setIcon(ui->lab_Ico, QChar(0xf015));
    //    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));
    //    //关联关闭按钮
    //    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(close()));
    //    connect(ui->btnManually, SIGNAL(clicked()), this, SLOT(close()));
    //    connect(ui->btnForce, SIGNAL(clicked()), this, SLOT(close()));
}
bool SwitchPowerConfig::setValue(uint16_t value) {
    this->m_value = value;
    uint8_t data1,data2;

    data1 = (uint8_t)value;
    data2 = (uint8_t)(value>>8);

    ui->sbox_cfg_1->setValue(data1);
    ui->sbox_cfg_2->setValue(data2);

    return true;
}
void SwitchPowerConfig::on_btnManually_clicked() {
    //    uint16_t val[4] = {0};
    //    val[0] = 0xF0A0;
    //    val[1] = 0x88;
    //    val[2] = ((uint16_t)ui->BmuID1->value() & 0xFF) << 8 | ((uint16_t)ui->CellID->value() & 0xF) << 4 |
    //             ((uint16_t)(ui->cbDirection->currentIndex() + 1) & 0xF);
    //    val[3] = ((uint16_t)(ui->doubleI->value()) & 0xFF) << 8 | ((uint16_t)ui->time->value() & 0xFF);
    //    Value.clear();
    //    Value.append(reinterpret_cast<char *>(&val), sizeof(val));
    loadValue();
    accept();
    close();
}

uint16_t SwitchPowerConfig::loadValue() {

    uint16_t tmp;
    tmp = (uint16_t)(ui->sbox_cfg_1->value() | ui->sbox_cfg_2->value()<<8);
    this->m_value = tmp;
    return this->m_value;
}
