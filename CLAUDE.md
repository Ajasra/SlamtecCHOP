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

- **SlamtecCHOP** (`SlamtecCHOP.cpp/.h`): Main TouchDesigner CHOP interface. Handles parameter updates, lidar connection lifecycle, and outputs 4 channels (angle/x, distance/y, quality, flag) with 360 * precision samples.

- **RPLidarDevice** (`drvlogic/RPLidarDevice.cpp/.h`): Lidar driver wrapper. Manages connection via serial or TCP/UDP, scan data acquisition, and device info retrieval. Connection runs in a detached background thread.

- **Parameters** (`Parameters.cpp/.h`): Parameter definitions and evaluation helpers. Defines three parameter pages: "Lidar Settings", "Connection Settings", "Output Settings".

### Data Flow

1. User enables "Active" parameter in TouchDesigner
2. `SlamtecCHOP::execute()` calls `RPLidarDevice::on_connect()` which spawns `thr_connect()` on a joinable thread
3. Background thread establishes serial/network channel, retrieves device info, starts scanning
4. Each `execute()` frame calls `RPLidarDevice::scan()` which polls `grabScanDataHq()` and populates `data_[]` array
5. `execute()` reads `data_[]` and outputs to CHOP channels in polar or cartesian coordinates

### Model-Specific Behavior

The SDK behaves differently across lidar models. Detection is based on `devinfo_.model`:

| Model Series | Model ID | Motor Control | Scan Method |
|--------------|----------|---------------|-------------|
| A-series (A1/A2/A3) | < 24 | `setMotorSpeed()` required | `startScanExpress()`, fallback to `startScan()` |
| S-series (S1/S2/S3) | >= 24 | Internal (skip `setMotorSpeed()`) | `startScanExpress()` only |

**Blocking SDK Calls to Avoid on S-series:**
- `getMotorInfo()` - hangs indefinitely on S2
- `setMotorSpeed()` - hangs on S2 (motor is internally controlled)
- `startScan()` - hangs on S2 (use `startScanExpress()` instead)

**Data Retrieval:**
- Use `grabScanDataHq()` with timeout=0 for polling
- `getScanDataWithIntervalHq()` returns uninitialized data on some models

### Key Data Structures

- `lidar_data` struct (in `drvlogic/common.h`): angle, distance, quality, flag
- `data_[]` array: 1440 samples (720 * 2) to accommodate precision up to 4x

## Debugging

Debug logging is enabled via `OutputDebugStringA()`. Use [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) to capture output:
1. Run DebugView as Administrator
2. Enable "Capture Global Win32"
3. Filter by "SlamtecCHOP" or "RPLidarDevice"

Log format: `[timestamp_ms] SlamtecCHOP :: message` or `[timestamp_ms] RPLidarDevice :: message`

## Known Issues / TODOs

- SDK 2.1.0 causes crashes - using older SDK version
- V4 version rolled back; V3 release is stable
- C1 support untested

## Parameter Names (for debugging/referencing)

- `Active`, `Standartmode`, `Connectiontype`, `Comport`, `Baudrate`
- `Networktype`, `Ipaddress`, `Ipport`
- `Precision` (1-4), `Qualitycheck`, `Distance`, `Coordsystem` (Polar/Cartesian)
