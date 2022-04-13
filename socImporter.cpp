#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "myhelper.h"
// [0] include QXlsx headers
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxchartsheet.h"
#include "xlsxdocument.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

QByteArray SOCImport() {
    QByteArray data;
    // [2] Reading excel file(*.xlsx)
    QString filename = QFileDialog::getOpenFileName(nullptr, "open", "", "*.xlsx");
    Document xlsxR(filename);
    if (xlsxR.load())  // load excel file
    {
        qDebug() << xlsxR.sheetNames();

        QInputDialog qDialog;
        qDialog.setWindowFlags(Qt::WindowCloseButtonHint);
        qDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
        qDialog.setComboBoxItems(xlsxR.sheetNames());
        qDialog.setWindowTitle("选择工作簿...");
        qDialog.setLabelText("请选择一个作为SOC表导入数据源");
        qDialog.setCancelButtonText(("放弃"));
        qDialog.setOkButtonText(("确定"));
        if (qDialog.exec()) {
            if (xlsxR.selectSheet(qDialog.textValue())) {
                do {
                    Cell* cell = xlsxR.cellAt(1, 1);  // get cell pointer.
                    if (cell == NULL || "%" != cell->readValue().toString()) {
                        qDebug() << "load % error.";
                        data.clear();
                        break;
                    }
                    cell = xlsxR.cellAt(1, 2);  // get cell pointer.
                    if (cell == NULL || "单体电压" != cell->readValue().toString()) {
                        qDebug() << "load \"单体电压\" error.";
                        data.clear();
                        break;
                    }
                    for (int i = 2; i <= 102; i++) {
                        cell = xlsxR.cellAt(i, 2);  // get cell pointer.
                        bool ok;
                        if (cell != NULL) {
                            uint16_t val = cell->readValue().toUInt(&ok);
                            if (ok) {
                                qDebug() << val;
                                data.append(reinterpret_cast<char*>(&val), sizeof(uint16_t));
                                continue;
                            }
                            break;
                        }
                    }
                    break;
                } while (true);
                if (0 == data.size()) {
                    myHelper::ShowMessageBoxError("数据格式错误！");
                } else if ((sizeof(uint16_t) * 101) != data.size()) {
                    myHelper::ShowMessageBoxError("数据长度不合法！");
                }
            } else {
                myHelper::ShowMessageBoxError("工作簿加载错误！");
            }
        }
    } else {
        myHelper::ShowMessageBoxError("表格加载错误！");
    }

    return data;
}
