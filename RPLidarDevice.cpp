#include "RPLidarDevice.h"

// create the class and constructors


RPLidarDevice::RPLidarDevice()
{
    status_msg_ = "RPLidar instance created";
    init();
}

RPLidarDevice::~RPLidarDevice()
{
    status_msg_ = "RPLidar destructor called";
}

// initialize the device
void RPLidarDevice::init()
{
    is_connected_ = false;
}

void RPLidarDevice::connect(std::string com_port, std::string baudrate)
{
    status_msg_ = "Connecting to RPLidar";
    printf(com_port.c_str());
    printf(baudrate.c_str());

    //convert baudrate to int
    int baudrate_int = std::stoi(baudrate);

    drv_ = *createLidarDriver();

    if (!drv_) {
        status_msg_ = "Error: Failed to create lidar driver";
        return;
    }

    is_connected_ = false;

    channel_ = (*createSerialPortChannel(com_port, baudrate_int));
    if (SL_IS_OK((drv_)->connect(channel_))) {
        op_result_ = drv_->getDeviceInfo(devinfo_);

        if (SL_IS_OK(op_result_)) 
        {
            is_connected_ = true;
        }
        else{
            delete drv_;
            drv_ = NULL;
        }
    }
}

void RPLidarDevice::disconnect()
{
    status_msg_ = "Disconnecting from RPLidar";
    // if (drv) {
    // 	drv->stop();
    // 	delete drv;
    // 	drv = NULL;
    // }

    is_connected_ = false;
}

void RPLidarDevice::start_scan()
{
    
}

void RPLidarDevice::stop_scan()
{
    
}

void RPLidarDevice::calibrate()
{
    status_msg_ = "Calibrating RPLidar";
}

void RPLidarDevice::reset()
{
    status_msg_ = "Resetting RPLidar";
}

std::string RPLidarDevice::get_device_status()
{
    return status_msg_;
}

bool RPLidarDevice::is_connected()
{
    return is_connected_;
}




