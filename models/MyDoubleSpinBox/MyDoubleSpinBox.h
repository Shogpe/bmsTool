#ifndef MY_SPIN_BOX_H
#define MY_SPIN_BOX_H
#include <QDateTime>
#include <QSpinBox>

class MyDoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT

   public:
    MyDoubleSpinBox(QWidget* parent = 0) : QDoubleSpinBox(parent) {}

    virtual QString textFromValue(double value) const {
        /* 4 - number of digits, 10 - base of number, '0' - pad character*/
        return QString("%1").arg(value);
    }
    virtual void mousePressEvent(QMouseEvent* event) { this->selectAll(); }

   private:
    void wheelEvent(QWheelEvent* event) { return; };
};

class MyTimeSpinBox : public QDoubleSpinBox {
    Q_OBJECT

   public:
    MyTimeSpinBox(QWidget* parent = 0) : QDoubleSpinBox(parent) {}

    virtual QString textFromValue(double value) const {
        /* 4 - number of digits, 10 - base of number, '0' - pad character*/
        return QDateTime::fromTime_t(value).toString("yyyy-MM-dd hh:mm:ss");
    }
    virtual void mousePressEvent(QMouseEvent* event) {}
};
class MyIPSpinBox : public QDoubleSpinBox {
    Q_OBJECT

   public:
    MyIPSpinBox(QWidget* parent = 0) : QDoubleSpinBox(parent) {}

    virtual QString textFromValue(double value) const {
        /* 4 - number of digits, 10 - base of number, '0' - pad character*/
        uint32_t ip_val = static_cast<uint32_t>(value);
        return QString("%1.%2.%3.%4")
            .arg(ip_val & 0xFF)
            .arg(ip_val >> 8 & 0xFF)
            .arg(ip_val >> 16 & 0xFF)
            .arg(ip_val >> 24 & 0xFF);
    }
    virtual double valueFromText(const QString& text) const override {
        QRegExp regExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");

        if (regExp.exactMatch(text)) {
            QStringList list = text.split(".");
            uint8_t ip_val[4] = {0};
            ip_val[0] = list.at(0).toUInt();
            ip_val[1] = list.at(1).toUInt();
            ip_val[2] = list.at(2).toUInt();
            ip_val[3] = list.at(3).toUInt();
            return ip_val[0] << 24 | ip_val[1] << 16 | ip_val[2] << 8 | ip_val[3];
        } else {
            return 0;
        }
    }

    virtual void mousePressEvent(QMouseEvent* event) {}
};
#endif
#include <QToolTip>
class MyStatusSpinBox : public QDoubleSpinBox {
    Q_OBJECT

   public:
    MyStatusSpinBox(QWidget* parent = 0) : QDoubleSpinBox(parent) {}

    virtual void mousePressEvent(QMouseEvent* event) {
      this->setToolTip(this->objectName());
    }
};
