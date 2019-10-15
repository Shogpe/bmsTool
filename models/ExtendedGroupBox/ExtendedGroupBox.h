#ifndef EXTENDED_GROUP_BOX_H_
#define EXTENDED_GROUP_BOX_H_

#include <QGroupBox>
#include <QVector>

class ExtendedGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    enum State
    {
        STATE_NORMAL,
        STATE_EXPAND
    };

public:
    ExtendedGroupBox(QWidget *parent = nullptr, State state = STATE_NORMAL);
    ExtendedGroupBox(const QString &title, QWidget *parent = nullptr, State state = STATE_NORMAL);

private Q_SLOTS:
    void onChecked(bool checked);

public:
    void addWidget(QWidget *widget);
    State getState() const;

private:
    QVector<QWidget*> children_;
    State state_;
};


#endif//EXTENDED_GROUP_BOX_H_