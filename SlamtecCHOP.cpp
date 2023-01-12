/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "SlamtecCHOP.h"
#include "Parameters.h"

#include <cassert>
#include <stdio.h>
#include <string>

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// For more information on CHOP_PluginInfo see CHOP_CPlusPlusBase.h

	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// For more information on OP_CustomOPInfo see CPlusPlus_Common.h
	OP_CustomOPInfo& customInfo = info->customOPInfo;

	// Unique name of the node which starts with an upper case letter, followed by lower case letters or numbers
	customInfo.opType->setString("Slamtec");
	// English readable name
	customInfo.opLabel->setString("SlamtecLidar");
	customInfo.opIcon->setString("SLT");
	// Information of the author of the node
	customInfo.authorName->setString("Vasily Betin");
	customInfo.authorEmail->setString("b@vasily.onl");

	// This CHOP takes no inputs
	customInfo.minInputs = 0;
	customInfo.maxInputs = 0;
}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	// printf("SlamtecCHOP::CreateCHOPInstance\n");
	return new SlamtecCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL

	// printf("SlamtecCHOP:DestroyCHOPInstance\n");
	delete (SlamtecCHOP*)instance;
}

};


SlamtecCHOP::SlamtecCHOP(const OP_NodeInfo*)
{
	debug_info("Constructor called");
	init();
}

SlamtecCHOP::~SlamtecCHOP()
{
	debug_info("Destructor called");
}

void
SlamtecCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs*, void*)
{
	// This chop doesn't need to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = false;
	ginfo->inputMatchIndex = 0;
}

bool
SlamtecCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void*)
{
	info->numChannels = num_channels_;
	info->numSamples = num_samples_;
	info->startIndex = 0;

	return true;
}

void
SlamtecCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs*, void*)
{
	std::string full_label;

	switch (coord_)
	{
		case CoordMenuItems::Polar:
			if (index == 0)
				full_label = "angle";
			else if (index == 1)
				full_label = "distance";
			break;
		case CoordMenuItems::Cartesian:
			if (index == 0)
				full_label = "x";
			else if (index == 1)
				full_label = "y";
			break;
	}

	name->setString(full_label.c_str());
}

void
SlamtecCHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void*)
{
	my_execute_count_++;

	// update parameters
	is_active_	= Parameters::evalActive(inputs) == 1;
	coord_		= Parameters::evalCoord(inputs);

	inputs->enablePar(ResetName, is_active_);
	
	if (is_active_ && !lidar->is_connected())
	{
		const std::string com_port = inputs->getParString(PortName);
		const std::string baudrate = inputs->getParString(BaudrateName);
		lidar->connect(com_port, baudrate);
	}
	else if (!is_active_ && lidar->is_connected())
	{
		lidar->disconnect();
	}

	// Get length and channels from the output since they first need to be set to the current size
	const int		channels = output->numChannels;

	// If not active or lidar not connected, return empty data
	if(!is_active_ || !lidar->is_connected())
	{
		for (int i = 0; i < channels; ++i)
		{
			for (int j = 0; j < num_samples_; ++j)
			{
				output->channels[i][j] = 0.0;
			}
		}
		return;
	}
	
}

void
SlamtecCHOP::setupParameters(OP_ParameterManager* manager, void*)
{
	Parameters::setup(manager);
}

void
SlamtecCHOP::init()
{
	my_execute_count_ = 0;
	num_samples_ = 0;
	
	is_was_active_ = false;
	
	lidar = new RPLidarDevice();
}

void 
SlamtecCHOP::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		debug_info("Reset pressed");
		lidar->reset();
	}
}

// Info outputs
int
SlamtecCHOP::getNumInfoCHOPChans(void* reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 1;
}

void 
SlamtecCHOP::getInfoCHOPChan(int32_t index,
	OP_InfoCHOPChan* chan,
	void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = static_cast<float>(my_execute_count_);
	}
}

bool
SlamtecCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
SlamtecCHOP::getInfoDATEntries(int32_t index,
	int32_t nEntries,
	OP_InfoDATEntries* entries,
	void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");
		// Set the value for the second column
		#ifdef WIN32
			sprintf_s(tempBuffer, "%d", my_execute_count_);
		#else // macOS
			snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
		#endif
		entries->values[1]->setString(tempBuffer);
	}
	
	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("Lidar status");
		// Set the value for the second column
		#ifdef WIN32
			const std::string status = lidar->get_device_status();
		#else // macOS
			snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
		#endif
		entries->values[1]->setString(status.c_str());
	}
}

// helper functions
// Print out to console
void 
SlamtecCHOP::debug_info(const char* message)
{
	std::string combined;
	combined += "SlamtecCHOP :: ";
	combined += message;
	combined += "\n";
	printf(combined.c_str());

	/*
	printf("\n"
		"Firmware Ver: %d.%02d\n"
		"Hardware Rev: %d\n",
		devinfo.firmware_version >> 8, devinfo.firmware_version & 0xFF, (int)devinfo.hardware_version);
		*/
}