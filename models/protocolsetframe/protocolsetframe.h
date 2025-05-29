#ifndef PROTOCOLSETFRAME_H
#define PROTOCOLSETFRAME_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class ProtocolSetFrame; }
QT_END_NAMESPACE

class ProtocolSetFrame : public QDialog
{
    Q_OBJECT

public:
    ProtocolSetFrame(QWidget *parent = nullptr);
    ~ProtocolSetFrame();
    void setText(QString txt);
    void setCPVer(int ver);
    void setVerList(QList<int> list);
private slots:
    void on_close_clicked();

    void on_runChange_clicked();

private:
    Ui::ProtocolSetFrame *ui;
    QList<int> cproVerList;

signals:
    void setCPVerConfirm(int ver);
};
#endif // PROTOCOLSETFRAME_H
