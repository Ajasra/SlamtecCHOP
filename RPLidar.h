#pragma once
#include <string>

#include "sl_lidar.h" 
#include "sl_lidar_driver.h"
#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

using namespace sl;

class RPLidarDevice
{
public:

    RPLidarDevice();              // Constructor
    ~RPLidarDevice();             // Destructor

    void init();
    void connect(std::string com_port, std::string baudrate);
    void disconnect();

    static void start_scan();
    static void stop_scan();

    void calibrate();
    void reset();

    std::string get_device_status();

    bool is_connected();

private:

    std::string status_msg_;

    bool is_connected_;

    IChannel* channel_;
    ILidarDriver * drv_;

    sl_lidar_response_device_info_t devinfo_;
    sl_result     op_result_;
    
};
