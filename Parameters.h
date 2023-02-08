#pragma once

class OP_Inputs;
class OP_ParameterManager;

#pragma region ParNames and ParLabels

// Names of the parameters

constexpr static char PageMainName[] = "Lidar Settings";
constexpr static char PageConnectionsName[] = "Connection Settings";
constexpr static char PageOutputName[] = "Output Settings";

constexpr static char DefaultName[] = "Default";
constexpr static char DefaultLabel[] = "Run with Default Settings";

constexpr static char StandartModeName[] = "Standartmode";
constexpr static char StandartModeLabel[] = "Standart Mode";

constexpr static char ActiveName[] = "Active";
constexpr static char ActiveLabel[] = "Active";

constexpr static char ConnectName[] = "Connectiontype";
constexpr static char ConnectLabel[] = "Serial / TCP";

constexpr static char PortName[] = "Comport";
constexpr static char PortLabel[] = "COM Port";

constexpr static char BaudrateName[] = "Baudrate";
constexpr static char BaudrateLabel[] = "Baudrate";

constexpr static char NetworkTypeName[] = "Networktype";
constexpr static char NetworkTypeLabel[] = "TCP / UDP";

constexpr static char IpName[] = "Ipaddress";
constexpr static char IpLabel[] = "IP Address";

constexpr static char NetworkPortName[] = "Ipport";
constexpr static char NetworkPortLabel[] = "Network port";

constexpr static char SpeedName[] = "Motorspeed";
constexpr static char SpeedLabel[] = "Motor Speed";

constexpr static char SetSpeedName[] = "Setspeed";
constexpr static char SetSpeedLabel[] = "Set Speed";

constexpr static char CoordsName[] = "Coordsystem";
constexpr static char CoordsLabel[] = "Coordinate System";

constexpr static char PrecisionName[] = "Precision";
constexpr static char PrecisionLabel[] = "Precision";

constexpr static char QualityName[] = "Qualitycheck";
constexpr static char QualityLabel[] = "Quality Check";

constexpr static char DistanceName[] = "Distance";
constexpr static char DistanceLabel[] = "Distance (m)";

#pragma endregion

#pragma region Menus

enum class CoordMenuItems
{
	Polar, 
	Cartesian
};

#pragma endregion


#pragma region Parameters
class Parameters
{
public:
	static void				setup(OP_ParameterManager*);

	// Active
	static int				evalActive(const OP_Inputs* input);
	static int				evalStandart(const OP_Inputs* input);
	static int				evalQuality(const OP_Inputs* input);

	static int				evalConnectionType(const OP_Inputs* input);
	static int				evalNetworkType(const OP_Inputs* input);
	
	static int				evalMotorSpeed(const OP_Inputs* input);
	static int				evalPrecision(const OP_Inputs* input);

	// Coordinate system
	static CoordMenuItems	evalCoord(const OP_Inputs* input);

};
#pragma endregion