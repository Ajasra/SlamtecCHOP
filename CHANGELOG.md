# Changelog

## [Unreleased] - 2025-12-13

### Fixed

- **TouchDesigner freeze on startup/reload**: Fixed blocking SDK calls that caused TD to hang when loading projects with the plugin in Active state
  - Identified `getMotorInfo()`, `setMotorSpeed()`, and `startScan()` as blocking calls on S-series lidars
  - Commented out `getMotorInfo()` which hangs indefinitely on S2
  - Made motor control model-aware: only call `setMotorSpeed()` for A-series (model < 24)
  - Switched from `startScan()` to `startScanExpress()` as primary scan method

- **No scan data displayed**: Changed data retrieval from `getScanDataWithIntervalHq()` to `grabScanDataHq()`
  - `getScanDataWithIntervalHq()` was returning uninitialized memory (0xCCCCCCCC values)
  - `grabScanDataHq()` with timeout=0 properly returns valid scan data

- **Plugin reinit/reload crash**: Fixed thread lifecycle management
  - Changed from detached thread to joinable thread with proper cleanup
  - Added 2-second timeout on thread join to prevent indefinite hangs
  - Added `_stop_requested` atomic flag for clean thread termination
  - Reset `is_busy_` flag in `init_data()` to allow reconnection after failed attempts

- **Baudrate menu mismatch**: Fixed baudrate array to match parameter menu options
  - Array was missing 460800 entry, causing incorrect baudrate selection for indices 2-3

### Changed

- **Model-aware connection logic**: Added detection for S-series vs A-series lidars
  - S-series (model >= 24): Skip `setMotorSpeed()`, use `startScanExpress()` only
  - A-series (model < 24): Call `setMotorSpeed()`, fallback to `startScan()` if express fails

- **Build configuration**:
  - Updated project paths from hardcoded `M:\` to relative `$(ProjectDir)` paths
  - Removed per-file compile overrides that caused runtime library mismatches
  - Updated toolset to v143 (VS2022)

### Added

- **Debug logging**: Added timestamped debug output via `OutputDebugStringA()`
  - Use DebugView to capture logs filtered by "SlamtecCHOP" or "RPLidarDevice"
  - Logs connection flow, scan data stats, and error conditions

- **CLAUDE.md**: Added project documentation for Claude Code assistance
  - Build setup instructions
  - Architecture overview
  - Model-specific SDK behavior reference
  - Debugging instructions

### Technical Details

**Root Cause Analysis**: The original freeze occurred because certain Slamtec SDK functions block indefinitely on S-series lidars (S1/S2/S3) that have internal motor control. When TD loaded a project with the plugin saved in Active state, the connection thread would call these blocking functions during TD's initialization, preventing the main thread from completing startup.

**Files Modified**:
- `SlamtecCHOP.cpp` - Added debug logging, startup delay infrastructure
- `SlamtecCHOP.h` - Added startup frame counter and debug helper
- `drvlogic/RPLidarDevice.cpp` - Model-aware connection, fixed thread cleanup, changed scan method
- `drvlogic/RPLidarDevice.h` - Added atomic stop flag
- `BasicGeneratorCHOP.vcxproj` - Fixed paths, removed compile overrides
