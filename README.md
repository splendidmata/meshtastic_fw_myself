# meshtastic_fw_myself

个人 Meshtastic 固件构建仓库。每个设备/版本使用独立分支，并在 GitHub Releases 提供可直接刷写的固件包。

## 固件列表

| 设备 | 编译环境 | Meshtastic 版本 | 分支 | Release |
|---|---|---|---|---|
| GAT562 Mesh Tracker Max（摇杆版，英文） | `gat562_family` | 2.8.1.8e6a88d | `gat562-max/2.8.1.8` | `gat562-max-v2.8.1.8` |
| GAT562 Mesh Tracker Max（摇杆版，简体中文） | `gat562_family_cn` | 2.8.1.8e6a88d | `gat562-max/2.8.1.8-cn` | `gat562-max-v2.8.1.8` |
| SakuraPi Pocket Namiji（ESP32-C3） | `sakurapi-namiji` | 2.8.1.8e6a88d | `sakurapi-namiji/2.8.1.8` | `sakurapi-namiji-v2.8.1.8` |

Release 下载：[github.com/splendidmata/meshtastic_fw_myself/releases](https://github.com/splendidmata/meshtastic_fw_myself/releases)

分支与移植细节：

- GAT562：[GAT562_PORTING.md](GAT562_PORTING.md)
- Namiji：[NAMIJI_PORTING.md](NAMIJI_PORTING.md)

## 命名规范

- 分支：`<设备>/<Meshtastic版本>`，中文分支追加 `-cn`
- 标签：`<设备>-v<Meshtastic版本>`
- Release 附件：
  - `meshtastic_<设备>_<版本>.zip`：BLE / MTools OTA 包
  - `meshtastic_<设备>_<版本>.uf2`：USB 拖拽刷机包（nRF52）
  - `meshtastic_<设备>-cn_<版本>.zip/.uf2`：中文版
  - `source-<设备>_<版本>.zip`：对应分支源码快照
  - `SHA256SUMS.txt`：全部附件校验值

## 本版功能（2.8.1.8e6a88d）

两台设备共同具备：

- **GCJ-02 坐标转换**：交给手机 App 的坐标转为国标坐标，LoRa 空口与内部存储始终 WGS-84
  - GAT562：位置页二级菜单 `GCJ-02 → 开 / 关`
  - Namiji：App 的 `Device Configuration → Double tap as button press` 作为开关（默认关）
- 默认关闭，行为与官方固件一致

GAT562 额外：

- 简体中文界面 + 拼音输入（仅 CN 分支）
- 侧键短按熄屏、任意键唤醒；已移除街机小游戏
- 环境页气压高度：QNH（GPS 校准）/ PAlt(ISA) 回退，含温度修正

Namiji 额外：

- 无屏无按键，全部配置通过 App / 远程管理下发
- AHT10 温湿度、GPS、WiFi/BLE、SX1268 收发

## 刷机

### GAT562（nRF52）

USB（推荐）：

1. 按住设备右侧按键不放，用左侧开关开机，进入 DFU。
2. 电脑出现 GAT562 磁盘后，把 `.uf2` 拖入磁盘。

蓝牙 OTA：

1. 按住摇杆不放，用左侧开关开机，进入 BLE DFU。
2. 使用 MTools BLE App 或 Meshtastic App 选择 `.zip` 刷入。
3. 从 MeshCore / WhisperOS 切换时，建议先执行 ERASE。

### Namiji（ESP32-C3）

完整镜像从 `0x0` 写入：

```bash
python -m esptool --chip esp32c3 -p COMx -b 460800 write_flash 0x0 firmware-sakurapi-namiji-2.8.1.8e6a88d.factory.bin
```

或在源码分支目录内：

```bash
python -m platformio run -e sakurapi-namiji -t upload
```

注意：写完整镜像会覆盖 NVS，WiFi、频道、开关状态需要重新配置。

## 设备资源占用

| 设备 / 环境 | RAM | Flash |
|---|---|---|
| GAT562 `gat562_family` | 95,956 / 248,832 | 656,060 / 815,104（80.5%） |
| GAT562 `gat562_family_cn` | 96,148 / 248,832 | 666,036 / 815,104（81.7%） |
| Namiji `sakurapi-namiji` | — | app 2,173,920 / 4,194,304；factory 2,239,456 |

GAT562 的 Flash 硬上限按 83.5%（约 680 KB）控制，留出 OTA 余量。

## 重新编译

```bash
python -m venv .venv
.venv/Scripts/pip install "setuptools<72" "platformio==6.2.0"
.venv/Scripts/pio run -e gat562_family_cn      # 或 gat562_family / sakurapi-namiji
```

产物在 `.pio/build/<env>/`。`version.properties` 已固定 `long = 2.8.1.8e6a88d`，重新编译不会改变版本号。

编译前注意：

- 每次 `pio run` 只能保留**当前 env** 的 `.pio/build` 目录，切换 env 会清掉上一个，产物要立刻拷出
- PlatformIO 6.2 核心与 pioarduino 平台对 `tool-scons` 版本要求冲突，详见各移植指南的「常见问题」
