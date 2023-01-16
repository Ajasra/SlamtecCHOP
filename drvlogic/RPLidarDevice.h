
#pragma once
#include "common.h"

class RPLidarDevice {

public:
    
    RPLidarDevice();
    ~RPLidarDevice();
    bool on_connect(const char * port, int baudrate);
    void on_disconnect();
    bool check_device_health(int * errorCode = NULL);
    [[nodiscard]] bool is_connected() const { return is_connected_; }

    void scan(float min_dist, float max_dist);
    void init_data();

    void update_status();
    
    std::string get_device_status();

    std::string serial_number;
    std::string firmware_version;
    std::string hardware_version;
    std::string model;
    std::string max_speed;
    std::string min_speed;
    std::string desired_speed;

    lidar_data data_[720];
    int data_count_;
    
protected:
    bool  is_connected_;
    std::string status_msg_;
    
    sl_result     op_result_;
    sl_lidar_response_device_info_t devinfo_;
    sl_lidar_response_device_health_t healthinfo_;
    LidarMotorInfo motorinfo_;
    ILidarDriver * lidar_drv_;
    IChannel* channel_;
};