# Changelog

## [Unreleased] - 2025-12-13

### Fixed

- **TouchDesigner freeze on startup/reload**: Fixed blocking SDK calls that caused TD to hang when loading projects with the plugin in Active state
  - Identified `getMotorInfo()`, `setMotorSpeed()`, and `startScan()` as blocking calls on S2 (model 113)
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

### Added

- **Startup delay**: Added 60-frame (~1 second) delay before allowing connection to prevent issues during TD initialization

- **Debug logging**: Added timestamped debug output via `OutputDebugStringA()`
  - Use DebugView to capture logs filtered by "SlamtecCHOP" or "RPLidarDevice"
  - Logs connection flow, model detection, scan data stats, and error conditions

- **CLAUDE.md**: Added comprehensive project documentation including:
  - Build setup instructions
  - Architecture overview
  - Model-specific SDK behavior reference (S2=113, S3=129)
  - Debugging instructions
  - Known issues and failed experiments

### Changed

- **Model-aware connection logic**: Added detection for S-series vs A-series lidars
  - S-series (model >= 24): Skip `setMotorSpeed()`, use `startScanExpress()` only
  - A-series (model < 24): Call `setMotorSpeed()`, fallback to `startScan()` if needed

- **Build configuration**:
  - Updated project paths from hardcoded `M:\` to relative `$(ProjectDir)` paths
  - Removed per-file compile overrides that caused runtime library mismatches
  - Updated toolset to v143 (VS2022)

- **Removed build artifacts from repo**: Updated `.gitignore` to exclude `.vs/`, `x64/`, build outputs

### Known Limitations

- **S3 motor noise**: S3 runs at a fixed motor speed that may be louder than older plugin versions. Motor speed is firmware-controlled on S-series and not adjustable via SDK.
- **A-series baudrate**: Requires manual selection (115200 for A1/A2, 256000 for A3)

### Technical Details

**Root Cause Analysis**: The original freeze occurred because certain Slamtec SDK functions block indefinitely on S2 lidar (model 113) which has internal motor control. When TD loaded a project with the plugin saved in Active state, the connection thread would call these blocking functions during TD's initialization, preventing the main thread from completing startup.

**Files Modified**:
- `SlamtecCHOP.cpp` - Added debug logging, startup delay
- `SlamtecCHOP.h` - Added startup frame counter (STARTUP_DELAY_FRAMES = 60)
- `drvlogic/RPLidarDevice.cpp` - Model-aware connection, fixed thread cleanup, changed scan method
- `drvlogic/RPLidarDevice.h` - Added atomic stop flag
- `BasicGeneratorCHOP.vcxproj` - Fixed paths, removed compile overrides
- `.gitignore` - Added build artifact exclusions
- `CLAUDE.md` - Created project documentation
- `CHANGELOG.md` - Created changelog

**Tested Models**:
- S2 (model 113): ✓ Working
- S3 (model 129): ✓ Working
- A2: Requires correct baudrate selection (not 1000000)
