# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TouchDesigner C++ CHOP plugin for reading data from Slamtec Lidar sensors (RPLIDAR A1/A2/A3/S1/S2/S3). The plugin provides real-time point cloud data via serial or TCP/UDP network connections.

## Build Setup

1. Clone the [rplidar SDK](https://github.com/Slamtec/rplidar_sdk) to `../rplidar_sdk` relative to this project
2. Open `SlamtecCHOP.sln` in Visual Studio 2022 (v143 toolset)
3. Build rplidar_driver project first, or use pre-built `libs/x64/rplidar_driver.lib`
4. Ensure Windows 10 SDK is installed
5. Build in Debug (uses `/MTd`) or Release (uses `/MD`) - must match rplidar_driver.lib
6. Output: `x64/Debug/SlamtecCHOP.dll` or `x64/Release/SlamtecCHOP.dll`

**Important**: Close TouchDesigner before rebuilding - the DLL will be locked if TD has it loaded.

## Architecture

### Core Components

- **SlamtecCHOP** (`SlamtecCHOP.cpp/.h`): Main TouchDesigner CHOP interface. Handles parameter updates, lidar connection lifecycle, and outputs 4 channels (angle/x, distance/y, quality, flag) with 360 * precision samples. Includes a 60-frame startup delay to prevent connection during TD initialization.

- **RPLidarDevice** (`drvlogic/RPLidarDevice.cpp/.h`): Lidar driver wrapper. Manages connection via serial or TCP/UDP, scan data acquisition, and device info retrieval. Connection runs in a joinable background thread with proper cleanup on disconnect.

- **Parameters** (`Parameters.cpp/.h`): Parameter definitions and evaluation helpers. Defines three parameter pages: "Lidar Settings", "Connection Settings", "Output Settings".

### Data Flow

1. User enables "Active" parameter in TouchDesigner
2. `SlamtecCHOP::execute()` waits for startup delay (60 frames), then calls `RPLidarDevice::on_connect()`
3. `on_connect()` spawns `thr_connect()` on a joinable thread
4. Background thread establishes serial/network channel, retrieves device info, starts scanning
5. Each `execute()` frame calls `RPLidarDevice::scan()` which polls `grabScanDataHq()` and populates `data_[]` array
6. `execute()` reads `data_[]` and outputs to CHOP channels in polar or cartesian coordinates

### Model-Specific Behavior

The SDK behaves differently across lidar models. Known model IDs:

| Model | Model ID | Motor Control | Notes |
|-------|----------|---------------|-------|
| A1/A2/A3 | < 24 | `setMotorSpeed()` required | Use baudrate 115200 or 256000 |
| S2 | 113 | Internal (hangs on `setMotorSpeed()`) | Most SDK calls that block will hang |
| S3 | 129 | Internal | Works with most SDK calls |

**Blocking SDK Calls - Model Compatibility:**

| SDK Call | A-series | S2 (113) | S3 (129) |
|----------|----------|----------|----------|
| `getMotorInfo()` | ✓ | ✗ HANGS | ? |
| `setMotorSpeed()` | ✓ Required | ✗ HANGS | ✓ Works but no effect |
| `startScan()` | ✓ | ✗ HANGS | ✓ Works but no data |
| `startScanExpress()` | ✓ | ✓ | ✓ |
| `grabScanDataHq()` | ✓ | ✓ | ✓ |

**Current Implementation:**
- Skip `setMotorSpeed()` for S-series (model >= 24) to avoid S2 hang
- Use `startScanExpress(false, 0, 0, ...)` for all models
- Fall back to `startScan()` only for A-series if express fails

### Key Data Structures

- `lidar_data` struct (in `drvlogic/common.h`): angle, distance, quality, flag
- `data_[]` array: 1440 samples (720 * 2) to accommodate precision up to 4x

## Debugging

Debug logging is enabled via `OutputDebugStringA()`. Use [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) to capture output:
1. Run DebugView as Administrator
2. Enable "Capture Global Win32"
3. Filter by "SlamtecCHOP" or "RPLidarDevice"

Log format: `[timestamp_ms] SlamtecCHOP :: message` or `[timestamp_ms] RPLidarDevice :: message`

Key debug messages to look for:
- `Model ID: X (S-series: yes/no)` - confirms model detection
- `startScanExpress returned: 0` - successful scan start
- `scan: result=0x0, count=N, write_count=M` - data retrieval stats

## Known Issues / Limitations

- **SDK 2.1.0 causes crashes** - using older SDK version
- **S3 motor noise**: S3 runs louder than some older plugin versions. Attempts to control via scan mode selection (Standard vs Express/DenseBoost) or `setMotorSpeed()` were unsuccessful - motor speed appears to be internally fixed on S-series.
- **S2 blocking calls**: Many SDK calls hang indefinitely on S2. The current code carefully avoids these.
- **A-series baudrate**: A1/A2 typically need 115200, A3 needs 256000. Using wrong baudrate causes `getDeviceInfo()` timeout.
- **C1 support**: Untested

## Failed Experiments (for future reference)

### Motor Speed / Noise Control on S3

Attempted approaches that did NOT reduce S3 motor noise:

1. **Scan mode selection via `startScanExpress(mode)`**: Tried modes 0 (Standard, 62us), 1 (DenseBoost, 31us), etc. No audible difference.

2. **`startScan()` vs `startScanExpress()`**: `startScan()` succeeds on S3 but returns no valid data with `grabScanDataHq()`. Would need different data retrieval method.

3. **`setMotorSpeed()` on S3**: Call succeeds but has no effect on motor RPM. S-series motors are internally controlled.

The motor speed on S-series appears to be firmware-controlled and not adjustable via SDK.

## Parameter Names (for debugging/referencing)

- `Active`, `Standartmode`, `Connectiontype`, `Comport`, `Baudrate`
- `Networktype`, `Ipaddress`, `Ipport`
- `Precision` (1-4), `Qualitycheck`, `Distance`, `Coordsystem` (Polar/Cartesian)
