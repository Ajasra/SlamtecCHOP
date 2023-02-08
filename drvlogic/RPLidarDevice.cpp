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
    
    lidar_drv_ = nullptr;
    channel_ = nullptr;
    precision_ = 2.0f;
    qualityCheck_ = false;

    init_data();
}

RPLidarDevice::~RPLidarDevice()
{
    status_msg_ = "RPLidar destructor called";
    on_disconnect();
    delete lidar_drv_;
    lidar_drv_ = nullptr;
}

void
RPLidarDevice::setLidar(bool serial, const char* address_1, int address_2, float precision, bool qualityCheck, bool standart, bool udp)
{
    if (is_busy_)
    {
        // check if it in connection now, then just do nothing
        status_msg_ = "RPLidar is busy";
        return;
    }
    
    init_data();

    _address_1 = address_1;
    _address_2 = address_2;
    precision_ = precision;
    qualityCheck_ = qualityCheck;
    _standart = standart;
    _udp = udp;
    _channelTypeSerial = serial;
}

bool
RPLidarDevice::thr_connect(bool& serial, std::string& address_1, int& address_2, bool& standart, bool &udp)
{

    if(serial)
        channel_ = (*createSerialPortChannel(address_1, baudRateLists[address_2]));
    else
    {
        if(udp)
            channel_ = *createUdpChannel(address_1, address_2);
        else
            channel_ = *createTcpChannel(address_1, address_2);
    }

    if (!lidar_drv_)
        lidar_drv_ = *createLidarDriver();

    if (!(bool)lidar_drv_)
    {
        is_busy_ = false;
        return SL_RESULT_OPERATION_FAIL == 1;
    }
    
    sl_result ans =(lidar_drv_)->connect(channel_);

    if (SL_IS_FAIL(ans)) {
        status_msg_ = "Error, cannot bind to the specified address: " + _address_1;
        is_busy_ = false;
        return false;
    }
    
    ans = lidar_drv_->getDeviceInfo(devinfo_);
    if (SL_IS_FAIL(ans)) {
        status_msg_ = "Failed to get device info. code: " + std::to_string(ans);
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
    
    get_scan_modes();

    if(serial)
        lidar_drv_->setMotorSpeed();
    
    if(standart)
        lidar_drv_->startScanExpress(0,0,0,&currentScanMode);
    else
        lidar_drv_->startScan(0,1, 0, &currentScanMode);

    is_connected_ = true;
    status_msg_ = "Connected to RPLidar on " + _address_1;
    is_busy_ = false;
    return false;
}

bool
RPLidarDevice::on_connect()
{
    if (is_connected_ || is_busy_) return true;
    if(_channelTypeSerial)
    {
        status_msg_ = "Connecting to RPLidar on PORT: " + _address_1 + " BAUDRATE: " + std::to_string(_address_2);
    }else
    {
        status_msg_ = "Connecting to RPLidar TCP on IP: " + std::string(_address_1) + " PORT: " + std::to_string(_address_2);
    }
    

    is_busy_ = true;
    _lidarThread = std::thread([this] {this->thr_connect(_channelTypeSerial, _address_1, _address_2, _standart, _udp);});
    _lidarThread.detach();
    
    return true;
}

void
RPLidarDevice::on_disconnect()
{
    status_msg_ = "Disconnecting from RPLidar";
    
    if (is_connected_) {
        lidar_drv_->stop();
        if(_channelTypeSerial) lidar_drv_->setMotorSpeed(0);
    }
    is_connected_ = false;
    delete lidar_drv_;
    lidar_drv_ = nullptr;
    delete channel_;
    channel_ = nullptr;
    init_data();
}

void
RPLidarDevice::update_status()
{
     printf("SLAMTEC LIDAR S/N: ");
     serial_number = "";
     for (unsigned char pos : devinfo_.serialnum)
     {
         printf("%02X", pos);
         serial_number += std::to_string(pos);
     }
    
     firmware_version = std::to_string(devinfo_.firmware_version>>8) + ".";
     const int minor = devinfo_.firmware_version & 0xFF;
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
    if(motor_ctrl_support_ != 0 && _channelTypeSerial)
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
    if (SL_IS_OK(op_result_)) { // the macro IS_OK is the preferred way to judge whether the operation is succeed.
        printf("SLAMTEC Lidar health status : %d\n", healthinfo_.status);
        if (healthinfo_.status == SL_LIDAR_STATUS_ERROR) {
            printf("Error, slamtec lidar internal error detected. Please reboot the device to retry.\n");
            lidar_drv_->reset();
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
    // status_msg_ = "Getting scan modes";
    lidar_drv_->getAllSupportedScanModes(scanModes);
    scanModesStr = "";
    for (auto& scanMode : scanModes)
    {
        scanModesStr += scanMode.scan_mode;
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
    if(is_busy_ || !is_connected_) return;
    
    sl_lidar_response_measurement_node_hq_t nodes[8192];
    size_t   count = _countof(nodes);
    
    op_result_ = lidar_drv_->getScanDataWithIntervalHq(nodes, count);
    data_count_ = count;

    if (SL_IS_OK(op_result_)) {
        for (int pos = 0; pos < static_cast<int>(count) ; ++pos) {

            bool write = true;
            if(qualityCheck_)
            {
                if(nodes[pos].flag < 2 || nodes[pos].quality < 150)
                {
                    write = false;
                }
            }

            const double tempAngle = nodes[pos].angle_z_q14 * 90.f / 16384.f;
            if (tempAngle > 360 || tempAngle < 0)
            {
                write = false;
            }

            const int halfAngle = floor(tempAngle * precision_);
            const float distance = nodes[pos].dist_mm_q2 / 4.0f;
            if (distance > max_dist || distance < min_dist)
            {
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

        // generate random number form 1 to 100
        _rnd_number = rand() % 100 + 1;
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

    _address_1 = "";
    _address_2 = 0;
    precision_ = 1;
    qualityCheck_ = false;
    _standart = false;
    _udp = false;
    _channelTypeSerial = true;
    _rnd_number = 0;
}


// TODO:
// 1. Add a set motor speed if it available for current lidar
// 2. Auto search for baudrate
// 3. Refactor all code
// 4. Move data and connection into separate thread to not lock TD while it trying to connect or any problem happens