#include "RPLidarDevice.h"
#include <thread>

static const int baudRateLists[] = {
    115200,
    256000,
    1000000
};

RPLidarDevice::RPLidarDevice()
{
    
    status_msg_ = "RPLidar instance created";

    is_connected_ = false;
    is_busy_ = false;
    
    serial_number = "";
    hardware_version = "";
    firmware_version = "";
    is_connected_ = false;
    
    lidar_drv_ = NULL;
    channel_ = NULL;
    precision_ = 2.0f;
    qualityCheck_ = false;

    init_data();
}

RPLidarDevice::~RPLidarDevice()
{
    status_msg_ = "RPLidar destructor called";
    on_disconnect();
    delete lidar_drv_;
    lidar_drv_ = NULL;
}

void
RPLidarDevice::setPrecision(float precision, bool qualityCheck)
{
    precision_ = precision;
    qualityCheck_ = qualityCheck;
}

bool
RPLidarDevice::thr_connect(std::string& port, int& baudrate, bool& standart)
{
    channelTypeSerial_ = true;

    if (!lidar_drv_)
        lidar_drv_ = *createLidarDriver();

    if (!(bool)lidar_drv_)
    {
        is_busy_ = false;
        return SL_RESULT_OPERATION_FAIL == 1;
    }
    
    channel_ = (*createSerialPortChannel(port, baudRateLists[baudrate]));
    sl_result ans =(lidar_drv_)->connect(channel_);

    if (SL_IS_FAIL(ans)) {
        printf("Error, cannot bind to the specified serial port %s.\n"
            , port);
        is_busy_ = false;
        return false;
    }
    
    ans = lidar_drv_->getDeviceInfo(devinfo_);
    if (SL_IS_FAIL(ans)) {
        printf("Failed to get device info. code: %x\n", ans);
        is_busy_ = false;
        return false;
    }
    ans = lidar_drv_->getMotorInfo(motorinfo_);

    update_status();

    if(!check_device_health())
    {
        is_busy_ = false;
        return false;
    }

    is_connected_ = true;
    status_msg_ = "Connected to RPLidar";
    
    get_scan_modes();

    if(channelTypeSerial_)
    {
        lidar_drv_->setMotorSpeed();
    }
    
    if(standart)
    {
        lidar_drv_->startScanExpress(0,0,0,&currentScanMode);
    }else
    {
        lidar_drv_->startScan(0,1, 0, &currentScanMode);
    }

    is_busy_ = false;
    return false;
}

bool
RPLidarDevice::on_connect(const char* port, int baudrate, bool standart)
{
    if (is_connected_ || is_busy_) return true;
    // status_msg_ = "Connecting to RPLidar";

    port_s = port;
    _baudrate = baudrate;
    _standart = standart;

    is_busy_ = true;
    _lidarThread = std::thread([this] {this->thr_connect(port_s, _baudrate, _standart);});
    _lidarThread.detach();
    
    return true;
}

bool RPLidarDevice::on_connect_tcp(const char* ip, int port, bool udp = false)
{
    if (is_connected_) return true;
    status_msg_ = "Connecting to RPLidar TCP";

    // printf("Connecting to IP: %s\n", ip);
    // printf("Connecting to PORT: %d\n", port);

    channelTypeSerial_ = false;

    if(udp)
    {
        printf("UDP\n");
        channel_ = *createUdpChannel(ip, port);
    }else
    {
        printf("TCP\n");
        channel_ = *createTcpChannel(ip, port);
    }

    if (!lidar_drv_)
        lidar_drv_ = *createLidarDriver();

    if (!(bool)lidar_drv_) return SL_RESULT_OPERATION_FAIL == 1;
    
    sl_result ans =(lidar_drv_)->connect(channel_);

    if (SL_IS_FAIL(ans)) {
        return false;
    }
    // retrieve the devinfo
    ans = lidar_drv_->getDeviceInfo(devinfo_);

    if (SL_IS_FAIL(ans)) {
        printf("Failed to get device info. code: %x\n", ans);
        return false;
    }
    ans = lidar_drv_->getMotorInfo(motorinfo_);

    update_status();

    if(!check_device_health())
    {
        return false;
    }

    if (SL_IS_FAIL(ans)) {
        return false;
    }
    
    is_connected_ = true;
    return true;
}

void
RPLidarDevice::on_disconnect()
{
    status_msg_ = "Disconnecting from RPLidar";

    if (_lidarThread.joinable())
    {
        _lidarThread.join();
        printf("Thread closed\n");
        //_lidarThread.detach();
        //_lidarThread.~thread();
        //std::terminate();
        is_busy_ = false;
    }
    
    if (is_connected_) {
        lidar_drv_->stop();
        if(channelTypeSerial_) lidar_drv_->setMotorSpeed(0);
    }
    is_connected_ = false;
    delete lidar_drv_;
    lidar_drv_ = NULL;
    delete channel_;
    channel_ = NULL;
    channelTypeSerial_ = false;
    init_data();
}

