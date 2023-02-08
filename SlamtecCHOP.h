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

#ifndef __SlamtecCHOP__
#define __SlamtecCHOP__

#include "CHOP_CPlusPlusBase.h"
#include "Parameters.h"

#include "drvlogic/RPLidarDevice.h"

// #include "rplidar_sdk/sdk/include/sl_lidar.h"
// #include "rplidar_sdk/sdk/include/sl_lidar_driver.h"

/*
This example implements a CHOP which takes the following parameters:
	- Length: The number of samples produced per channel.
	- Number of Channels: The number of channels produced.
	- Apply Scale: If On, scale values.
	- Scale: A scalar by which the output signal is scaled.
	- Operation: One of [Add, Multiply, Power] which controls which operation the output signal constitutes

The output values are: scale*(channel operation sample)

This CHOP is a generator so it does not need an input and it is not time sliced.
*/

// Check methods [getNumInfoCHOPChans, getInfoCHOPChan, getInfoDATSize, getInfoDATEntries]
// if you want to output values to the Info CHOP/DAT

// To get more help about these functions, look at CHOP_CPlusPlusBase.h
class SlamtecCHOP : public CHOP_CPlusPlusBase
{
public:
	SlamtecCHOP(const OP_NodeInfo* info);
	virtual ~SlamtecCHOP();

	virtual void		getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void*) override;
	virtual bool		getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
	virtual void		getChannelName(int32_t index, OP_String *name, const OP_Inputs*, void*) override;

	virtual int			getNumInfoCHOPChans(void* reserved1) override;
	virtual void		getInfoCHOPChan(int index,
							OP_InfoCHOPChan* chan,
							void* reserved1) override;

	virtual bool		getInfoDATSize(OP_InfoDATSize* infoSize, void* resereved1) override;
	virtual void		getInfoDATEntries(int32_t index,
							int32_t nEntries,
							OP_InfoDATEntries* entries,
							void* reserved1) override;

	virtual void		execute(CHOP_Output*, const OP_Inputs*, void*) override;

	virtual void		setupParameters(OP_ParameterManager* manager, void*) override;

	virtual void		pulsePressed(const char* name, void* reserved1) override;

	void				init();

private:

	bool	is_active_;
	bool 	is_was_active_;
	bool 	is_standart_;
	bool	is_network_;
	bool    is_quality_;
	
	CoordMenuItems	coord_;

	int32_t	my_execute_count_;
	int		num_samples_;
	int		num_channels_ = 4;
	int		motor_speed_ = 0;
	double	distance_max_ = 40;
	double	distance_min_ = 0;

	float precision_ = 2.0f;
	
	// Helper functions
	static void debug_info(const char* message);

	const OP_NodeInfo* myNodeInfo;

	RPLidarDevice *lidar;

};

const float degreesToRadians = (2.0f * 3.1415926535f) / 360.0f;

#endif
