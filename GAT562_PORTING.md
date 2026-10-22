# GAT562 移植与版本跟进指南

面向 GAT562 Mesh Tracker Max（摇杆版）与 GAT562 T9（键盘版）的固件移植记录，重点说明如何跟随 Meshtastic 官方主线制作新版本。

当前参考版本：`v2.8.1.8e6a88d`

| 分支 | 用途 | 编译环境 |
|---|---|---|
| `gat562-max/2.8.1.8` | 英文版 | `gat562_family` / `gat562_t9` |
| `gat562-max/2.8.1.8-cn` | 中文版（简体 + 拼音） | `gat562_family_cn` / `gat562_t9_cn` |

---

## 1. 分支与基线策略

核心原则：**不与厂商业务分支合并，只按补丁重放。**

- 基线永远取官方 tag，例如 `v2.8.1.8e6a88d`；设备支持作为一个提交叠在 tag 上
- 厂商 `gat-iot/firmware` 的 `gat-iot` 分支只作为补丁来源，不整分支 merge
- 每个新版本新建分支，不覆盖旧版本分支
- 汉化只放在 CN 分支，EN 分支不引入 `ui_translate.*` 与中文字库，避免占用 Flash

2.8.1.8 参考提交：

| 提交 | 内容 |
|---|---|
| `25ed80df` | GAT562 设备移植（板级、变体、输入、CJK、拼音、厂商功能） |
| `dfd8fbc3` / `547a0e29` | 环境页气压高度 |
| `f67ba2ca` / `c7590e89` | QNH 海压高度（GPS 校准） |
| `880af617` / `7b2f003b` | 侧键熄屏、移除街机小游戏 |
| `f6823411` / `f6d339cc` | GCJ-02 坐标转换（出口）与中文词条 |
| `078ff8b5` / `40e0acdc` | App 航点反向转换（入口） |
| `41dbb5cd` / `539ef904` | 开关持久化修复 |
| `95865612` / `9db09d3d` | GCJ-02 开关改为二级菜单 |

---

## 2. 跟随官方主线的流程

```bash
# 1. 同步官方标签，确认目标版本
git fetch origin --tags
git ls-remote --tags origin "v2.8*"

# 2. 以新 tag 建基线分支
git checkout -b gat562-<新版本> v<新tag>

# 3. 重放设备移植提交
git cherry-pick <port提交>
# 冲突时优先接受官方结构，再把厂商逻辑手工套回去

# 4. 重放后续功能提交
git cherry-pick <气压高度提交> <QNH提交> <按键裁剪提交>

# 5. 编译英文环境
python -m platformio run -e gat562_family

# 6. 建中文分支并重放汉化提交
git checkout -b gat562-<新版本>-cn
git cherry-pick <汉化提交>

# 7. 编译中文环境
python -m platformio run -e gat562_family_cn
```

### 预期冲突点

| 文件 | 原因 |
|---|---|
| `src/graphics/Screen.cpp` | 输入事件、帧切换、汉化收口点，官方改动频繁 |
| `src/input/InputBroker.cpp` | 按键事件枚举、ALT/USER 键配置 |
| `src/graphics/VirtualKeyboard.cpp` | 官方键盘布局改动 |
| `src/graphics/draw/*.cpp` | UI 重绘、缓冲区大小 |
| `variants/.../platformio.ini` | 官方基础 flags 与 lib_deps 变化 |
| `src/graphics/OLEDDisplay*` | 本项目使用独立 `lib/OLEDDisplayCJK`，需核对显示接口签名 |
| `src/mesh/generated/*` | protobuf 重新生成后字段/枚举名可能变化 |

---

## 3. 设备移植的分层清单

一次完整移植包含五层，按顺序处理：

1. **板级**
   - `boards/gat562_mesh_trial_tracker.json`
   - `boards/gat562_t9.json`
2. **引脚**：`variants/nrf52840/gat562_mesh_trial_tracker/variant.h`
   - I2C `13/14`、GPS `15/16`、LoRa `42/47/46/38/37`
   - 电池 A0、按键 `9/12`、摇杆 `28/4/30/31/26`
   - 板载 BME280 地址 `0x77`
3. **构建环境**：`variants/nrf52840/gat562_mesh_trial_tracker/platformio.ini`
   - `PRIVATE_HW`、hw_model slug、传感器开关、`lib_deps`
   - `GAT562_CN_UI=1` 仅加在 CN 环境
