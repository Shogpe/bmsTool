#ifndef FIRMWARE_DIALOG_H
#define FIRMWARE_DIALOG_H

#include <QDialog>
#include <QElapsedTimer>
#include <QTimer>
namespace Ui {
class firmwareDialog;
}
#pragma pack(1)
typedef struct {
    uint32_t magic_head;  //固件头部魔数,可供搜索
    uint16_t head_type;  //固件头部类型,指定头部解析方式,适用的硬件版本
    uint16_t head_len;  //固件头部长度
    uint32_t firmware_version;
    uint32_t firmware_size;
    uint32_t firmware_crc32;
    char     RELEASEDATE[ 12 ];  // Mar 20 2019
    char     RELEASETIME[ 10 ];  // 12:34:00
    uint16_t check_sum;          //固件头部校验,人工4字节对齐
} FIRMWARE_HEAER;
typedef struct {
    uint32_t firmware_version;
    uint32_t firmware_crc32;
    uint32_t firmware_size;
} FIRMWARE_TAIL;
#pragma pack()
#define APP_FIRMWARE_HEAD 0xBFE2B2A9
#define APP_FIRMWARE_TYPE 0x1
class firmwareDialog : public QDialog {
    Q_OBJECT
    enum MACHINE_STATE {
        M_BOOTLOADR = 0,
        M_TRIGER,
        M_INFO1,
        M_INFO2,
        M_DATA,
        M_EXIT,
    };

   public:
    explicit firmwareDialog(QWidget* parent = nullptr);
    ~firmwareDialog();

   private slots:
    void on_transmitBrowse_clicked();

   signals:
    /**
     * @brief msg_send 消息发送
     * @param proto 协议号
     * @param b 数据内容
     * @param cnt 是否有后续包
     * @param ptp 是否点对点发送
     */
    void msg_send(int proto, const QByteArray b, bool cnt = 0, bool ptp = 1);

   private:
    Ui::firmwareDialog* ui;
    int state;
    int loop_time;
    bool is_ptp;

    QElapsedTimer timecost;
    QByteArray m_filedata;
    uint32_t m_version;
    uint32_t m_crc32;
    uint32_t m_crc32_send;
    uint32_t m_filelen;
    int m_fileoffset;
    int loadfile(QString fileName);
};

#endif  // FIRMWARE_DIALOG_H
