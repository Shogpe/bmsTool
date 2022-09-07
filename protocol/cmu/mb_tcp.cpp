#include "mb_tcp.h"
#include <QDebug>
#include <QTimerEvent>
#include "myhelper.h"
#include "utils.h"

mb_tcp::mb_tcp(QString ip, uint16_t port, uint8_t addr) {
    cmu = nullptr;
    drv_status = 0;
    mb_ip = ip.toStdString();
    mb_port = port;
    mb_addr = addr;
    cmu = modbus_new_tcp(mb_ip.c_str(), port);
    modbus_set_response_timeout(cmu, 5, 0);
    modbus_set_slave(cmu, addr);
}

mb_tcp::~mb_tcp() {
    if (this->cmu) {
        this->close();
        modbus_free(this->cmu);
    }
    this->cmu = nullptr;
}
int mb_tcp::Connect() {
    modbus_close(this->cmu);
    return modbus_connect(this->cmu);
}

void mb_tcp::close() { return modbus_close(this->cmu); }

int mb_tcp::init_config(vector<MB_NODE>& tab_config) {
    qDebug() << "init config";
    NodeReg node_reg_tmp;
    reg_list_.clear();
    wr_list_.clear();
    int index = -1;
    ST_NODE_DATA tmp_data;
    tmp_data.sysData.val.f64 = 0;
    tab_data.clear();
    name_map.clear();
    int node_index = 0;
    for (vector<MB_NODE>::iterator node_iter = tab_config.begin(); node_iter != tab_config.end(); node_iter++) {
        node_reg_tmp.default_val = 0;
        if (node_iter->reg_type > NONE_REG) {
            node_reg_tmp.reg_type = node_iter->reg_type;
            node_reg_tmp.reg_addr = node_iter->reg_addr;
            node_reg_tmp.data_type = node_iter->data_type;
            node_reg_tmp.index = node_index++;
            node_reg_tmp.factor = node_iter->factor;
            if ((index = JudgeReg(node_reg_tmp)) != -1) {
                InsertReg(node_reg_tmp, index);
            } else {
                NewReg(node_reg_tmp);
            }
        }
        tab_data.push_back(tmp_data);
        name_map[node_iter->name] = node_reg_tmp;
    }
    return 0;
}

/*
 * 读取数据
 **/
int mb_tcp::ReadData(uint8_t type, int start_addr, int reg_num, uint16_t* dest) {
    int status = 0;
    int read_len = 0;
    int shortnum = max_frame_len;
    int rc = 0;
    if (!reg_num) return status;
    switch (type) {
        case MODBUS_FC_READ_HOLDING_REGISTERS: {
            do {
                read_len = reg_num > shortnum ? shortnum : reg_num;
                rc = modbus_read_registers(this->cmu, start_addr, read_len, dest);
                if (rc < 0) {
                    qDebug() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc;
                    rc = modbus_read_registers(this->cmu, start_addr, read_len, dest);
                }
                if (rc > 0) {
                    status += rc;
                }
                reg_num -= read_len;
                dest += read_len;
                start_addr += read_len;
            } while (reg_num);
        } break;
        case MODBUS_FC_READ_INPUT_REGISTERS: {
            do {
                read_len = reg_num > shortnum ? shortnum : reg_num;
                rc = modbus_read_input_registers(this->cmu, start_addr, read_len, dest);
                if (rc > 0) {
                    status += rc;
                } else {
                    qDebug() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc;
                }
                reg_num -= read_len;
                dest += read_len;
                start_addr += read_len;
            } while (reg_num);
        } break;
        default:
            break;
    }
    if (status <= 0) {
        drv_status &= ~(0x01U << _ST_ONLINE);
        err_counter++;
    } else {
        err_counter = 0;
        drv_status |= (0x01 << _ST_ONLINE);
    }

    return status;
}
vector<ST_NODE_DATA> mb_tcp::ReadALL() {
    /* Read 5 registers from the address 0 */
    this->ReadAI();
    return tab_data;
}
int mb_tcp::read_value(uint16_t type, uint16_t addr, uint16_t len, uint16_t* v) {
    int ret = -1;
    if (type == 0x03) {
        ret = modbus_read_registers(cmu, addr, len, v);
    } else if (type == 0x04) {
        ret = modbus_read_input_registers(cmu, addr, len, v);
    } else {
    }
    if (ret < 0)
        qDebug() << "read failed" << type << ":" << addr << ":" << ret;
    else
        qDebug() << "read ok " << type << ":" << addr << ":" << len;
    return ret;
}
int mb_tcp::write_ao(uint16_t addr, uint16_t len, uint16_t* pv) {
    int ret = -1;
    if (!pv) return ret;
    if (len < 1) return ret;
    if (len == 1) {
        ret = write_ao(addr, *pv);
        return ret;
    }
    ret = modbus_write_registers(cmu, addr, len, pv);
    if (ret < 0)
        qDebug() << "wr aos failed" << addr << ":" << ret;
    else
        qDebug() << "wr aos " << addr << ":" << len;
    return ret;
}
int mb_tcp::write_ao(uint16_t addr, uint16_t v) {
    int ret = -1;
    ret = modbus_write_register(cmu, addr, v);
    if (ret < 0)
        qDebug() << "wr ao failed" << addr << ":" << ret;
    else
        qDebug() << "wr ao " << addr << ":" << v;
    return ret;
}

