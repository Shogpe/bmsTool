#include "inputbox.h"
#include <QDebug>
#include <QInputDialog>
#include <QMouseEvent>
#include "PropertyHelper.h"
#include "myhelper.h"
#include "ui_inputbox.h"
#include "QHBoxLayout"

InputBox::InputBox(QWidget *parent) : QWidget(parent), ui(new Ui::InputBox) {
    ui->setupUi(this);
    m_value = 0;
    m_maximum = 99999;
    m_minimum = 0;
    m_type = CT_VALUE;
    m_cleanText = " ";
    uiInit();
}
void InputBox::uiInit() {
//    ui->text->setMaxLength(12);
//    QFontMetrics fm(ui->text->font());
//    ui->text->setMaximumWidth(fm.width("12345.67 kWh"));
    ui->text->installEventFilter(this);
    connect(this, &InputBox::prefixChanged, this, [=](QString str) { ui->prefix->setText(str); });
    connect(this, &InputBox::suffixChanged, this, [this]() { setText(QString::number(value())); });
    connect(this, &InputBox::valueChanged, this, &InputBox::setValueDirect);
    connect(this, &InputBox::readOnlyChanged, this, [this](bool val) {
        ui->text->setReadOnly(val);
        ui->text->setFrame(!val);
        if (val) {
            ui->text->removeEventFilter(this);
        } else {
            ui->text->installEventFilter(this);
        }
    });
    connect(this, &InputBox::typeChanged, this, [this](int t) {
        switch (t) {
            case CT_COMBO:
            case CT_STRING:
            case CT_BIT_ARR:
            case CT_HEX_VER: {
                ui->text->setMaxLength(64);
                QFontMetrics fm(ui->text->font());
                ui->text->setMaximumWidth(fm.width(" ") * 64);
            } break;
            case CT_VALUE:
            default:
                ui->text->setMaxLength(12);
                QFontMetrics fm(ui->text->font());
                if(this->suffix().startsWith("kWh")||this->suffix().startsWith(" kWh"))
                {
                    ui->text->setMaximumWidth(fm.width("12345.67 kWh"));
                }
                else
                {
                    ui->text->setMaximumWidth(fm.width("123456"));
                }
                break;
        }
    });
    connect(ui->text, &QLineEdit::returnPressed, this, &InputBox::editingFinished);
    connect(this, &QWidget::objectNameChanged, this, [this](QString name) { setObjectName(name); });
    //    this->startTimer(1000);
    //    setText(QString::number(value()));
    this->setValueDirect(value());


}

void InputBox::editingFinished() {
    QString str = ui->text->text();
    switch (this->type()) {
        case CT_COMBO:
        case CT_STRING:
        case CT_BIT_ARR:
        case CT_HEX_VER:
            break;
        case CT_VALUE:
        default: {
            bool ok = false;
            if (str.endsWith(suffix())) {
                str = str.left(str.length() - suffix().length()).trimmed();
            }
            double val = str.toDouble(&ok);
            qDebug() << str << val << ok;
            if (ok && (val <= maximum()) && (val >= minimum())) {
                qDebug() << val << maximum() << minimum();
                if (QString::number(val) != cleanText()) {
                    qDebug() << objectName() << val << value() << cleanText();
                    setValue(val);
                }
            } else {
                setCleanText("");
                setText(QString::number(value()));
            }
        } break;
    }
}
void InputBox::setValueDirect(qreal val) {
    m_value = val;
    int ivalue = 0;
    uint uvalue = 0;
    //    qDebug() << prefix() << type() << objectName() << "valchg" << val;
    switch (this->type()) {
        case CT_COMBO:

            break;
        case CT_STRING:

            break;
        case CT_HEX_VER:

//            setText(myHelper::IntegerToHexString(val));
            uvalue = val;
            setText(QString("%1.%2.%3.%4")
                        .arg((uvalue >> 24) & 0xFF, 2, 16, QChar('0'))
                        .arg((uvalue >> 16) & 0xFF, 2, 16, QChar('0'))
                        .arg((uvalue >> 8)  & 0xFF, 2, 16, QChar('0'))
                        .arg((uvalue >> 0)  & 0xFF, 2, 16, QChar('0')).toUpper());
            break;
        case CT_BIT_ARR: {
            ivalue = val;
            setText(QString("%1,%2,%3,%4")
                        .arg(ivalue >> 24 & 0xFF, 8, 2, QChar('0'))
                        .arg(ivalue >> 16 & 0xFF, 8, 2, QChar('0'))
                        .arg(ivalue >> 8  & 0xFF, 8, 2, QChar('0'))
                        .arg(ivalue >> 0  & 0xFF, 8, 2, QChar('0')));
        } break;
        case CT_VALUE:
        default:
            setText(QString::number(val));
            break;
    }
}

bool InputBox::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->text) {
        if (event->type() == QEvent::FocusOut) {
            qDebug() << "text FocusOut";
            editingFinished();
        }
    }
    return QWidget::eventFilter(watched, event);
}
void InputBox::setText(QString str) {
    if (str == cleanText()) return;
    setCleanText(str);
    QString allText;
    if (suffix().isEmpty()) {
        allText = cleanText();
    } else {
        allText = QString("%1 %2").arg(cleanText(), suffix());
    }
    if (ui->text->text() != allText) ui->text->setText(allText);
}

void InputBox::setFontAndSize(int s){

    if((s > 5) && (s < 10)){
        QFont f;
        f.setFamily(QFontInfo(ui->text->font()).family()); // 保持原字体族
        f.setPointSize(s);
        f.setWeight(QFont::Normal);

        // 应用字体
        ui->prefix->setFont(f);
        ui->text->setFont(f);
        this->setFont(f);

        // 计算合适的高度
        QFontMetrics fm(f);
        ui->text->setMinimumHeight(fm.height() + 2); // 基础高度 + 边距
        ui->text->setMaximumHeight(fm.height() + 4);
//        qDebug()<<f;
    }
}

