
#pragma once
#include "common.h"

class RPLidarDevice {

    public:
        
        RPLidarDevice();
        ~RPLidarDevice();
        
        // connect and disconnect
        bool on_connect();
        void on_disconnect();
        bool thr_connect(bool& serial, std::string& port, int& baudrate, bool& standart, bool& udp);
    
        bool check_device_health(int * errorCode = NULL);
        void get_scan_modes();

        void scan(float min_dist, float max_dist);
        void init_data();

        void update_status();
        
        std::string get_device_status();
        [[nodiscard]] bool is_connected() const { return is_connected_; }

        void setMotorSpeed(int speed);
        void setLidar(bool serial, const char* address_1, int address_2, float precision, bool qualityCheck = true, bool standart = false, bool udp = true);

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

        std::string _address_1;     // serial port / ip
        int _address_2;          // baudrate / network port
        bool _standart;
        bool _udp;
        bool _channelTypeSerial = true;

        int _rnd_number = 0;
        
    protected:

        std::string status_msg_;
        
        bool  is_connected_;
        bool  is_busy_;             // for multithreading

        IChannel* channel_;
        ILidarDriver * lidar_drv_;
        
        sl_result     op_result_;
        sl_lidar_response_device_info_t devinfo_;
        sl_lidar_response_device_health_t healthinfo_;
        LidarMotorInfo motorinfo_;
        MotorCtrlSupport motor_ctrl_support_;

        
        // parameters
        float precision_;
        bool qualityCheck_;

        std::thread _lidarThread;
};