#include <QDebug>
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
            //QWARN(QString::number(type).toStdString().c_str());
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
    for (int i = 0; i < 1000*1000; i++) {
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

void AutoTest::cleanupTestCase() {}

QTEST_APPLESS_MAIN(AutoTest)

#include "autotest.moc"
