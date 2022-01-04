#include "frmbalancebox.h"
#include "iconhelper.h"
#include "myhelper.h"
#include "ui_frmbalancebox.h"
frmBalanceBox::frmBalanceBox(QWidget *parent) : QDialog(parent), ui(new Ui::frmBalanceBox) {
    ui->setupUi(this);
    this->initStyle();
    //    myHelper::moveFormToCenter(this);
}

frmBalanceBox::~frmBalanceBox() { delete ui; }

void frmBalanceBox::initStyle() {
    this->setWindowTitle("均衡控制");
    //设置窗体标题栏隐藏
    //    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    this->setWindowFlags(Qt::WindowCloseButtonHint);
    //设置图形字体
    //    IconHelper::Instance()->setIcon(ui->lab_Ico, QChar(0xf015));
    //    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));
    //    //关联关闭按钮
    //    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnManually, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnForce, SIGNAL(clicked()), this, SLOT(close()));
}

void frmBalanceBox::on_btnManually_clicked() {
    uint16_t val[4] = {0};
    val[0] = 0xF0A0;
    val[1] = 0x66;
    val[2] = ((uint16_t)ui->BmuID1->value() && 0xFF) << 8 | ((uint16_t)ui->CellID->value() && 0xF) << 4 |
             ((uint16_t)(ui->cbDirection->currentIndex() + 1) && 0xF);
    val[3] = ((uint16_t)(ui->doubleI->value() * 10) && 0xFF) << 8 | ((uint16_t)ui->time->value() && 0xFF);
    Value.clear();
    Value.append(reinterpret_cast<char *>(&val), sizeof(val));
    done(1);
    this->close();
}

void frmBalanceBox::on_btnForce_clicked() {
    uint16_t val[4] = {0};
    val[0] = 0xF0A0;
    val[1] = 0xBB;
    if (ui->cbClose) {
        val[2] = 0x55 << 8 | ((uint16_t)ui->BmuID1->value() && 0xFF);
    } else {
        val[2] = 0xAA << 8 | ((uint16_t)ui->BmuID1->value() && 0xFF);
    }
    val[3] = ui->targetU->value() * 10000;
    Value.clear();
    Value.append(reinterpret_cast<char *>(&val), sizeof(val));
    done(1);
    this->close();
}
