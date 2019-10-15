#include "ExtendedGroupBox.h"

ExtendedGroupBox::ExtendedGroupBox(QWidget *parent /*= nullptr*/, State state /*= STATE_NORMAL*/) : QGroupBox(parent) {
    setObjectName("ExtendedGroupBox");
    setCheckable(true);
    state_ = state;
    if (state_ == STATE_NORMAL) {
        //隐藏垂直边框
        setFlat(true);
        setChecked(false);
    }
    setStyleSheet(
        QString("QGroupBox#ExtendedGroupBox::indicator{width:8px;height:8px;}QGroupBox#ExtendedGroupBox::indicator:"
                "unchecked{image:url(:/icons/uncheck.png);}QGroupBox#ExtendedGroupBox::indicator:checked{image:url(:/"
                "icons/check.png);}"));
    connect(this, SIGNAL(clicked(bool)), this, SLOT(onChecked(bool)));
}

ExtendedGroupBox::ExtendedGroupBox(const QString &title, QWidget *parent /*= nullptr*/, State state /*= STATE_NORMAL*/)
    : QGroupBox(title, parent) {
    setObjectName("ExtendedGroupBox");
    setCheckable(true);
    state_ = state;
    if (state_ == STATE_NORMAL) {
        //隐藏垂直边框
        setFlat(true);
        setChecked(false);
    }
    connect(this, SIGNAL(clicked(bool)), this, SLOT(onChecked(bool)));
}

void ExtendedGroupBox::addWidget(QWidget *widget) {
    if (widget != nullptr) {
        if (state_ == STATE_NORMAL) {
            widget->setVisible(false);
        }
        children_.push_back(widget);
    }
}

void ExtendedGroupBox::onChecked(bool checked) {
    if (checked) {
        //显示垂直边框
        setFlat(false);
        for (auto iter = children_.begin(); iter != children_.end(); ++iter) {
            (*iter)->setVisible(true);
        }
        state_ = STATE_EXPAND;
    } else {
        //隐藏垂直边框
        setFlat(true);
        for (auto iter = children_.begin(); iter != children_.end(); ++iter) {
            (*iter)->setVisible(false);
        }
        state_ = STATE_NORMAL;
    }
}

ExtendedGroupBox::State ExtendedGroupBox::getState() const { return state_; }
