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
	return new SlamtecCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	
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
	if(lidar->is_connected())
	{
		lidar->on_disconnect();
	}
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
	info->numSamples = 360 * static_cast<int>(precision_);
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

	if(index == 2)
		full_label = "quality";
	else if(index == 3)
		full_label = "flag";

	name->setString(full_label.c_str());
}

void
SlamtecCHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void*)
{
	my_execute_count_++;

	// update parameters
	is_active_		= Parameters::evalActive(inputs) == 1;
	is_standart_	= Parameters::evalStandart(inputs) == 1;
	coord_			= Parameters::evalCoord(inputs);
	is_network_		= Parameters::evalConnectionType(inputs) == 1;

	precision_		= static_cast<float>(Parameters::evalPrecision(inputs));
	is_quality_		= Parameters::evalQuality(inputs);
	inputs->getParDouble2(DistanceName,distance_min_,distance_max_);

	inputs->enablePar(StandartModeName, !is_active_);
	inputs->enablePar(ConnectName, !is_active_);
	inputs->enablePar(PortName, !is_active_);
	inputs->enablePar(BaudrateName, !is_active_);
	inputs->enablePar(IpName, !is_active_);
	inputs->enablePar(NetworkPortName, !is_active_);
	inputs->enablePar(NetworkPortName, !is_active_);
	inputs->enablePar(PrecisionName, !is_active_);
	inputs->enablePar(QualityName, !is_active_);

	if(!is_active_)
	{
		inputs->enablePar(NetworkPortName, is_network_);
		inputs->enablePar(IpName, is_network_);
		inputs->enablePar(PortName, !is_network_);
		inputs->enablePar(BaudrateName, !is_network_);
	}
	
	if (is_active_ && !lidar->is_connected())
	{
		num_samples_ = static_cast<int>(precision_) * 360;

		if(is_network_)
		{
			const bool udp	= Parameters::evalNetworkType(inputs) == 1;
			const std::string ip_address = inputs->getParString(IpName);
			const int ip_port = inputs->getParInt(NetworkPortName);

			lidar->setLidar(false, ip_address.c_str(), ip_port, precision_, is_quality_, is_standart_, udp);
			lidar->on_connect();
		}else
		{
			const std::string com_port = inputs->getParString(PortName);
			const int baudrate = inputs->getParInt(BaudrateName);

			lidar->setLidar(true, com_port.c_str(), baudrate, precision_, is_quality_, is_standart_, false);
			lidar->on_connect();
		}
	}
	else if (!is_active_ && lidar->is_connected())
	{
		lidar->on_disconnect();
	}

	if(is_active_ && lidar->is_connected())
	{

		lidar->scan(distance_min_ * 1000, distance_max_ * 1000);

		switch (coord_)
		{
			case CoordMenuItems::Polar:
				for(int i = 0; i < num_samples_; i++)
				{
					output->channels[0][i] = static_cast<float>(lidar->data_[i].angle) / precision_;
					output->channels[1][i] = lidar->data_[i].distance;
					output->channels[2][i] = static_cast<float>(lidar->data_[i].quality);
					output->channels[3][i] = static_cast<float>(lidar->data_[i].flag);
				}
				break;
			case CoordMenuItems::Cartesian:
				for(int i = 0; i < num_samples_; i++)
				{
					float angle = static_cast<float>(lidar->data_[i].angle) / precision_ * degreesToRadians;
					output->channels[0][i] = lidar->data_[i].distance * cos(angle);
					output->channels[1][i] = lidar->data_[i].distance * sin(angle);
					output->channels[2][i] = static_cast<float>(lidar->data_[i].quality);
					output->channels[3][i] = static_cast<float>(lidar->data_[i].flag);
				}
			break;
		}
		
	}
	
	// If not active or lidar not connected, return empty data
	if(!is_active_ || !lidar->is_connected())
	{
		for (int i = 0; i < output->numChannels; ++i)
		{
			for (int j = 0; j < num_samples_; ++j)
			{
				output->channels[i][j] = -1.0;
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
	num_samples_ = 360 * static_cast<int>(precision_);
	is_was_active_ = false;
	lidar = new RPLidarDevice();
}

void 
SlamtecCHOP::pulsePressed(const char* name, void* reserved1)
{
	// if(!strcmp(name, "Setspeed"))
	// {
	// 	debug_info("Set speed pressed");
	// 	lidar->setMotorSpeed(motor_speed_);
	// }
}

// Info outputs
int
SlamtecCHOP::getNumInfoCHOPChans(void* reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 3;
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
	if (index == 1)
	{
		chan->name->setString("received Samples");
		chan->value = static_cast<float>(lidar->data_count_);
	}
	if (index == 2)
	{
		chan->name->setString("Random");
		chan->value = static_cast<float>(lidar->_rnd_number);
	}
}

bool
SlamtecCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 11;
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

	if (index == 2)
	{
		// Set the value for the first column
		entries->values[0]->setString("Serial number");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->serial_number;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 3)
	{
		// Set the value for the first column
		entries->values[0]->setString("Firmware version");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->firmware_version;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 4)
	{
		// Set the value for the first column
		entries->values[0]->setString("Hardware version");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->hardware_version;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 5)
	{
		// Set the value for the first column
		entries->values[0]->setString("Model");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->model;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 6)
	{
		// Set the value for the first column
		entries->values[0]->setString("Motor max/min/desired speed");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->max_speed + "/" + lidar->min_speed + "/" + lidar->desired_speed;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 7)
	{
		// Set the value for the first column
		entries->values[0]->setString("Last reading");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = std::to_string(lidar->data_count_);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 8)
	{
		// Set the value for the first column
		entries->values[0]->setString("Available scan modes");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->scanModesStr;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 9)
	{
		// Set the value for the first column
		entries->values[0]->setString("Current scan mode");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->currentScanMode.scan_mode;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}

	if (index == 10)
	{
		// Set the value for the first column
		entries->values[0]->setString("Motor control");
		// Set the value for the second column
		#ifdef WIN32
		const std::string status = lidar->motor_control;
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(status.c_str());
	}
}

void 
SlamtecCHOP::debug_info(const char* message)
{
	std::string combined;
	combined += "SlamtecCHOP :: ";
	combined += message;
	combined += "\n";
	printf(combined.c_str());
	
}