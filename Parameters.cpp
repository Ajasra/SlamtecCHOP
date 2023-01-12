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

BaudMenuItems
Parameters::evalBaud(const OP_Inputs* input)
{
	return static_cast<BaudMenuItems>(input->getParInt(BaudrateName));
}

CoordMenuItems
Parameters::evalCoord(const OP_Inputs* input)
{
	return static_cast<CoordMenuItems>(input->getParInt(CoordsName));
}

PortMenuItems
Parameters::evalPort(const OP_Inputs* input)
{
	return static_cast<PortMenuItems>(input->getParInt(PortName));
}



#pragma endregion

#pragma region Setup

void
Parameters::setup(OP_ParameterManager* manager)
{
	
	// is active
	{
		OP_NumericParameter isActive;

		isActive.name = ActiveName;
		isActive.label = ActiveLabel;
		isActive.page = PageName;
		isActive.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendToggle(isActive);
		assert(res == OP_ParAppendResult::Success);
	}

	// COM Port
	{
		OP_StringParameter cp;

		cp.name = PortName;
		cp.label = PortLabel;
		cp.page = PageName;
		cp.defaultValue = "COM3";
		std::array<const char*, 11> Names =
		{
			"COM0", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM9", "COM10"
		};
		std::array<const char*, 11> Labels =
		{
			"COM0", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM9", "COM10"
		};
		OP_ParAppendResult res = manager->appendMenu(cp, int(Names.size()), Names.data(), Labels.data());

		assert(res == OP_ParAppendResult::Success);
	}

	// pulse
	{
		OP_NumericParameter np;

		np.name = ResetName;
		np.label = ResetLabel;
		np.page = PageName;

		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Coordinate system
	{
		OP_StringParameter sp;

		sp.name = CoordsName;
		sp.label = CoordsLabel;
		sp.page = PageName;
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

	// Bandwitch
	{
		OP_StringParameter bw;

		bw.name = BaudrateName;
		bw.label = BaudrateLabel;
		bw.page = PageName;
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
}

#pragma endregion