void
RPLidarDevice::update_status()
{
     // print out the device serial number, firmware and hardware version number..
     printf("SLAMTEC LIDAR S/N: ");
     serial_number = "";
     for (int pos = 0; pos < 16 ;++pos) {
         printf("%02X", devinfo_.serialnum[pos]);
         serial_number += std::to_string(devinfo_.serialnum[pos]);
     }
    
     firmware_version = std::to_string(devinfo_.firmware_version>>8) + ".";
     int minor = devinfo_.firmware_version & 0xFF;
     if(minor < 10) firmware_version += "0" + std::to_string(minor);
     else firmware_version += std::to_string(minor);
     hardware_version = std::to_string(devinfo_.hardware_version);
     model = std::to_string(devinfo_.model);

     max_speed = std::to_string(motorinfo_.max_speed);
     min_speed = std::to_string(motorinfo_.min_speed);
     desired_speed = std::to_string(motorinfo_.desired_speed);
}

std::string
RPLidarDevice::get_device_status()
{
    return status_msg_;
}

void RPLidarDevice::setMotorSpeed(int speed)
{
    if(motor_ctrl_support_ != 0 && channelTypeSerial_)
    {
        lidar_drv_->setMotorSpeed(speed);
        status_msg_ = "Changing motor speed";
        lidar_drv_->getMotorInfo(motorinfo_);
        update_status();
    }
    
}

bool  RPLidarDevice::check_device_health(int * errorCode)
{
    status_msg_ = "Checking device health";
    op_result_ = lidar_drv_->getHealth(healthinfo_);
    if (SL_IS_OK(op_result_)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        printf("SLAMTEC Lidar health status : %d\n", healthinfo_.status);
        if (healthinfo_.status == SL_LIDAR_STATUS_ERROR) {
            printf("Error, slamtec lidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want slamtec lidar to be reboot by software
            // drv->reset();
            return false;
        } else {
            return true;
        }

    } else {
        printf("Error, cannot retrieve the lidar health code: %x\n", op_result_);
        return false;
    }
}

void RPLidarDevice::get_scan_modes()
{
    status_msg_ = "Getting scan modes";
    lidar_drv_->getAllSupportedScanModes(scanModes);
    scanModesStr = "";
    for (int i = 0; i < scanModes.size(); i++)
    {
        scanModesStr += scanModes[i].scan_mode;
        scanModesStr += " | ";
    }

    lidar_drv_->checkMotorCtrlSupport(motor_ctrl_support_);
    switch (motor_ctrl_support_)
    {
        case MotorCtrlSupportNone:
            motor_control = "None";
            break;
        case MotorCtrlSupportPwm:
            motor_control = "PWM";
            break;
        case MotorCtrlSupportRpm:
            motor_control = "RPM";
            break;
    }
    
}

void RPLidarDevice::scan(float min_dist, float max_dist)
{
    if(is_busy_) return;
    
    sl_lidar_response_measurement_node_hq_t nodes[8192];
    size_t   count = _countof(nodes);
    
    op_result_ = lidar_drv_->getScanDataWithIntervalHq(nodes, count);
    data_count_ = count;

    if (SL_IS_OK(op_result_)) {
        for (int pos = 0; pos < (int)count ; ++pos) {

            bool write = true;

            if(qualityCheck_)
            {
                if(nodes[pos].flag < 2 || nodes[pos].quality < 150)
                {
                    // skip not updated
                    write = false;
                }
            }

            double tempAngle = nodes[pos].angle_z_q14 * 90.f / 16384.f;
            if (tempAngle > 360.0f || tempAngle < 0.0f)
            {
                // skip bad angles
                write = false;
            }
            
            int halfAngle = floor(tempAngle * precision_);
            float distance = nodes[pos].dist_mm_q2 / 4.0f;
            if (distance > max_dist || distance < min_dist)
            {
                // skip bad distances
                data_[halfAngle].distance = 0;
                write = false;
            }

            if(write)
            {
                data_[halfAngle].distance = distance;
                data_[halfAngle].angle = halfAngle;
                data_[halfAngle].quality = nodes[pos].quality;
                data_[halfAngle].flag = nodes[pos].flag;
            }
            
        }
    }
}

void RPLidarDevice::init_data()
{
    for(int i =0; i < 720*2; i++)
    {
        data_[i].distance = 0;
        data_[i].angle = i / 2;
        data_[i].quality = 0;
        data_[i].flag = 0;
    }
    scanModesStr = "";
}


// TODO:
// 1. Add a set motor speed if it available for current lidar
// 2. Auto search for baudrate
// 3. Refactor all code
// 4. Move data and connection into separate thread to not lock TD while it trying to connect or any problem happens