4. **输入 / 显示**
   - `src/input/InputBroker.cpp`、`src/graphics/Screen.cpp`
   - `src/graphics/VirtualKeyboard.*`、`src/input/GAT562T9Keyboard.*`
5. **功能补丁**
   - GPS、环境遥测、身份与 BLE 名称、区域功率、低功耗、汉化

重建移植文件清单：

```bash
git diff --name-status v<官方tag> <port提交>
```

---

## 4. 关键实现落点

### 4.1 按键与熄屏

事件链：

```
ButtonThread -> InputBroker -> Screen::handleInputEvent
```

事件类型：

- `INPUT_BROKER_USER_PRESS`：侧键短按
- `INPUT_BROKER_ALT_PRESS`：第二按键短按
- `INPUT_BROKER_SELECT` / `SELECT_LONG`：摇杆按下 / 长按

实现要点：

- 熄屏使用 `handleSetOn(false)`
- 唤醒由 `InputBroker` 先触发 `EVENT_INPUT`，再丢弃唤醒那一次事件（`screenWasOff` 分支），因此不会出现“亮起后立即熄灭”
- 摇杆版与 T9 的短按行为统一为熄屏；翻页交给摇杆左右
- 调试按键时必须同时加 `-DInputBrokerDebug=1` 并去掉 `-DDEBUG_MUTE`，否则日志全部静音

### 4.2 环境页布局与气压高度

- 指标采用两列流式布局，第一个指标会被抽到发送者所在行
- 追加内容要记录目标指标在 `entries` 中的索引，画完该行后再插入并 `currentY += rowHeight`
- 单位统一读取 `config.display.units`
- 气压高度公式（ISA）：

```
h = 44330 * (1 - (P / 1013.25)^(1 / 5.25588))
```

### 4.3 QNH 海压高度

气压计测到的是本站气压，QNH 必须借助外部海拔反推：

```
QNH = P * (1 - L*h / (T + 273.15 + L*h))^(-5.25588)
L = 0.0065 K/m
```

实现策略：

- GPS 有锁且 `localPosition.has_altitude` 时校准，最多每 60 秒刷新一次
- 无锁时沿用缓存值；从未校准时回退标准大气（1013.25 hPa）
- QNH 合理区间限制为 900–1100 hPa，拒绝异常样本
- 显示标签区分：`QNH:`（已校准）与 `PAlt(ISA):`（回退标准大气）
- 温度修正：`h_corr = h_ISA * (T_meas + 273.15) / (288.15 - 0.0065 * h_ISA)`

### 4.4 汉化

- 集中词表：`src/graphics/ui_translate.cpp` / `ui_translate.h`
- 收口点：`showOverlayBanner`、`PUSH_FRAME_TITLE`、`drawCommonHeader`、各 Renderer 的固定文案
- 未命中必须回退英文，保证官方新增内容不崩溃
- 中文 UTF-8 更长，所有 `snprintf` 缓冲区需要放大
- 原本依赖英文字符串的逻辑判断需要中英双兼容，例如 `Signal:`、`Alert Received`、`Favorite`
- 用 `GAT562_CN_UI` 宏隔离，EN 环境不编译词表
- 字库：CN 用 `gat562_utf8_10x10.h`（CJK 6448 字），EN 用 `gat562_ascii_only_10x10.h`

### 4.5 GCJ-02 坐标转换

目标：交给手机 App 的坐标显示在国内底图上不偏移，同时不污染 LoRa 空口。

- 只改 `PhoneAPI` 边界：出口（本机/他机 NodeInfo、实时包、缓存重发包）转 GCJ-02，入口只对 `WAYPOINT_APP` 反算回 WGS-84
- `POSITION_APP` 入口不转：系统定位服务本身给的就是 WGS-84；iOS 地图标点给的是 GCJ-02
- LoRa 广播、屏幕显示、内部存储始终 WGS-84
- 开关默认关闭，关闭时与官方固件行为一致

开关持久化（nRF52 特有，务必注意）：

- 标志写在 `/prefs/gcj02`，单字节
- `Adafruit_LittleFS` 的 `FILE_O_WRITE` 会把文件指针移到末尾，直接写会变成**追加**，重启后读到的是旧首字节
- 正确做法：写入前先 `FSCom.remove(path)`，再打开写 1 字节

菜单交互：

