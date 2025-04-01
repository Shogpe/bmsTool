#ifndef INPUTBOX_H
#define INPUTBOX_H
#include <QDoubleSpinBox>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "PropertyHelper.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class InputBox;
}
QT_END_NAMESPACE

class InputBox : public QWidget {
    Q_OBJECT
    AUTO_PROPERTY(int, type, Type)                  // 控件类型
    AUTO_PROPERTY(QString, cleanText, CleanText)    // 值字符串
    AUTO_PROPERTY(QString, prefix, Prefix)          // 前缀
    AUTO_PROPERTY(QString, suffix, Suffix)          // 后缀
    AUTO_PROPERTY(double, value, Value)             // 值
    AUTO_PROPERTY(double, maximum, Maximum)         // 值
    AUTO_PROPERTY(double, minimum, Minimum)         // 值
    AUTO_PROPERTY(bool, readOnly, ReadOnly)         // 值

    // 兼容QDoubleSpinbox
    AUTO_PROPERTY(bool, wrapping, Wrapping)                                     // 值
    AUTO_PROPERTY(bool, frame, Frame)                                           // 值
    AUTO_PROPERTY(QString, specialValueText, SpecialValueText)                  // 值
    AUTO_PROPERTY(QString, alignment, Alignment)                                // 值
    AUTO_PROPERTY(QDoubleSpinBox::ButtonSymbols, buttonSymbols, ButtonSymbols)  // 值
    AUTO_PROPERTY(bool, accelerated, Accelerated)                               // 值
    AUTO_PROPERTY(QString, correctionMode, CorrectionMode)                      // 值
    AUTO_PROPERTY(bool, keyboardTracking, KeyboardTracking)                     // 值
    AUTO_PROPERTY(bool, showGroupSeparator, ShowGroupSeparator)                 // 值
    AUTO_PROPERTY(int, decimals, Decimals)                                      // 值
    AUTO_PROPERTY(double, singleStep, SingleStep)                               // 值
    AUTO_PROPERTY(QString, stepType, StepType)                                  // 值

   public:
    enum CONTROL_TYPE {
        CT_VALUE = 0,
        CT_COMBO,
        CT_STRING,
        CT_HEX_VER,   // 16进制分组
        CT_BIT_ARR,   // 位数组
        CT_TIME_STR,  // 时间字符串
    };
    InputBox(QWidget *parent = nullptr);
    void uiInit();
    bool eventFilter(QObject *watched, QEvent *event);
    void setText(QString str);
    void setFontAndSize(int s);

   public slots:
    void editingFinished();
    void setValueDirect(qreal val);

   protected:
    //重写mousePressEvent事件
    //    virtual void focusOutEvent(QFocusEvent *event);

   private:
    Ui::InputBox *ui;
};
class MyEditableLabel : public QWidget {
    Q_OBJECT
    AUTO_PROPERTY(QString, text, Text)
   public:
    MyEditableLabel(QWidget *parent = nullptr) : QWidget(parent), m_label(new QLabel), m_lineEdit(new QLineEdit) {
        setLayout(new QVBoxLayout);
        layout()->setMargin(0);
        layout()->setSpacing(0);
        layout()->addWidget(&stacked);

        stacked.addWidget(m_label);
        stacked.addWidget(m_lineEdit);
        m_label->installEventFilter(this);
        m_lineEdit->installEventFilter(this);
        setSizePolicy(m_lineEdit->sizePolicy());
        connect(m_lineEdit, &QLineEdit::textChanged, this, &MyEditableLabel::setText);
    }

    bool eventFilter(QObject *watched, QEvent *event) {
        if (watched == m_lineEdit) {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
                if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Escape ||
                    keyEvent->key() == Qt::Key_Enter) {
                    m_label->setText(m_lineEdit->text());
                    stacked.setCurrentIndex(0);
                }
            } else if (event->type() == QEvent::FocusOut) {
                m_label->setText(m_lineEdit->text());
                stacked.setCurrentIndex(0);
            }
        } else if (watched == m_label) {
            if (event->type() == QEvent::MouseButtonDblClick) {
                stacked.setCurrentIndex(1);
                m_lineEdit->setText(m_label->text());
                m_lineEdit->setFocus();
            }
        }
        return QWidget::eventFilter(watched, event);
    }

   private:
    QLabel *m_label;
    QLineEdit *m_lineEdit;
    QStackedWidget stacked;
};
#endif  // INPUTBOX_H
