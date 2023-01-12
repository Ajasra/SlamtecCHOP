#pragma once

class OP_Inputs;
class OP_ParameterManager;

#pragma region ParNames and ParLabels

// Names of the parameters

constexpr static char PageName[] = "Lidar Settings";

constexpr static char ActiveName[] = "Active";
constexpr static char ActiveLabel[] = "Active";

constexpr static char PortName[] = "Comport";
constexpr static char PortLabel[] = "COM Port";

constexpr static char ResetName[] = "Reset";
constexpr static char ResetLabel[] = "Reset";

constexpr static char CoordsName[] = "Coordsystem";
constexpr static char CoordsLabel[] = "Coordinate System";

constexpr static char BaudrateName[] = "Baudrate";
constexpr static char BaudrateLabel[] = "Baudrate";


#pragma endregion

#pragma region Menus

enum class CoordMenuItems
{
	Polar, 
	Cartesian
};


enum class BaudMenuItems
{
	b115200,
	b256000,
	b1000000
};

enum class PortMenuItems
{
	COM0,
	COM1,
	COM2,
	COM3,
	COM4,
	COM5,
	COM6,
	COM7,
	COM8,
	COM9,
	COM10
};

#pragma endregion


#pragma region Parameters
class Parameters
{
public:
	static void				setup(OP_ParameterManager*);

	// Active
	static int				evalActive(const OP_Inputs* input);

	// Port
	static PortMenuItems	evalPort(const OP_Inputs* input);

	// Coordinate system
	static CoordMenuItems	evalCoord(const OP_Inputs* input);

	// Baudrate
	static BaudMenuItems	evalBaud(const OP_Inputs* input);

};
#pragma endregion