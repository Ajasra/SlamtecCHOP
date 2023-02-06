# TouchDesigner Slamtec Lidar support
CPlusPlus CHOP for TouchDesigner to read data from Slamtec Lidar.
Pleaase check the Dubug folder or [release](https://github.com/Ajasra/SlamtecCHOP/releases) for example.

### Supported:
- RPLIDAR A1 / A2 / A3
- RPLIDAR S1 / S2

For the setup check your COM port in the windows manager and setup baundrate to match your lidar model.
You can add the infoDAT to see lidar information.

### Compiling
1. Clone this repository
2. Clone the [rplidar sdk](https://github.com/Slamtec/rplidar_sdk)
3. Open in Visual Studio 2019
4. Add the rplidar sdk to the project
5. Follow [this steps](https://github.com/Slamtec/rplidar_sdk/issues/71#issuecomment-1382005055)s to setup right linking
6. Build

#### TODO:
- [x] Support for all serial baudrates
- [x] Receive lidar information and available modes
- [ ] Support for the network models over TCP/UDP
- [ ] Settings for the lidar (change mode, change motor speed, etc)

Did you find it useful? - support by [buying me a book](https://www.buymeacoffee.com/vasilyonl)
