#include "frmmessagebox.h"
#include "iconhelper.h"
#include "myhelper.h"
#include "ui_frmmessagebox.h"

frmMessageBox::frmMessageBox(QWidget *parent) : QDialog(parent), ui(new Ui::frmMessageBox) {
    ui->setupUi(this);

    // this->mousePressed = false;
    //设置窗体标题栏隐藏
    this->setWindowFlags(Qt::WindowTitleHint);
    ui->groupBox->setStyleSheet("border:none");
    //设置窗体关闭时自动释放内存
    this->setAttribute(Qt::WA_DeleteOnClose);
    //设置图形字体
    // IconHelper::Instance()->setIcon(ui->lab_Ico, QChar(0xf015), 12);

    //关联关闭按钮
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}

frmMessageBox::~frmMessageBox() { delete ui; }

void frmMessageBox::SetMessage(const QString &msg, int type) {
    if (type == 0) {
        ui->labIcoMain->setStyleSheet("border-image: url(:/image/msg_info.png);");
        setWindowIcon(QIcon(":/image/msg_info.png"));
        ui->btnCancel->setVisible(false);
        this->setWindowTitle(tr("提示"));
    } else if (type == 1) {
        ui->labIcoMain->setStyleSheet("border-image: url(:/image/msg_question.png);");
        setWindowIcon(QIcon(":/image/msg_question.png"));
        this->setWindowTitle(tr("询问"));
    } else if (type == 2) {
        ui->labIcoMain->setStyleSheet("border-image: url(:/image/msg_error.png);");
        setWindowIcon(QIcon(":/image/msg_error.png"));
        ui->btnCancel->setVisible(false);
        this->setWindowTitle(tr("错误"));
    }

    ui->labInfo->setText(msg);
}

void frmMessageBox::on_btnOk_clicked() {
    done(QDialog::Accepted);
    this->close();
}

// void frmMessageBox::mouseMoveEvent(QMouseEvent *e)
//{
//    if (mousePressed && (e->buttons() && Qt::LeftButton)) {
//        this->move(e->globalPos() - mousePoint);
//        e->accept();
//    }
//}

// void frmMessageBox::mousePressEvent(QMouseEvent *e)
//{
//    if (e->button() == Qt::LeftButton) {
//        mousePressed = true;
//        mousePoint = e->globalPos() - this->pos();
//        e->accept();
//    }
//}

// void frmMessageBox::mouseReleaseEvent(QMouseEvent *)
//{
//    mousePressed = false;
//}
