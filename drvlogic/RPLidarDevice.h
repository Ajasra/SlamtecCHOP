
#pragma once
#include "common.h"

class RPLidarDevice {

public:
    
    RPLidarDevice();
    ~RPLidarDevice();
    void setPrecision(float precision, bool qualityCheck);
    bool thr_connect(std::string& port, int& baudrate, bool& standart);
    bool on_connect(const char * port, int baudrate, bool standart = false);
    bool on_connect_tcp(const char * ip, int port, bool udp);
    // bool on_connect_udp(const char * ip, int port);
    void on_disconnect();
    bool check_device_health(int * errorCode = NULL);
    void setMotorSpeed(int speed);
    [[nodiscard]] bool is_connected() const { return is_connected_; }

    void get_scan_modes();

    void scan(float min_dist, float max_dist);
    void init_data();

    void update_status();
    
    std::string get_device_status();

    // lidar data
    std::string serial_number;
    std::string firmware_version;
    std::string hardware_version;
    std::string model;
    // motor data
    std::string max_speed;
    std::string min_speed;
    std::string desired_speed;
    std::string motor_control;
    //scan modes
    std::vector<LidarScanMode> scanModes;
    std::string scanModesStr = "";
    LidarScanMode currentScanMode;
    
    lidar_data data_[720*2];
    int data_count_;

    std::string port_s;
    int _baudrate;
    bool _standart;
    
protected:

    std::string status_msg_;
    
    bool  is_connected_;
    bool  is_busy_;             // for multithreading
    
    sl_result     op_result_;
    sl_lidar_response_device_info_t devinfo_;
    sl_lidar_response_device_health_t healthinfo_;
    LidarMotorInfo motorinfo_;
    ILidarDriver * lidar_drv_;
    IChannel* channel_;

    MotorCtrlSupport motor_ctrl_support_;

    
    // parameters
    bool channelTypeSerial_ = false;
    float precision_;
    float qualityCheck_;

    std::thread _lidarThread;
};