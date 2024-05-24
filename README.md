# TouchDesigner Slamtec Lidar support
CPlusPlus CHOP for TouchDesigner to read data from Slamtec Lidar.
Please check the Dubug folder or [release](https://github.com/Ajasra/SlamtecCHOP/releases) for example.

### Supported:
RPLIDAR A1 / A2 / A3 / S1 / S2 / S3 / C1

For the setup check your COM port in the Windows manager and set up the baud rate to match your lidar model.
You can add the infoDAT to see lidar information.

### Compiling
1. Clone this repository
2. Clone the [rplidar sdk](https://github.com/Slamtec/rplidar_sdk)  (currently compiled with 2.1.0)
3. Open in Visual Studio 2022
4. Add the rplidar SDK to the project
5. Follow [this steps](https://github.com/Slamtec/rplidar_sdk/issues/71#issuecomment-1382005055)s to setup right linking
6. Build

#### TODO:
- [x] Support for all serial baudrates
- [x] Receive lidar information and available modes
- [x] Support for the network models over TCP/UDP
- [x] Settings for the lidar (change mode, change motor speed, etc)
- [x] Add C1 support

Did you find it useful? - support by [buying me a book](https://www.buymeacoffee.com/vasilyonl)
