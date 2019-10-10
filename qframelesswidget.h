#ifndef QFRAMELESSWIDGET_H
#define QFRAMELESSWIDGET_H

#include <QtWidgets>

class QFramelessWidget : public QWidget{
    Q_OBJECT
public:
    explicit QFramelessWidget(QWidget *parent = nullptr):QWidget(parent){
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        setMouseTracking(true);
    }
private:
    enum Direction{
        UP, DOWN, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE
    };
    const int Padding = 1;
    bool isLeftPressDown = false;// 判断左键是否按下
    QPoint dragPosition;// 窗口移动拖动时需要记住的点
    Direction dir = NONE;// 窗口大小改变时，记录改变方向
    void judgeRegionSetCursor(const QPoint& currentPoint){
        // 获取窗体在屏幕上的位置区域，tl为topleft点，rb为rightbottom点
        auto tl = mapToGlobal(rect().topLeft());
        auto rb = mapToGlobal(rect().bottomRight());
        auto x = currentPoint.x();
        auto y = currentPoint.y();
        if ((tl.x() + Padding) >= x && tl.x() <= x && (tl.y() + Padding) >= y && tl.y() <= y){
            // 左上角
            dir = LEFTTOP;
            this->setCursor(QCursor(Qt::SizeFDiagCursor));  // 设置鼠标形状
        }else if ((x >= rb.x() - Padding) && x <= rb.x() && y >= (rb.y() - Padding) && y <= rb.y()){
            // 右下角
            dir = RIGHTBOTTOM;
            this->setCursor(QCursor(Qt::SizeFDiagCursor));
        }else if (x <= (tl.x() + Padding) && x >= tl.x() && (y >= rb.y() - Padding) && y <= rb.y()){
            //左下角
            dir = LEFTBOTTOM;
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
        }else if (x <= rb.x() && x >= rb.x() - Padding && y >= tl.y() && y <= tl.y() + Padding) {
            // 右上角
            dir = RIGHTTOP;
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
        }else if (x <= (tl.x() + Padding) && x >= tl.x()) {
            // 左边
            dir = LEFT;
            this->setCursor(QCursor(Qt::SizeHorCursor));
        }else if (x <= rb.x() && x >= (rb.x() - Padding)) {
            // 右边
            dir = RIGHT;
            this->setCursor(QCursor(Qt::SizeHorCursor));
        }else if (y >= tl.y() && y <= tl.y() + Padding) {
            // 上边
            dir = UP;
            this->setCursor(QCursor(Qt::SizeVerCursor));
        }else if (y <= rb.y() && y >= (rb.y() - Padding)) {
            // 下边
            dir = DOWN;
            this->setCursor(QCursor(Qt::SizeVerCursor));
        }else{
            // 默认
            dir = NONE;
            this->setCursor(QCursor(Qt::ArrowCursor));
        }
    }
protected:
    void mouseReleaseEvent(QMouseEvent *event){
        isLeftPressDown = false;
        dir = NONE;
        releaseMouse();
        setCursor(QCursor(Qt::ArrowCursor));
        QWidget::mouseReleaseEvent(event);
    }
    void mousePressEvent(QMouseEvent *event){
        switch (event->button()) {
        case Qt::LeftButton:
            isLeftPressDown = true;
            if (dir != NONE) {
                this->mouseGrabber();
            }
            else {
                dragPosition = event->globalPos() - this->frameGeometry().topLeft();
            }
            break;
        default:
            QWidget::mousePressEvent(event);
        }
    }
    void mouseMoveEvent(QMouseEvent *event){
        auto gloPoint = event->globalPos();
        auto tl = mapToGlobal(rect().topLeft());
        auto rb = mapToGlobal(rect().bottomRight());
        if (!isLeftPressDown)judgeRegionSetCursor(gloPoint);
        else if(dir != NONE){
                QRect rMove(tl, rb);
                switch (dir) {
                case LEFT:
                    if(rb.x() - gloPoint.x() <= this->minimumWidth())rMove.setX(tl.x());
                    else rMove.setX(gloPoint.x());
                    break;
                case RIGHT:
                    rMove.setWidth(gloPoint.x() - tl.x());
                    break;
                case UP:
                    if(rb.y() - gloPoint.y() <= this->minimumHeight())rMove.setY(tl.y());
                    else rMove.setY(gloPoint.y());
                    break;
                case DOWN:
                    rMove.setHeight(gloPoint.y() - tl.y());
                    break;
                case LEFTTOP:
                    if(rb.x() - gloPoint.x() <= this->minimumWidth())rMove.setX(tl.x());
                    else rMove.setX(gloPoint.x());
                    if(rb.y() - gloPoint.y() <= this->minimumHeight())rMove.setY(tl.y());
                    else rMove.setY(gloPoint.y());
                    break;
                case RIGHTTOP:
                    rMove.setWidth(gloPoint.x() - tl.x());
                    rMove.setY(gloPoint.y());
                    break;
                case LEFTBOTTOM:
                    rMove.setX(gloPoint.x());
                    rMove.setHeight(gloPoint.y() - tl.y());
                    break;
                case RIGHTBOTTOM:
                    rMove.setWidth(gloPoint.x() - tl.x());
                    rMove.setHeight(gloPoint.y() - tl.y());
                    break;
                default:
                    QWidget::mouseMoveEvent(event);
                    return;
                }setGeometry(rMove);
        }else{
            move(event->globalPos() - dragPosition);
            event->accept();
        }QWidget::mouseMoveEvent(event);
    }
};

#endif // QFRAMELESSWIDGET_H