- 与 GPS 开关保持一致，改为二级菜单 `GCJ-02 → Back / 开 / 关`，进入时预选当前状态
- 不再采用「点一下即翻转」，避免误触


---

## 5. 构建与刷机

### nRF52（GAT562）

```bash
python -m platformio run -e gat562_family_cn
python -m platformio run -e gat562_family_cn -t upload --upload-port COMx
```

产物：

- `.pio/build/<env>/firmware-<env>-<version>.uf2`（USB 拖拽）
- `.pio/build/<env>/firmware-<env>-<version>-ota.zip`（BLE OTA）

USB 刷机：按住右侧按键，拨动左侧开关进入 DFU，把 `.uf2` 拖入磁盘。

### ESP32-C3（Namiji）

```bash
python -m esptool --chip esp32c3 -p COMx -b 460800 write_flash 0x0 firmware-sakurapi-namiji-<version>.factory.bin
```

完整镜像从 `0x0` 写入，注意 NVS 配置存在被覆盖的风险。

---

## 6. 升级后的回归清单

1. 两个环境编译通过，确认 Flash 余量（上限 815,104）：EN 656,060（80.5%）/ CN 666,036（81.7%），控制线 83.5%
2. 串口启动日志：版本号、`SX1268 init success`、区域与频率
3. BLE 配对码页、设备名
4. GPS 定位、NMEA 授时、位置页
5. 环境页：温度 / 湿度 / 气压、`海压高度`（等待 GPS 锁定后再观察）
6. 摇杆四向与按下；侧键短按熄屏、任意键唤醒；长按打开菜单
7. GCJ-02：位置页二级菜单可开 / 关；开启后 App 坐标偏移约 400 米；断电重启后状态保持；LoRa 空口坐标不变
8. 汉化抽查：菜单、通知横幅、LoRa / 位置 / 环境页、蓝牙配对页
9. 中文输入：拼音候选、上下翻页、确认与删除
10. LoRa 收发、节点列表
11. ESP32 设备：WiFi 连接、Web 与 API 端口

---

## 7. 常见问题

- **PlatformIO 与平台抢 `tool-scons`**：PlatformIO 6.2 核心要求 `tool-scons 4.11.1`（缺 `SCons.Tool.FortranCommon`），pioarduino 平台要求 `4.8.1`，每轮构建互相覆盖并最终用错版本。修复：把核心 `dependencies.py` 与平台 `platform.json` 的 `tool-scons` 统一指向 `scons-local-4.8.1`，并保证包根目录存在 `scons.py`
- **`platforms.lock` 写入受限**：构建环境若限制写 `~/.platformio`，`pio run` 启动即失败，需要放开该目录读写
- **写 flag 文件要重建**：nRF52 的 `Adafruit_LittleFS` 以追加方式打开，单字节标志必须先 `remove` 再写，否则永远读到旧值
- **ESP32 依赖下载中断**：使用 `curl -C -` 续传并手工解压，`.piopm` 必须为无 BOM 的 UTF-8
- **串口被占用**：上一个读取脚本未退出会报 `Access to the port is denied`，先确认无残留进程
- **枚举或宏改名**：编译报“未声明”时先检查 `src/mesh/generated` 与 `configuration.h`
- **删除 `lib/` 下目录会触发全量重编**，属于正常现象
- **气压高度不等于真实海拔**：QNE（1013.25）与 QNH（当地海压）必须在标签上区分
- **发布仓库推送**：使用 `commit-tree` 将设备树挂到发布仓库的基线提交上，避免把整段上游历史推入发布仓库

---

## 8. 发布约定

- 分支：`<设备>/<Meshtastic版本>`，中文分支追加 `-cn`
- 标签：`<设备>-v<Meshtastic版本>`
- Release 附件：
  - `meshtastic_<设备>_<版本>.zip`：BLE / MTools OTA 包
  - `meshtastic_<设备>_<版本>.uf2`：USB 拖拽刷机包
  - `meshtastic_<设备>-cn_<版本>.zip/.uf2`：中文版
  - `source-<设备>_<版本>.zip`：分支源码快照（`git archive`）
  - `SHA256SUMS.txt`：全部附件校验值
- 标签需指向最新提交，否则 GitHub 自动生成的 `Source code` 会是旧代码
- 发布仓库保持线性：用 `git commit-tree <本地tree> -p <远端tip>` 构造提交后推送，不做强推
- 提交作者：`splendidmata <28244088+splendidmata@users.noreply.github.com>`
