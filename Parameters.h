#pragma once

class OP_Inputs;
class OP_ParameterManager;

#pragma region ParNames and ParLabels

// Names of the parameters

constexpr static char PageName[] = "Lidar Settings";

constexpr static char DefaultName[] = "Default";
constexpr static char DefaultLabel[] = "Run with Default Settings";

constexpr static char StandartModeName[] = "Standartmode";
constexpr static char StandartModeLabel[] = "Standart Mode";

constexpr static char ActiveName[] = "Active";
constexpr static char ActiveLabel[] = "Active";

constexpr static char PortName[] = "Comport";
constexpr static char PortLabel[] = "COM Port";

constexpr static char BaudrateName[] = "Baudrate";
constexpr static char BaudrateLabel[] = "Baudrate";

constexpr static char SpeedName[] = "Motorspeed";
constexpr static char SpeedLabel[] = "Motor Speed";

constexpr static char SetSpeedName[] = "Setspeed";
constexpr static char SetSpeedLabel[] = "Set Speed";

constexpr static char CoordsName[] = "Coordsystem";
constexpr static char CoordsLabel[] = "Coordinate System";




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
	static int				evalMotorSpeed(const OP_Inputs* input);

	// Coordinate system
	static CoordMenuItems	evalCoord(const OP_Inputs* input);

};
#pragma endregion