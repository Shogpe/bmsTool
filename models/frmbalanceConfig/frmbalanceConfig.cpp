#include "frmbalanceConfig.h"
#include "iconhelper.h"
#include "myhelper.h"
#include "ui_frmbalanceConfig.h"
frmbalanceConfig::frmbalanceConfig(QWidget *parent) : QDialog(parent), ui(new Ui::frmbalanceConfig) {
    ui->setupUi(this);
    this->initStyle();
    //    myHelper::moveFormToCenter(this);
}

frmbalanceConfig::~frmbalanceConfig() { delete ui; }

void frmbalanceConfig::initStyle() {
    this->setWindowTitle(tr("均衡配置"));
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
bool frmbalanceConfig::setValue(uint16_t value) {
    this->m_value = value;
    if(protocal_ver != CMU_V3){
        ui->nChannel->setValue(m_value >> 12 & 0x0F);
        ui->iBalance->setValue(m_value >> 8 & 0x0F);
        ui->tBalance->setValue(m_value & 0xFF);
    }else{
        ui->nChannel->setValue(m_value >> 11 & 0x1F);
        ui->iBalance->setValue(m_value >> 8 & 0x07);
        ui->tBalance->setValue(m_value & 0xFF);
    }

    return true;
}
void frmbalanceConfig::on_btnManually_clicked() {
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

uint16_t frmbalanceConfig::loadValue() {
    if(protocal_ver != CMU_V3){
        this->m_value = ((uint16_t)ui->nChannel->value() & 0xF) << 12 | ((uint16_t)ui->iBalance->value() & 0xF) << 8 |
                        ((uint16_t)(ui->tBalance->value()) & 0xFF);
    }else{
        this->m_value = ((uint16_t)ui->nChannel->value() & 0x1F) << 11 | ((uint16_t)ui->iBalance->value() & 0x7) << 8 |
                        ((uint16_t)(ui->tBalance->value()) & 0xFF);
    }
    return this->m_value;
}
