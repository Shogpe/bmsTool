#ifndef STATEGROUPBOX_H
#define STATEGROUPBOX_H

#include <QGroupBox>
#include <QLabel>
namespace Ui {
class StateGroupBox;
}

class StateGroupBox : public QGroupBox
{
  Q_OBJECT

 public:
  explicit StateGroupBox(QWidget *parent = nullptr);
  ~StateGroupBox();
  void init(QStringList &StateList);
  bool setValue(uint16_t value);

 private:
  Ui::StateGroupBox *ui;
  QList<QLabel*> StatusList;
};

#endif // STATEGROUPBOX_H
