#ifndef Rebootbmus_H
#define Rebootbmus_H

#include <QTimer>
#include <QWidget>

namespace Ui {
class Rebootbmus;
}
enum ProVersion{
    Active = 0,
    Merge,
    Water
};
class Rebootbmus : public QWidget
{
    Q_OBJECT

public:
    explicit Rebootbmus(QWidget *parent = nullptr);
    ~Rebootbmus();
    ProVersion version;

private:
    Ui::Rebootbmus *ui;
    QTimer timer_script;
    QWidget *parentForm;
    uint32_t Tx_Cnt;


private slots:
    void btnclick();
    void timerUpadte();

signals:
    void CloseThisForm();
    void send_data();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // Rebootbmus_H
