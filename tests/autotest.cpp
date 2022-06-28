#include <QDebug>
#include <QJsonObject>
#include <QtTest>
// add necessary includes here
extern "C" {
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"
}
class AutoTest : public QObject {
    Q_OBJECT

   public:
    AutoTest();
    ~AutoTest();

   private slots:
    void initTestCase();
    void test_case1();
    void test_lua_parse();
    void test_str_parse();
    void cleanupTestCase();
};

AutoTest::AutoTest() {}

AutoTest::~AutoTest() {}

void AutoTest::initTestCase() {}

void AutoTest::test_case1() {
    QString name("AutoTest");
    bool isEnable = true;
    QVERIFY(isEnable);
    QCOMPARE(name, QString("AutoTest"));
}
QString getLuaString(uint32_t value, QString script) {
    //每次都重新创建lua解析器
    QString ret = "";
    lua_State* L = luaL_newstate();  //新建lua解析器
    luaL_openlibs(L);                //载入lua基础库
    //
    lua_pushnumber(L, value);
    lua_setglobal(L, "val");
    int ok = luaL_dostring(L, script.toStdString().c_str());  //加载脚本
    if (ok == 0) {
        //检查栈顶数据是否为字符串，否则退出
        int type = lua_type(L, -1);
        if (type == 4) {
            ret = QString(lua_tostring(L, -1));
        } else {
            // QWARN(QString::number(type).toStdString().c_str());
        }
    }
    lua_close(L);
    return ret;
}
void AutoTest::test_lua_parse() {
    const QString swi_rule = R"EOF(
        local switch = {
            [1] = function()
                return ("case1")
            end,
            [2] = function()
                return ("case2")
            end,
            [3] = function()
                return ("case3")
            end,
            [31] = function()
                return ("case3")
            end
        }
        local f = switch[val]
        if(f) then
            return f()
        else
            return string.format("%g",val)
        end
        )EOF";
    QString code =
        ""
        "ret = 10*val;return tostring(ret)"
        "";
    QCOMPARE(getLuaString(121, code), "1210.0");
    for (int i = 0; i < 10; i++) {
        switch (i) {
            case 1:
                QCOMPARE(getLuaString(i, swi_rule), "case1");
                break;
            case 2:
                QCOMPARE(getLuaString(i, swi_rule), "case2");
                break;
            case 3:
                QCOMPARE(getLuaString(i, swi_rule), "case3");
                break;
            case 31:
                QCOMPARE(getLuaString(i, swi_rule), "case3");
                break;
            default:
                QCOMPARE(getLuaString(i, swi_rule), QString::number(i));
                break;
        }
    }
}
typedef struct {
    QString prefix;  // 前缀
    uint16_t type;   // 数据类型
    double factor;   // 系数
    double offset;   // 偏移量
    QString suffix;  // 单位
} ST_FORMAT;
uint16_t getDataType(QString type) {
    QMap<QString, uint16_t> type_map = {
        {"U16", 0x202},
        {"I16", 0x201},
        {"BIT", 0x206},
        {"MAP", 0x207},
    };
    return type_map.value(type, 0x202);
}
double getData(uint64_t raw, ST_FORMAT format) {
    switch (format.type) {
        case 0x201:
            return int16_t(raw) * format.factor + format.offset;
        default:
            return uint16_t(raw) * format.factor + format.offset;
    }

    return 0;
}
ST_FORMAT getFormat(QString rule) {
    QStringList formatArr = rule.split(",");
    ST_FORMAT format;
    format.prefix = QString(formatArr.at(0));
    format.type = 0x202;
    format.factor = 1;
    format.offset = 0;
    format.suffix = "";
    if (formatArr.size() == 5) {
        format.prefix = QString(formatArr.at(0));
        format.type = getDataType(formatArr.at(1));
        format.factor = QString(formatArr.at(2)).toDouble();
        format.offset = QString(formatArr.at(3)).toDouble();
        format.suffix = QString(formatArr.at(4));
    } else if (formatArr.size() == 2) {
        format.prefix = QString(formatArr.at(0));
        format.type = getDataType(formatArr.at(1));
    }
    return format;
}
QString getData(QString key, uint64_t raw, QString jsonStr) {
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        if (jsonDocument.isObject()) {
            QVariantMap result = jsonDocument.toVariant().toMap();
            if (result.contains(key)) {
                QJsonObject value = result.value(key).toJsonObject();
                if (value.contains(QString::number(raw))) {
                    return value.value(QString::number(raw)).toString();
                }
            }
        }
    }
    return "";
}
QString getData(uint64_t raw, QString rule, QString key = "", QString code = "") {
    ST_FORMAT format = getFormat(rule);
    switch (format.type) {
        case 0x201:
            return QString::number(int16_t(raw) * format.factor + format.offset);
        case 0x202:
            return QString::number(uint16_t(raw) * format.factor + format.offset);
        case 0x207:
            return getData(key, raw, code);
            qWarning() << rule << "parse failed.";
            break;
        default:
            break;
    }

    return QString::number(raw);
}
void AutoTest::test_str_parse() {
    uint64_t raw = 0x87658321;
    QStringList rules = {
        "门限电压,U16,0.0001,0,V",
        "门限电流,I16,0.01,0,A",
        "门限电流,MAP",
    };
    QStringList results = {
        "门限电压:3.3569V",
        "门限电流:-319.67A",
        "门限电流:-319.67A",
    };
    QString key = "evt_id";
    QString codeStr = R"EOF(
            {"evt_id":{"0":"KM+","1":"KM-","2":"KMR","2271576865":"QF"},"evt_threshold":{"AA55":"合闸","56AA":"分闸"}}
        )EOF";
    for (int i = 0; i < rules.size(); i++) {
        qDebug() << getData(raw, rules.at(i), key, codeStr);
    }
}
void AutoTest::cleanupTestCase() {}

QTEST_APPLESS_MAIN(AutoTest)

#include "autotest.moc"
