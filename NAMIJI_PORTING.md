# Namiji 移植与版本跟进指南

面向 SakuraPi Pocket Namiji（ESP32-C3，无屏、无按键）的固件移植记录，重点说明如何跟随 Meshtastic 官方主线制作新版本，以及 GCJ-02 坐标转换的落地方式。

当前参考版本：`v2.8.1.8e6a88d`

| 项 | 值 |
|---|---|
| 分支 | `sakurapi-namiji/2.8.1.8` |
| 编译环境 | `sakurapi-namiji` |
| 硬件型号 | `SAKURAPI_NAMIJI = 254` |
| 标签 | `sakurapi-namiji-v2.8.1.8` |

---

## 1. 分支与基线策略

与 GAT562 一致：**不与厂商分支合并，只按补丁重放。**

- 基线取官方 tag `v2.8.1.8e6a88d`
- 设备适配与功能补丁作为独立提交叠在 tag 上
- 每个新版本新建分支，不覆盖旧版本
- Namiji 没有中文 UI 需求（无屏），不需要 `-cn` 分支

2.8.1.8 参考提交：

| 提交 | 内容 |
|---|---|
| `927c7503` | SakuraPi Pocket Namiji 适配（变体、环境、传感器、低功耗） |
| `29750108` | GCJ-02 坐标转换（`Gcj02.*` + `PhoneAPI` 挂点） |

---

## 2. 跟随官方主线的流程

```bash
# 1. 同步官方标签
git fetch origin --tags
git ls-remote --tags origin "v2.8*"

# 2. 以新 tag 建分支
git checkout -b sakurapi-namiji/<新版本> v<新tag>

# 3. 重放设备移植提交
git cherry-pick <namiji 适配提交>

# 4. 重放功能提交（GCJ-02 等）
git cherry-pick <gcj02 提交>

# 5. 编译
pio run -e sakurapi-namiji

# 6. 立刻把产物拷出（切换 env 会清掉 build 目录）
cp .pio/build/sakurapi-namiji/*.factory.bin <归档目录>/
```

### 预期冲突点

| 文件 | 原因 |
|---|---|
| `src/mesh/PhoneAPI.cpp` | 手机协议出口/入口，官方每个版本都可能变动 |
| `variants/esp32c3/namiji/variant.h` | 引脚定义，需与厂商硬件对齐 |
| `variants/esp32c3/namiji/platformio.ini` | 基础 flags、传感器开关 |
| `src/graphics/generated/*` | 无屏变体仍需编译，注意 `MESHTASTIC_EXCLUDE_SCREEN` |
| `src/mesh/generated/*` | protobuf 重新生成后字段名可能变化 |

---

## 3. 设备移植清单

1. **变体目录**：`variants/esp32c3/namiji/variant.h`
   - LoRa（SX1268 / E22-400M33S）：`SCK 6`、`MISO 9`、`MOSI 8`、`CS 7`、`RESET 10`、`DIO1 20`、`BUSY 21`、`RXEN 5`
   - GPS：`RX 3` / `TX 4`
   - I2C：`SDA 2` / `SCL 0`（AHT10 挂在 `0x38`）
   - 电池 ADC：`GPIO1`，`ADC_MULTIPLIER 5.52`
   - **没有** `BUTTON_PIN`、`LED_PIN`、加速度计
2. **构建环境**：`variants/esp32c3/namiji/platformio.ini`
   - `board = esp32-c3-devkitm-1`、`board_build.f_cpu = 80000000L`
   - `-D SAKURAPI_NAMIJI`、`-D HAS_SCREEN=0`、`-D I2C_NO_RESCAN=1`
   - `ARDUINO_USB_MODE=1`、`ARDUINO_USB_CDC_ON_BOOT=1`
3. **低功耗**：modem sleep、CPU 80MHz、BLE 广播间隔放宽
4. **无声光反馈**：所有 UI / 蜂鸣依赖必须可空跑

---

## 4. GCJ-02 坐标转换

### 4.1 设计约束

- 只改**交给手机**的坐标，LoRa 空口与内部存储始终 WGS-84
- 关闭时行为与官方固件完全一致
- 不修改 protobuf schema

### 4.2 开关载体

Namiji 没有屏幕和按键，配置只能由手机通过 `AdminMessage` 下发；因此开关必须落在某个官方已有字段上。

选用 `config.device.double_tap_as_button_press`：

| App 里「Double tap as button press」 | 字段值 | GCJ-02 |
|---|---|---|
| 关（出厂默认） | `false` | 关闭 |
| 开 | `true` | 开启 |

- 该字段需要加速度计才有实际作用，Namiji 没有加速度计，因此完全空转
- 它是**正向布尔**，默认 `false`，所以 App 开关位置与功能同向
- 持久化由 NodeDB 自带，**不需要**额外的 flag 文件

