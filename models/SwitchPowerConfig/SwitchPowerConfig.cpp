#include "SwitchPowerConfig.h"
#include "ui_SwitchPowerConfig.h"

#define COL_IDX      0
#define COL_AC       1
#define COL_SW       2
#define COL_ONOFF    3
#define COL_PCSAC    4
#define COL_VALUE    5

typedef struct
{
    uint16_t row;
    uint16_t reg;
    bool     isH;
}regMap_t;
const regMap_t rowToRegMap[10] =
{
    {1 ,5450,true},
    {2 ,5454,false},
    {3 ,5450,false},
    {4 ,5455,false},
    {5 ,5452,false},
    {6 ,5451,true},
    {7 ,5454,true},
    {8 ,5451,false},
    {9 ,5455,true},
    {10,5452,true},
};

SwitchPowerConfig::SwitchPowerConfig(QWidget *parent, uint16_t m) : QDialog(parent), ui(new Ui::SwitchPowerConfig) {
    ui->setupUi(this);
    this->initStyle();

    mode = m;
    if(mode == PCS_POWER_SET)
    {
        ui->lable_name1->hide();
        ui->lable_name2->hide();
        ui->sbox_cfg_1->hide();
        ui->sbox_cfg_2->hide();
        ui->widget_spacer->hide();
        ui->tableWidget->show();


        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(COL_IDX, QHeaderView::ResizeToContents);
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(COL_SW, QHeaderView::ResizeToContents);
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(COL_VALUE, QHeaderView::ResizeToContents);
        for(int r = 0; r < ui->tableWidget->rowCount(); r++)
        {
            for(int c = 0; c < COL_VALUE; c++)
            {
                QTableWidgetItem* item = ui->tableWidget->item(r,c);
                if(item->text()==tr("高压箱有AC")
                ||item->text()==tr("合闸")
                ||item->text()==tr("PCS开机")
                ||item->text()==tr("PCS有AC"))
                {
                    item->setTextColor(Qt::darkGreen);
                }
                if(item->text()==tr("高压箱无AC")
                ||item->text()==tr("分闸")
                ||item->text()==tr("PCS关机")
                ||item->text()==tr("PCS无AC"))
                {
                    item->setTextColor(Qt::darkRed);
                }
                item->setFlags(item->flags()&(~Qt::ItemIsEnabled));
            }

        }
    }
    else
    {
        mode = BMU_POWER_SET;
        ui->lable_name1->show();
        ui->lable_name2->show();
        ui->sbox_cfg_1->show();
        ui->sbox_cfg_2->show();
        ui->widget_spacer->show();
        ui->tableWidget->hide();
    }
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


bool SwitchPowerConfig::setValue(QVector<uint16_t> data)
{
    m_data = data;
    qDebug()<<">>>" << data;
    if(data.count() == 0 || data.count()%2 == 1)
    {
        return false;
    }
    for(int i = 0; i < m_data.count()/2; i++)
    {
        uint16_t reg = m_data[i*2];
        uint16_t val = m_data[i*2+1];

        for(int mapIdx = 0; mapIdx < 10; mapIdx++)
        {
            if((reg == rowToRegMap[mapIdx].reg) && (rowToRegMap[mapIdx].isH == false))
            {
                uint16_t row = rowToRegMap[mapIdx].row;
                QTableWidgetItem* item = ui->tableWidget->item(row,COL_VALUE);
                item->setText(QString::number((val>>0)&0xFF));
            }
            if((reg == rowToRegMap[mapIdx].reg) && (rowToRegMap[mapIdx].isH == true))
            {
                uint16_t row = rowToRegMap[mapIdx].row;
                QTableWidgetItem* item = ui->tableWidget->item(row,COL_VALUE);
                item->setText(QString::number((val>>8)&0xFF));
            }
        }
    }
    return true;
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

void SwitchPowerConfig::loadValue() {

    if(mode == PCS_POWER_SET)
    {
        for(int i = 0; i < this->m_data.count()/2; i++)
        {
            uint16_t reg = this->m_data[i*2];
            uint16_t numL = 0xFF,numH = 0xFF;

            for(int mapIdx = 0; mapIdx < 10; mapIdx++)
            {
                if((reg == rowToRegMap[mapIdx].reg) && (rowToRegMap[mapIdx].isH == false))
                {
                    uint16_t row = rowToRegMap[mapIdx].row;
                    QTableWidgetItem* item = ui->tableWidget->item(row,COL_VALUE);
                    numL = (item->text().toUInt())&0xFF;
                }
                if((reg == rowToRegMap[mapIdx].reg) && (rowToRegMap[mapIdx].isH == true))
                {
                    uint16_t row = rowToRegMap[mapIdx].row;
                    QTableWidgetItem* item = ui->tableWidget->item(row,COL_VALUE);
                    numH = (item->text().toUInt())&0xFF;

                }
            }
            if((numL!=0xFF)&&(numH!=0xFF))
            {
                uint16_t val = ((numL&0xFF) << 0) | ((numH&0xFF) << 8);
                qDebug()<< reg << numH << numL << val;
                this->m_data[i*2+1] = val;
            }
        }
    }
    else
    {
        uint16_t tmp;
        tmp = (uint16_t)(ui->sbox_cfg_1->value() | ui->sbox_cfg_2->value()<<8);
        this->m_value = tmp;
    }
}
