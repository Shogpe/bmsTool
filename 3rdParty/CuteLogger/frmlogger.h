#ifndef frmLogger_H
#define frmLogger_H

#include <QListWidgetItem>
#include <QWidget>

class QRadioButton;
namespace Ui {
class frmLogger;
}

class frmLogger : public QWidget {
    Q_OBJECT

   public:
    explicit frmLogger(QWidget *parent = 0);
    ~frmLogger();

   private:
    Ui::frmLogger *ui;
    QList<QRadioButton *> rdbList;
    quint8 m_log_filter = -1;
    void SaveSettings();
    void LoadLogSettings();

   private slots:
    void initForm();
    void append(const QString &flag = QString());
    void getLogDirection();
    void getstr(const int level, const QString &content);

   private slots:
    void checkItemChanged(QListWidgetItem *item);
    void on_cboxViewRows_currentIndexChanged(int index);
    void on_listType_itemPressed(QListWidgetItem *item);
};

#endif  // frmLogger_H
