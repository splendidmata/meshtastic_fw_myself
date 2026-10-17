#include "graphics/ui_translate.h"

#include <cstdint>
#include <cstring>

#if defined(GAT562_CN_UI)

namespace
{

struct UiTranslation {
    const char *en;
    const char *zh;
};

// Keyed by the exact literal used at the call site. Entries are only needed
// for strings this firmware can render; anything else is returned unchanged.
static const UiTranslation kTable[] = {
    // Navigation / confirmations
    {"Back", "返回"},
    {"Exit", "退出"},
    {"OK", "确定"},
    {"Confirm", "确认"},
    {"Reject", "拒绝"},
    {"Accept", "接受"},
    {"Finish", "完成"},
    {"Cancel", "取消"},
    {"Yes", "是"},
    {"No", "否"},
    {"Got it!", "知道了"},
    {"Reboot", "重启"},
    {"Shutdown", "关机"},
    {"Reboot / Shutdown", "重启/关机"},
    {"Reboot/Shutdown", "重启/关机"},
    {"Reboot Device?", "重启设备？"},
    {"Shutdown Device?", "关机设备？"},
    {"Rebooting...", "正在重启..."},
    {"Resuming...", "继续..."},

    // State / toggles
    {"Enabled", "开启"},
    {"Disabled", "关闭"},
    {"On", "开"},
    {"Off", "关"},
    {"All Enabled", "全部开启"},
    {"All Disabled", "全部关闭"},
    {"Toggle Bluetooth", "切换蓝牙"},
    {"Bluetooth", "蓝牙"},
    {"Bluetooth Toggle", "蓝牙开关"},
    {"WiFi", "WiFi"},
    {"WiFi Toggle", "WiFi开关"},
    {"WiFi Enabled", "WiFi已开启"},
    {"WiFi Disabled", "WiFi已关闭"},
    {"WiFi Menu", "WiFi菜单"},
    {"WiFi Actions", "WiFi操作"},
    {"Toggle Backlight", "切换背光"},
    {"Brightness", "亮度"},
    {"Sleep Screen", "休眠屏幕"},
    {"Power", "电源"},
    {"Display Options", "显示选项"},
    {"Display Units", "显示单位"},
    {" Select display units", " 选择显示单位"},
    {"Metric", "公制"},
    {"Imperial", "英制"},
    {"Transmit Enabled", "发射开启"},
    {"Transmit Disabled", "发射已禁用"},
    {"Switch to MUI", "切换到MUI"},
    {"Switch to MUI?", "切换到MUI？"},
    {"Revert to Normal", "恢复正常"},

    // Frames / display
    {"Frame Visibility", "页面显示"},
    {"Show/Hide Frames", "显示/隐藏页面"},
    {"Show Clock", "显示时钟"},
    {"Hide Clock", "隐藏时钟"},
    {"Show Position", "显示位置"},
    {"Hide Position", "隐藏位置"},
    {"Show Power", "显示电源"},
    {"Hide Power", "隐藏电源"},
    {"Show LoRa", "显示LoRa"},
    {"Hide LoRa", "隐藏LoRa"},
    {"Show Chirpy", "显示Chirpy"},
    {"Hide Chirpy", "隐藏Chirpy"},
    {"Show Node Lists", "显示节点列表"},
    {"Hide Node Lists", "隐藏节点列表"},
    {"Show Position Lists", "显示位置列表"},
    {"Hide Position Lists", "隐藏位置列表"},
    {"Show Favorites", "显示收藏"},
    {"Hide Favorites", "隐藏收藏"},
    {"Show Env. Telemetry", "显示环境遥测"},
    {"Hide Env. Telemetry", "隐藏环境遥测"},
    {"Show AQ Telemetry", "显示空气质量"},
    {"Hide AQ Telemetry", "隐藏空气质量"},
    {"Show Bubbles", "显示气泡"},
    {"Hide Bubbles", "隐藏气泡"},
    {"Message Bubbles", "消息气泡"},
    {"Message bubbles enabled", "消息气泡已开启"},
    {"Message bubbles disabled", "消息气泡已关闭"},
    {"Show NL - Bearings", "显示节点方位"},
    {"Hide NL - Bearings", "隐藏节点方位"},
    {"Show NL - Distance", "显示节点距离"},
    {"Hide NL - Distance", "隐藏节点距离"},
    {"Show NL - Hops/Signal", "显示节点跳数/信号"},
    {"Hide NL - Hops/Signal", "隐藏节点跳数/信号"},
    {"Show NL - Last Heard", "显示节点最近听到"},
    {"Hide NL - Last Heard", "隐藏节点最近听到"},
    {"Favorites Only", "仅收藏"},
    {"Favorite Nodes Only", "仅收藏节点"},
    {"Favorites Only: ", "仅收藏: "},
    {"Theme", "主题"},
    {"Node Name Length", "节点名长度"},
    {"Short", "短"},
    {"Long", "长"},
    {"Show Long/Short Name", "显示长/短名"},
    {"Analog", "模拟"},
    {"Digital", "数字"},
    {"Clock Face", "表盘"},
    {"Which Face?", "选择表盘"},
    {"Clock Action", "时钟操作"},
    {"Time Format", "时间格式"},
    {"12-hour", "12小时制"},
    {"24-hour", "24小时制"},
    {"Timezone", "时区"},
    {"Pick Timezone", "选择时区"},
    {"Format", "格式"},
    {"Number Picker", "数字选择"},

    // Home / system / reset
    {"Home", "主页"},
    {"Home Action", "主页操作"},
    {"System", "系统"},
    {"System Action", "系统操作"},
    {"System Only", "仅系统"},
    {"Reset All", "全部重置"},
    {"Reset NodeDB", "重置节点库"},
    {"Confirm Reset NodeDB", "确认重置节点库"},
    {"Preserve Favorites", "保留收藏"},
    {"Initiate node-db reset", "开始重置节点库"},
    {"Initiate node-db reset, keep favorites", "开始重置节点库(保留收藏)"},
    {"Keep licensed", "保持授权"},
    {"Test Announce", "测试广播"},
    {"Test Menu", "测试菜单"},
    {"Hidden Test Menu", "隐藏测试菜单"},
    {"This is a test of the emergency broadcast system. This is only a test.",
     "这是紧急广播系统测试，仅为测试。"},

    // Nodes
    {"Node", "节点"},
    {"Nodes", "节点"},
    {"Node Action", "节点操作"},
    {"Node Actions / Settings", "节点操作/设置"},
    {"Node to Trace", "追踪节点"},
    {"Node to Verify", "验证节点"},
    {"Pick A Node", "选择节点"},
    {"Pick Node", "选择节点"},
    {"Node !%08x", "节点 !%08x"},
    {"Nodenum: 0x%08x", "节点号: 0x%08x"},
    {"Ignore Node", "忽略节点"},
    {"Unignore Node", "取消忽略"},
    {"Favorite", "收藏"},
    {"Unfavorite", "取消收藏"},
    {"Remove Favorite", "取消收藏"},
    {"Favorites", "收藏夹"},
    {"Favorites Action", "收藏操作"},
    {"Key Verification", "密钥验证"},
    {"Verification: \n", "验证: \n"},
    {"Trace Route", "路由追踪"},
    {"Send Node Info", "发送节点信息"},
    {"Send Position", "发送位置"},

    // Messages
    {"Message", "消息"},
    {"Messages", "消息"},
    {"Message Action", "消息操作"},
    {"New Message", "新消息"},
    {"New Freetext Msg", "新的自由文本"},
    {"New Preset", "新预设"},
    {"New Preset Msg", "新的预设消息"},
    {"No messages", "暂无消息"},
    {"Reply", "回复"},
    {"Reply to Channel", "回复频道"},
    {"Reply to DM", "回复私聊"},
    {"Reply to Last Msg", "回复最新消息"},
    {"Go To Chat", "进入聊天"},
    {"View Chats", "查看聊天"},
    {"View All Chats", "查看全部聊天"},
    {"Select Conversation", "选择会话"},
    {"Delete", "删除"},
    {"Delete All", "全部删除"},
    {"Delete All Chats", "删除全部聊天"},
    {"Delete Messages", "删除消息"},
    {"Delete This Chat", "删除此聊天"},
    {"Delete Oldest", "删除最旧"},
    {"Deleting all messages", "正在删除全部消息"},
    {"Deleting all messages in thread", "正在删除此会话消息"},
    {"Deleting oldest message", "正在删除最旧消息"},
    {"Read Aloud", "朗读"},
    {"Mute Notifications", "静音通知"},
    {"Unmute Notifications", "取消静音"},
    {"Mute Channel", "静音频道"},
    {"Unmute Channel", "取消静音频道"},
    {"Unmute", "取消静音"},
    {"Temporarily Mute", "临时静音"},
    {"Notifications", "通知"},
    {"Notification Sounds", "通知声音"},
    {"Alert Received", "收到警报"},
    {"Text Input", "文字输入"},
    {"[Select Destination]", "[选择目标]"},
    {"[-- Free Text --]", "[-- 自由文本 --]"},

    // Waypoints / geofence
    {"Waypoint Action", "航点操作"},
    {"Remove Waypoint", "删除航点"},
    {"Unnamed Waypoint", "未命名航点"},
    {"Unnamed Geofence", "未命名围栏"},
    {"Geofence Alerts", "围栏提醒"},
    {"No Geofences", "无围栏"},
    {"Enter Alerts: ", "进入提醒: "},
    {"Exit Alerts: ", "离开提醒: "},
    {"Lost and Found", "失物招领"},

    // GPS / compass / position
    {"GPS Action", "GPS操作"},
    {"GPS Format", "GPS格式"},
    {"Toggle GPS", "切换GPS"},
    {"Compass Calibrate", "指南针校准"},
    {"North Directions?", "北方方向？"},
    {"Compass", "指南针"},
    {"Position", "位置"},
    {"Smart Position", "智能位置"},
    {"Smrt Postn", "智能位置"},
    {"Toggle Smart Position", "切换智能位置"},
    {"Freeze Heading", "锁定航向"},
    {"Fixed Ring", "固定环"},
    {"Distance", "距离"},
    {"Bearings", "方位"},
    {"Last Heard", "最近听到"},
    {"Hops/Sig", "跳数/信号"},
    {"Hops/Signal", "跳数/信号"},
    {"No GPS", "无GPS"},
    {"No GPS Lock", "GPS未定位"},
    {"No GPS present", "未检测到GPS"},
    {"No Sats", "无卫星"},
    {"No Lock", "未锁定"},
    {"Fix", "已定位"},
    {"Heading", "航向"},
    {"GPS Time Only", "仅GPS时间"},
    {"GPS off", "GPS已关闭"},
    {"BT off", "蓝牙已关闭"},
    {"Fixed GPS", "固定位置"},

    // Coordinate formats
    {"DEC", "十进制"},
    {"DMS", "度分秒"},
    {"Decimal Degrees", "十进制度"},
    {"Degrees Minutes Seconds", "度分秒"},
    {"Maidenhead Locator", "梅登黑德网格"},
    {"Military Grid Reference System", "军事网格参考"},
    {"Universal Transverse Mercator", "通用横轴墨卡托"},
    {"Open Location Code", "开放位置代码"},
    {"Ordnance Survey Grid Ref", "英国地形网格"},

    // LoRa / radio
    {"LoRa Actions", "LoRa操作"},
    {"LoRa Region", "LoRa区域"},
    {"Set the LoRa region", "设置LoRa区域"},
    {"LoRa", "LoRa"},
    {"LoRa Info", "LoRa信息"},
    {"Radio Preset", "无线预设"},
    {"Frequency Slot", "频率槽"},
    {"Slot %lu", "槽位 %lu"},
    {"Slot 0 (Auto)", "槽位0(自动)"},
    {"Broadcast Interval", "广播间隔"},
    {"Update Interval", "更新间隔"},
    {"FEM LNA", "前放低噪放"},

    // Device roles
    {"Device Role", "设备角色"},
    {"Client", "客户端"},
    {"Client Mute", "客户端静音"},
    {"Client Hidden", "隐藏客户端"},
    {"Tracker", "追踪器"},
    {"Router", "路由器"},
    {"Router Late", "延迟路由"},
    {"Repeater", "中继"},

    // Status / debug labels
    {"ChUtil:", "信道占用:"},
    {"Up:", "运行:"},
    {"Up: ", "运行: "},
    {"Hop:", "跳:"},
    {"Last: ", "上次: "},
    {"Last: ?", "上次: ?"},
    {"Heard:?", "听到:?"},
    {" Heard:?", " 听到:?"},
    {"Signal:", "信号:"},
    {"Heap:", "内存:"},
    {"Flash:", "闪存:"},
    {"PSRAM:", "PSRAM:"},
    {"SD:", "SD:"},
    {"Ver: %.*s", "版本: %.*s"},
    {"App Connected (BLE)", "App已连接(BLE)"},
    {"App Connected (WiFi)", "App已连接(WiFi)"},
    {"App Connected (Ethernet)", "App已连接(以太网)"},
    {"App Connected (HTTP)", "App已连接(HTTP)"},
    {"App Connected (Serial)", "App已连接(串口)"},
    {"App Connected (Internal)", "App已连接(内部)"},
    {"No Apps Connected", "无App连接"},
    {"WiFi: Connected", "WiFi: 已连接"},
    {"WiFi: Not Connected", "WiFi: 未连接"},
    {"SSID Not Found", "未找到SSID"},
    {"Connection Lost", "连接丢失"},
    {"Connection Failed", "连接失败"},
    {"Idle ... Reconnecting", "空闲...重连中"},
    {"Local", "本地"},
    {"Bad", "差"},
    {"Fair", "一般"},
    {"Good", "良好"},
    {"Plugged In", "已插电"},
    {"Sleeping", "休眠中"},
    {"Screen Paused", "屏幕暂停"},
    {"Out of Boundary", "超出范围"},
    {"Altitude: %.0fft", "海拔: %.0fft"},
    {"Altitude: %.0im", "海拔: %.0im"},
    {"Alt: %.0fft", "海拔: %.0fft"},
    {"Alt: %.0im", "海拔: %.0im"},
    {"Steps: ", "步数: "},

    // Updating / certificate banners
    {"Creating SSL certificate", "正在创建SSL证书"},
    {"Please be patient and do not power off.", "请耐心等待，勿断电。"},
    {"Please wait . . .", "请稍候..."},
    {"Please wait . .  ", "请稍候.."},
    {"Updating", "正在更新"},

    // Frame titles
    {"Alert", "警报"},
    {"Clock", "时钟"},
    {"Games", "游戏"},
    {"Chirpy", "Chirpy"},
    {"Module", "模块"},

    // Welcome banners
    {"Welcome to Meshtastic!\nSwipe to navigate and\nlong press to select\nor open a menu.",
     "欢迎使用 Meshtastic！\n滑动浏览\n长按选择\n或打开菜单"},
    {"Welcome to Meshtastic!\nClick to navigate and\nlong press to select\nor open a menu.",
     "欢迎使用 Meshtastic！\n点击浏览\n长按选择\n或打开菜单"},
    {"Welcome to Meshtastic!\nUse the Select button\nto open menus\nand make selections.",
     "欢迎使用 Meshtastic！\n用选择键\n打开菜单\n并做出选择"},

    // Module names
    {"Canned Message", "预设消息"},
    {"External Notification", "外部通知"},
    {"Range Test", "距离测试"},
    {"Telemetry", "遥测"},
    {"Environment Telemetry", "环境遥测"},
    {"Air Quality Telemetry", "空气质量"},
    {"Device Telemetry", "设备遥测"},
    {"Waypoint", "航点"},
    {"Detection Sensor", "检测传感器"},
    {"Store & Forward", "存储转发"},
    {"StoreForward", "存储转发"},
    {"Remote Hardware", "远程硬件"},
    {"Serial", "串口"},
    {"Paxcounter", "客流计数"},
    {"Status Message", "状态消息"},
    {"Traffic Management", "流量管理"},
    {"Mesh Beacon", "网格信标"},
    {"Geofence", "地理围栏"},
    {"Audio", "音频"},

    // Dynamically formatted labels (format specifiers are preserved)
    {"%u sats", "%u颗卫星"},
    {"Sig:%s", "信号:%s"},
    {"Fav: %s", "收藏: %s"},
    {"Alert Received from\n%s", "来自以下节点的警报\n%s"},
    {"New Message from\n%s", "来自以下节点的新消息\n%s"},
    {"Uptime:%s Ver:%s", "运行:%s 版本:%s"},
    {"Ver: %s", "版本: %s"},
    {"No %s Connected", "无%s连接"},
    {"%s Connected (BLE)", "%s已连接(BLE)"},
    {"%s Connected (WiFi)", "%s已连接(WiFi)"},
    {"%s Connected (Serial)", "%s已连接(串口)"},
    {"%s Connected (Internal)", "%s已连接(内部)"},
    {"%s Connected (HTTP)", "%s已连接(HTTP)"},
    {"%s Connected (Ethernet)", "%s已连接(以太网)"},
    {"App: %s", "应用: %s"},
    {"Clients", "客户端"},
    {"Apps", "应用"},
    {"None", "无"},
    {"Eth", "以太网"},
    {"For help, please visit \nmeshtastic.org", "如需帮助，请访问\nmeshtastic.org"},

    // Position / status / environment labels
    {"On/Off Toggle", "GPS开关"},
    {"online", "在线"},
    {"Role: %s", "角色: %s"},
    {"Freq: %sMHz", "频率: %sMHz"},
    {"Freq: %sMHz (%d)", "频率: %sMHz (%d)"},
    {"Lat: %f", "纬度: %f"},
    {"Lon: %f", "经度: %f"},
    {"Lat: %2i° %2i' %2u\" %1c", "纬度: %2i° %2i' %2u\" %1c"},
    {"Lon: %3i° %2i' %2u\" %1c", "经度: %3i° %2i' %2u\" %1c"},
    {"Client Base", "客户端基站"},
    {"Sensor", "传感器"},
    {"TAK Tracker", "TAK追踪器"},

    // Environment telemetry
    {"Local Sensor", "本地传感器"},
    {"Tmp: ", "温度: "},
    {"Hum: ", "湿度: "},
    {"Prss: ", "气压: "},
    {"Light: ", "光照: "},
    {"White: ", "白光: "},
    {"Weight: ", "重量: "},
    {"Level: ", "液位: "},
    {"Rad: ", "辐射: "},
    {"IAQ: ", "空气: "},
    {" (Excellent)", "(优)"},
    {" (Good)", "(良)"},
    {" (Moderate)", "(中)"},
    {" (Poor)", "(差)"},
    {" (Unhealthy)", "(不健康)"},
    {" (Very Unhealthy)", "(很不健康)"},
    {" (Hazardous)", "(危险)"},
    {"Unhealthy IAQ", "空气质量不健康"},
    {"Very Unhealthy IAQ", "空气质量很差"},
    {"Hazardous IAQ", "空气质量危险"},

    // Bluetooth pairing screen / status banners
    {"Enter this code", "请输入配对码"},
    {"Name: ", "名称: "},
    {"Bluetooth OFF\nRebooting", "蓝牙已关闭\n正在重启"},
    {"Bluetooth ON\nRebooting", "蓝牙已开启\n正在重启"},
    {"Bluetooth OFF", "蓝牙已关闭"},
    {"Bluetooth ON", "蓝牙已开启"},
    {"Source", "数据源"},
    {"Environment", "环境"},
    {"PAlt: ", "气压高度: "},
    {"PAlt(ISA): ", "气压高度(ISA): "},
};

} // namespace

const char *uiTr(const char *text)
{
    if (!text)
        return text;

    // The renderers call uiTr() for the same literals on every frame. A tiny
    // pointer-keyed cache keeps repeated frames from walking the whole table.
    static const char *cacheKey[16] = {nullptr};
    static const char *cacheValue[16] = {nullptr};
    const uintptr_t slot = (reinterpret_cast<uintptr_t>(text) >> 4) & 15;
    if (cacheKey[slot] == text)
        return cacheValue[slot];

    for (const UiTranslation &entry : kTable) {
        if (strcmp(text, entry.en) == 0) {
            cacheKey[slot] = text;
            cacheValue[slot] = entry.zh;
            return entry.zh;
        }
    }

    cacheKey[slot] = text;
    cacheValue[slot] = text;
    return text;
}

#else

const char *uiTr(const char *text)
{
    return text;
}

#endif
