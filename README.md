# meshtastic_fw_myself

个人 Meshtastic 固件构建仓库。每个设备/版本使用独立分支，并在 GitHub Releases 提供可直接刷写的固件包。

## 固件列表

| 设备 | 编译环境 | Meshtastic 版本 | 分支 | Release |
|---|---|---|---|---|
| GAT562 Mesh Tracker Max（摇杆版，简体中文） | `gat562_family_cn` | 2.8.1.8e6a88d | `gat562-max/2.8.1.8` | `gat562-max-v2.8.1.8` |
| SakuraPi Pocket Namiji（ESP32-C3） | `sakurapi-namiji` | 2.8.1.8e6a88d | `sakurapi-namiji/2.8.1.8` | `sakurapi-namiji-v2.8.1.8` |

Release 下载：[github.com/splendidmata/meshtastic_fw_myself/releases](https://github.com/splendidmata/meshtastic_fw_myself/releases)

## 命名规范

- 分支：`<设备>/<Meshtastic版本>`，例如 `gat562-max/2.8.1.8`
- 标签：`<设备>-v<Meshtastic版本>`，例如 `gat562-max-v2.8.1.8`
- Release 附件：
  - `meshtastic_<设备>_<版本>.zip`：BLE / MTools OTA 包
  - `meshtastic_<设备>_<版本>.uf2`：USB 拖拽刷机包

## 刷机

### USB（推荐）

1. 按住设备右侧按键不放，用左侧开关开机，进入 DFU。
2. 电脑出现 GAT562 磁盘后，把 `.uf2` 拖入磁盘。

### 蓝牙 OTA

1. 按住摇杆不放，用左侧开关开机，进入 BLE DFU。
2. 使用 MTools BLE App 或 Meshtastic App 选择 `.zip` 刷入。
3. 从 MeshCore / WhisperOS 切换时，建议先执行 ERASE。

## GAT562 Mesh Tracker Max 2.8.1.8e6a88d

- 上游基线：`meshtastic/firmware` 标签 `v2.8.1.8e6a88d`
- 厂商支持：`gat-iot/firmware` 分支 `gat-iot`
- 编译环境：`gat562_family_cn`（摇杆版，简体中文 + 拼音）
- 资源占用：RAM 97,620 / 248,832；Flash 671,468 / 815,104
- 已在真机验证：刷机、启动、版本号、配置保留、电源/环境遥测、GPS 定位
- 注意：主频道位置精度为 0 时不会写入/广播经纬度，需要在 App 或 CLI 中开启位置共享

## SakuraPi Pocket Namiji 2.8.1.8e6a88d

- 上游基线：`meshtastic/firmware` 标签 `v2.8.1.8e6a88d`
- 厂商支持：`splendidmata/meshtastic_fw`（ssp97 的 ESP32-C3 适配与低功耗补丁）
- 编译环境：`sakurapi-namiji`
- 硬件型号：`SAKURAPI_NAMIJI = 254`
- 关键配置：SX1268（E22-400M33S）、GPS RX/TX = GPIO3/4、电池 ADC = GPIO1、I2C SDA/SCL = GPIO2/0
- 低功耗：启用 modem sleep，CPU 80MHz，BLE 广播间隔 500–1000ms
- 资源占用：RAM 110,056 / 327,680；Flash 2,189,109 / 4,194,304
- 已在真机验证：刷机、启动、版本号、BLE、WiFi、LoRa 收发、GPS 模块通信与 NMEA 授时
- 注意：室内通常搜不到卫星（`Sats=0`），定位需在窗边或室外冷启动 1–3 分钟

### 刷机（ESP32-C3）

完整镜像从 `0x0` 写入：

```bash
python -m esptool --chip esp32c3 -p COMx -b 460800 write_flash 0x0 firmware-sakurapi-namiji-2.8.1.8e6a88d.factory.bin
```

或在源码分支目录内直接：

```bash
python -m platformio run -e sakurapi-namiji -t upload
```

## 重新编译

```bash
python -m venv .venv
.venv/Scripts/pip install "setuptools<72" "platformio==6.1.19"
.venv/Scripts/pio run -e gat562_family_cn
```

产物在 `.pio/build/gat562_family_cn/`。`version.properties` 已固定 `long = 2.8.1.8e6a88d`，重新编译不会改变版本号。
