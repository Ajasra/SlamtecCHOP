#include <string>
#include <array>
#include "CPlusPlus_Common.h"
#include "Parameters.h"

#pragma region Evals

int
Parameters::evalActive(const OP_Inputs* input)
{
	return input->getParInt(ActiveName);
}

int
Parameters::evalStandart(const OP_Inputs* input)
{
	return input->getParInt(StandartModeName);
}

int                                                      
Parameters::evalConnectionType(const OP_Inputs* input)         
{                                                        
	return input->getParInt(ConnectName);           
}

int                                                             
Parameters::evalNetworkType(const OP_Inputs* input)          
{                                                               
	return input->getParInt(NetworkTypeName);                       
}                                                               

int
Parameters::evalMotorSpeed(const OP_Inputs* input)
{
	return input->getParInt(SpeedName);
}

CoordMenuItems
Parameters::evalCoord(const OP_Inputs* input)
{
	return static_cast<CoordMenuItems>(input->getParInt(CoordsName));
}

#pragma endregion

#pragma region Setup

void
Parameters::setup(OP_ParameterManager* manager)
{
	// is default
	// {
	// 	OP_NumericParameter isDefault;
	//
	// 	isDefault.name = DefaultName;
	// 	isDefault.label = DefaultLabel;
	// 	isDefault.page = PageName;
	// 	isDefault.defaultValues[0] = 1;
	//
	// 	OP_ParAppendResult res = manager->appendToggle(isDefault);
	// 	assert(res == OP_ParAppendResult::Success);
	// }
	
	// is active
	{
		OP_NumericParameter isActive;

		isActive.name = ActiveName;
		isActive.label = ActiveLabel;
		isActive.page = PageMainName;
		isActive.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendToggle(isActive);
		assert(res == OP_ParAppendResult::Success);
	}

	// is standart
	{
		OP_NumericParameter isStandart;

		isStandart.name = StandartModeName;
		isStandart.label = StandartModeLabel;
		isStandart.page = PageMainName;
		isStandart.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendToggle(isStandart);
		assert(res == OP_ParAppendResult::Success);
	}

	// Connection type toogle
	{
		OP_NumericParameter isNetwork;

		isNetwork.name = ConnectName;
		isNetwork.label = ConnectLabel;
		isNetwork.page = PageConnectionsName;
		isNetwork.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendToggle(isNetwork);
		assert(res == OP_ParAppendResult::Success);
	}

	// COM Port
	{
		OP_StringParameter cp;

		cp.name = PortName;
		cp.label = PortLabel;
		cp.page = PageConnectionsName;
		cp.defaultValue = "COM3";

		OP_ParAppendResult res = manager->appendString(cp);

		assert(res == OP_ParAppendResult::Success);
	}

	// Bandwitch
	{
		OP_StringParameter bw;

		bw.name = BaudrateName;
		bw.label = BaudrateLabel;
		bw.page = PageConnectionsName;
		bw.defaultValue = "1000000";

		std::array<const char*, 3> Names =
		{
			"115200", "256000", "1000000"
		};
		std::array<const char*, 3> Labels =
		{
			"115200", "256000", "1000000"
		};
		OP_ParAppendResult res = manager->appendMenu(bw, int(Names.size()), Names.data(), Labels.data());

		//OP_ParAppendResult res = manager->appendMenu(sp, int(Names.size()), names, labels);
		assert(res == OP_ParAppendResult::Success);
	}

	// Connection type toogle                                           
	{                                                                   
		OP_NumericParameter isUdp;                                  
                                                                    
		isUdp.name = NetworkTypeName;                                   
		isUdp.label = NetworkPortLabel;                                 
		isUdp.page = PageConnectionsName;                           
		isUdp.defaultValues[0] = 1;                                 
                                                                    
		OP_ParAppendResult res = manager->appendToggle(isUdp);      
		assert(res == OP_ParAppendResult::Success);                     
	}                                                                   

	
	// IP Address
	{
		OP_StringParameter ip;

		ip.name = IpName;
		ip.label = IpLabel;
		ip.page = PageConnectionsName;
		ip.defaultValue = "192.168.11.2";

		OP_ParAppendResult res = manager->appendString(ip);

		assert(res == OP_ParAppendResult::Success);
	}

	// Network port
	{
		OP_StringParameter np;

		np.name = NetworkPortName;
		np.label = NetworkPortLabel;
		np.page = PageConnectionsName;
		np.defaultValue = "8089";

		OP_ParAppendResult res = manager->appendString(np);

		assert(res == OP_ParAppendResult::Success);
	}

	// Motor speed
	// {
	// 	OP_NumericParameter	np;
	//
	// 	np.name = SpeedName;
	// 	np.label = SpeedLabel;
	// 	np.page = PageName;
	// 	np.minSliders[0] = 480;
	// 	np.maxSliders[0] = 1200;
	// 	np.minValues[0] = 480;
	// 	np.maxValues[0] = 1200;
	// 	np.clampMins[0] = true;
	// 	np.clampMaxes[0] = true;
	// 	np.defaultValues[0] = 600;
	// 	
	// 	OP_ParAppendResult res = manager->appendInt(np);
	// 	assert(res == OP_ParAppendResult::Success);
	// }
	
	// Coordinate system
	{
		OP_StringParameter sp;

		sp.name = CoordsName;
		sp.label = CoordsLabel;
		sp.page = PageOutputName;
		sp.defaultValue = "Polar";

		std::array<const char*, 2> Names =
		{
			"Polar", "Cartesian"
		};
		std::array<const char*, 2> Labels =
		{
			"Polar", "Cartesian"
		};
		OP_ParAppendResult res = manager->appendMenu(sp, int(Names.size()), Names.data(), Labels.data());

		//OP_ParAppendResult res = manager->appendMenu(sp, int(Names.size()), names, labels);
		assert(res == OP_ParAppendResult::Success);
	}
}

#pragma endregion