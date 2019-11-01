#include "stategroupbox.h"
#include "ui_stategroupbox.h"
#include <QLayout>

StateGroupBox::StateGroupBox(QWidget *parent) :
                                                QGroupBox(parent),
                                                ui(new Ui::StateGroupBox)
{
  ui->setupUi(this);
  QStringList StateList;
  StateList<< "111" << "222"<< "333";
  init(StateList);
}

StateGroupBox::~StateGroupBox()
{
  delete ui;
}
void StateGroupBox::init(QStringList &List){
  QHBoxLayout *layout = new QHBoxLayout();
  QVBoxLayout *Vlayout[2];
  Vlayout[0] = new QVBoxLayout();
  Vlayout[1] = new QVBoxLayout();
  QString iter;
  layout->addLayout(Vlayout[0]);
  layout->addLayout(Vlayout[1]);
  int count = List.size();
  for(int i=0;i<count;i++){
    int index = (i>count/2);
    StatusList << new QLabel(List.at(i));
    Vlayout[index]->addWidget(StatusList.at(i));
  }
  this->setLayout(layout);
}