/**< 判别寄存器插入条件，返回可用报文序号，否则返回-1 */
int mb_tcp::JudgeReg(NodeReg& node_reg) {
    int index = reg_list_.size();
    int shortnum = max_frame_len;
    int bitnum = shortnum * 16;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) / 2;
    reg_len = (reg_len) > 1 ? reg_len : 1;
    uint8_t reg_type = node_reg.reg_type;
    while (index--) {
        DataReg data_reg = reg_list_.at(index);
        if (reg_type == DO_REG || reg_type == DI_REG) {
            if ((uint16_t)(node_reg.reg_addr - data_reg.reg_start) < bitnum && data_reg.reg_type == reg_type) {
                return index;
            }
        }
        if (reg_type == AO_REG || reg_type == AI_REG) {
            if ((uint16_t)(node_reg.reg_addr + reg_len - data_reg.reg_start) < shortnum &&
                data_reg.reg_type == reg_type) {
                return index;
            }
        }
    }
    return -1;
}
void mb_tcp::NewReg(NodeReg& node_reg) {
    DataReg data_reg;
    DatabaseIO data_io_tmp;
    data_reg.data_io.clear();
    data_reg.reg_type = node_reg.reg_type;
    data_reg.reg_start = node_reg.reg_addr;
    data_reg.data_num = 1;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) >> 1;
    data_reg.reg_num = (reg_len) > 1 ? reg_len : 1;
    data_io_tmp.index = node_reg.index;
    data_io_tmp.factor = node_reg.factor;
    data_io_tmp.data_type = node_reg.data_type;
    data_io_tmp.offset = 0;
    data_reg.data_io.push_back(data_io_tmp);
    reg_list_.push_back(data_reg);
}
void mb_tcp::InsertReg(NodeReg& node_reg, int index) {
    DataReg* pdata_reg;
    pdata_reg = &reg_list_.at(index);
    DatabaseIO data_io_tmp;
    pdata_reg->data_num++;
    data_io_tmp.index = node_reg.index;
    data_io_tmp.factor = node_reg.factor;
    data_io_tmp.data_type = node_reg.data_type;
    data_io_tmp.offset = node_reg.reg_addr - pdata_reg->reg_start;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) >> 1;
    reg_len = (reg_len) > 1 ? reg_len : 1;
    int len = data_io_tmp.offset + reg_len;
    if (pdata_reg->reg_num < len) pdata_reg->reg_num = len;
    pdata_reg->data_io.push_back(data_io_tmp);
}

int mb_tcp::ReadAI() {
    int res = -1;
    uint16_t tab_buf[128];
    for (vector<DataReg>::iterator iter = reg_list_.begin(); iter != reg_list_.end(); iter++) {
        memset(tab_buf, 0, sizeof(tab_buf));
        res = ReadData(iter->reg_type, iter->reg_start, iter->reg_num, tab_buf);
        if (res == iter->reg_num) {
            for (vector<DatabaseIO>::iterator data_iter = iter->data_io.begin(); data_iter != iter->data_io.end();
                 data_iter++) {
                if (tab_data.size() > data_iter->index) {
                    tab_data.at(data_iter->index).isUpdate = true;
                    tab_data.at(data_iter->index).UpdateCnt++;
                    if (data_iter->data_type == 514) {
                        tab_data.at(data_iter->index).sysData.val.f64 = tab_buf[data_iter->offset] * data_iter->factor;
                    } else if (data_iter->data_type == 513) {
                        tab_data.at(data_iter->index).sysData.val.f64 =
                            (int16_t)tab_buf[data_iter->offset] * data_iter->factor;
                    } else if (data_iter->data_type == 0x4402) {  // 17410
                        tab_data.at(data_iter->index).sysData.val.f64 =
                            MODBUS_GET_INT32_FROM_INT16_SWAP(tab_buf, data_iter->offset) * data_iter->factor;
                    }
                }
            }
        }
    }
    return res;
}
