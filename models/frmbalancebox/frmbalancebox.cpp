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
enum BALANCE_MODE {
    BALANCE_STOP = 0x0,
    BALANCE_FORCE = 0x55,
    BALANCE_MANUAL = 0x88,
    BALANCE_AUTO = 0xAA,
    BALANCE_CHG = 0xA5,  // 均衡补电
};
void frmBalanceBox::initStyle() {
    this->setWindowTitle(tr("均衡控制"));
    loadValue();
    // 设置窗体标题栏隐藏
    //     this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    this->setWindowFlags(Qt::WindowCloseButtonHint);
    // 设置图形字体
    //     IconHelper::Instance()->setIcon(ui->lab_Ico, QChar(0xf015));
    //     IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));
    //     //关联关闭按钮
    //     connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(close()));
    //     connect(ui->btnManually, SIGNAL(clicked()), this, SLOT(close()));
    //     connect(ui->btnForce, SIGNAL(clicked()), this, SLOT(close()));
    ui->cbMode->addItem(tr("停止均衡"), BALANCE_STOP);
    ui->cbMode->addItem(tr("自动均衡"), BALANCE_AUTO);
    ui->cbMode->addItem(tr("强制均衡"), BALANCE_FORCE);
    ui->cbMode->addItem(tr("手动均衡"), BALANCE_MANUAL);
    ui->cbMode->addItem(tr("均衡补电"), BALANCE_CHG);
    ui->gManual->hide();
    curMode = ui->cbMode->currentData().toUInt();
    connect(ui->cbMode, &QComboBox::currentTextChanged, this, &frmBalanceBox::modeChange);
}

void frmBalanceBox::modeChange() {
    curMode = ui->cbMode->currentData().toUInt();

    switch (curMode) {
        case BALANCE_FORCE:
            ui->gManual->setTitle(tr("强制均衡参数"));
            ui->wTagetU->show();
            ui->wCellID->hide();
            ui->wChgI->hide();
            ui->gManual->show();
            ui->btnManually->setText(tr("开启强制均衡"));
            break;
        case BALANCE_MANUAL:
            ui->gManual->setTitle(tr("手动均衡参数"));
            ui->wTagetU->hide();
            ui->wCellID->show();
            ui->wChgI->show();
            ui->time->setSuffix(" S");
            ui->gManual->show();
            ui->btnManually->setText(tr("开启手动均衡"));
            break;
        case BALANCE_CHG:
            ui->gManual->setTitle(tr("均衡补电参数"));
            ui->wTagetU->show();
            ui->cbClose->hide();
            ui->wChgI->show();
            ui->wCellID->show();
            ui->time->setSuffix(" x10Min");
            ui->gManual->show();
            ui->btnManually->setText(tr("开启均衡补电"));
            break;
        case BALANCE_AUTO:
            ui->gManual->hide();
            break;
        case BALANCE_STOP:
        default:
            ui->gManual->hide();
            break;
    }
}

void frmBalanceBox::saveValue() {
    QSettings *settings = new QSettings("config.ini", QSettings::IniFormat);
    settings->value("balance/BMUID", "1").toInt();
    settings->setValue("balance/BMUID1", ui->BmuID1->value());
    settings->setValue("balance/cellId", ui->CellID->value());
    settings->setValue("balance/chg", ui->cbDirection->currentIndex());
    settings->setValue("balance/Icell", ui->doubleI->value());
    settings->setValue("balance/delay", ui->time->value());
    settings->setValue("balance/Ucell", ui->targetU->value());
    delete settings;
    emit valueChange(Value);
}
void frmBalanceBox::loadValue() {
    QSettings *settings = new QSettings("config.ini", QSettings::IniFormat);
    ui->BmuID1->setValue(settings->value("balance/BMUID1", "0").toInt());
    ui->CellID->setValue(settings->value("balance/cellId", "0").toInt());
    ui->cbDirection->setCurrentIndex(settings->value("balance/chg", "1").toInt());
    ui->doubleI->setValue(settings->value("balance/Icell", "2").toDouble());
    ui->time->setValue(settings->value("balance/delay", "10").toInt());
    ui->targetU->setValue(settings->value("balance/Ucell", "3.3").toDouble());
    delete settings;
}
void frmBalanceBox::on_btnManually_clicked() {
    curMode = ui->cbMode->currentData().toUInt();

    switch (curMode) {
        case BALANCE_FORCE: {
            uint16_t val[4] = {0};
            val[0] = 0xF0A0;
            val[1] = 0x55;
            if (ui->cbClose->isChecked()) {
                val[2] = 0x55 << 8 | ((uint16_t)ui->BmuID1->value() & 0xFF);
            } else {
                val[2] = 0xAA << 8 | ((uint16_t)ui->BmuID1->value() & 0xFF);
            }
            val[3] = ui->targetU->value() * 10000;
            Value.clear();
            Value.append(reinterpret_cast<char *>(&val), sizeof(val));
            this->saveValue();
        } break;
        case BALANCE_MANUAL: {
            uint16_t val[4] = {0};
            val[0] = 0xF0A0;
            val[1] = 0x88;
            val[2] = ((uint16_t)ui->BmuID1->value() & 0xFF) << 8 | ((uint16_t)ui->CellID->value() & 0xF) << 4 |
                     ((uint16_t)(ui->cbDirection->currentIndex() + 1) & 0xF);
            val[3] = ((uint16_t)(ui->doubleI->value()) & 0xFF) << 8 | ((uint16_t)ui->time->value() & 0xFF);
            Value.clear();
            Value.append(reinterpret_cast<char *>(&val), sizeof(val));
            this->saveValue();
        } break;
        case BALANCE_CHG: {
            uint16_t val[5] = {0};
            val[0] = 0xF0A0;
            val[1] = 0xA5;
            val[2] = ((uint16_t)ui->BmuID1->value() & 0xFF) << 8 | ((uint16_t)ui->CellID->value() & 0xF) << 4 |
                     ((uint16_t)(ui->cbDirection->currentIndex() + 1) & 0xF);
            val[3] = ((uint16_t)(ui->doubleI->value()) & 0xFF) << 8 | ((uint16_t)ui->time->value() & 0xFF);
            val[4] = ui->targetU->value() * 10000;
            Value.clear();
            Value.append(reinterpret_cast<char *>(&val), sizeof(val));
            this->saveValue();
        } break;
        case BALANCE_AUTO:
        case BALANCE_STOP:
        default: {
            Value.clear();
            this->saveValue();
        } break;
    }
}
bool frmBalanceBox::setMode(uint8_t mode) {
    for (int i = 0; i < ui->cbMode->count(); i++) {
        if (mode == ui->cbMode->itemText(i)) {
            ui->cbMode->setCurrentIndex(i);
            return true;
        }
    }
    //    ui->cbMode->setCurrentIndex(0);
    return false;
}

void frmBalanceBox::on_btnMode_clicked() {
    Value.clear();
    this->saveValue();
}