> 曾尝试借用 `led_heartbeat_disabled`，但 App 里该字段的 UI 是反向的（开关打开 = 字段 false），容易误导，已弃用。

### 4.3 代码落点

| 文件 | 作用 |
|---|---|
| `src/gps/Gcj02.h` / `Gcj02.cpp` | 算法、开关判定、包级转换 |
| `src/mesh/PhoneAPI.cpp` | 5 个挂点 |

`PhoneAPI` 挂点：

1. `STATE_SEND_OWN_NODEINFO` 出口 —— 本机 NodeInfo
2. `STATE_SEND_OTHER_NODEINFOS` 出口 —— 其它节点 NodeInfo
3. 实时包出口 —— `packetForPhone`
4. 缓存重发出口 —— `popReplayPacket`
5. 入口 —— `handleToRadio`，**只转 `WAYPOINT_APP`**

入口只转航点的原因：`POSITION_APP` 来自系统定位服务（CoreLocation / Android LocationManager），本身已是 WGS-84；而 iOS 地图标点来自国内底图（GCJ-02），必须反算。

### 4.4 算法要点

- Krasovsky 1940 椭球：`a = 6378245.0`，`ee = 0.00669342162296594323`
- 境外判定：`lon 72.004–137.8347`、`lat 0.8293–55.8271`
- 反算用 5 次不动点迭代
- 航点除主坐标外，还要同步处理 `bounding_box` 的四个角

---

## 5. 构建与刷机

### 编译

```bash
pio run -e sakurapi-namiji
```

产物：

- `.pio/build/sakurapi-namiji/firmware-sakurapi-namiji-<版本>.factory.bin`（从 `0x0` 写入）
- `.pio/build/sakurapi-namiji/firmware-sakurapi-namiji-<版本>.bin`（OTA app）
- `.pio/build/sakurapi-namiji/littlefs-sakurapi-namiji-<版本>.bin`

### 刷机

```bash
python -m esptool --chip esp32c3 -p COMx -b 460800 write_flash 0x0 firmware-sakurapi-namiji-<版本>.factory.bin
```

- 完整镜像覆盖 `0x0`–`0x22ffff`，**会重置 NVS**：WiFi、频道、开关状态都要重新配置
- 刷写后串口端口可能重新枚举（例如 COM5 → COM6），设备管理器里确认新端口
- 刷完建议彻底断电再上电，避免首次启动时 I2C 初始化卡住

---

## 6. 回归清单

1. 编译通过，确认 `RAM / Flash` 打印
2. 串口启动日志：版本号、`AHT10 found at address 0x38`、LoRa 初始化、区域与频率
3. App 蓝牙连接、设备名、节点信息
4. WiFi 连接与网络状态（如需 MQTT）
5. LoRa 收发：广播文本、私聊、航点
6. 遥测：温度 / 湿度 / 电压上报
7. **GCJ-02**：
   - 开关关闭 → App 收到 WGS-84
   - 开关打开 → App 收到 GCJ-02（昆明一带约偏移 400 米）
   - 断电重启后开关状态保持
   - LoRa 空口坐标不受开关影响
8. 远程管理：对端节点公钥加入 `security.admin_key` 后可远程改配置

---

## 7. 常见问题

- **`SCons.Tool.FortranCommon` 缺失**：PlatformIO 6.2 核心要求 `tool-scons 4.11.1`，pioarduino 平台要求 `4.8.1`，两者互相覆盖。把核心 `dependencies.py` 与平台 `platform.json` 的 `tool-scons` 统一指向 `scons-local-4.8.1`，并保证包内 `scons.py` 位于根目录
- **`platforms.lock` 权限错误**：构建工具被限制写入 `~/.platformio`，需要给该目录读写权限
- **串口 `PermissionError`**：手机 App 通过 USB 连接、或上一个监听脚本未退出，都会占用端口
- **启动日志停在 `Load /prefs/config.proto`**：factory 刷写后首次启动可能在 I2C 初始化处卡住，断电重上电即可
- **切换 env 会清空 build 目录**：每个 `pio run` 只保留当前环境，产物必须立刻归档
- **WiFi 密码修改后仍失败一次**：首次连接可能命中旧缓存，重新上电后再试

---

## 8. 发布约定

- 分支：`sakurapi-namiji/<Meshtastic版本>`
- 标签：`sakurapi-namiji-v<Meshtastic版本>`
- Release 附件：
  - `meshtastic_sakurapi-namiji_<版本>.zip`：含 `factory.bin`、OTA `.bin`、`littlefs.bin`、`.mt.json`
  - `source-sakurapi-namiji_<版本>.zip`：分支源码快照
  - `SHA256SUMS.txt`：附件校验值
- 提交作者：`splendidmata <28244088+splendidmata@users.noreply.github.com>`
- 发布仓库保持线性历史：用 `git commit-tree <本地tree> -p <远端tip>` 构造提交后推送，不做强推
