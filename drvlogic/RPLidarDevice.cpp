#include "RPLidarDevice.h"

static const int baudRateLists[] = {
    115200,
    256000,
    1000000
};

RPLidarDevice::RPLidarDevice()
{
    
    status_msg_ = "RPLidar instance created";

    is_connected_ = false;
    serial_number = "";
    hardware_version = "";
    firmware_version = "";
    is_connected_ = false;
    
    lidar_drv_ = NULL;
    channel_ = NULL;

    init_data();
    
}

RPLidarDevice::~RPLidarDevice()
{
    status_msg_ = "RPLidar destructor called";
    on_disconnect();
    delete lidar_drv_;
    lidar_drv_ = NULL;
}

bool
RPLidarDevice::on_connect(const char* port, int baudrate)
{
    if (is_connected_) return true;
    status_msg_ = "Connecting to RPLidar";

    channel_ = (*createSerialPortChannel(port, baudRateLists[baudrate]));

    if (!lidar_drv_)
        lidar_drv_ = *createLidarDriver();

    if (!(bool)lidar_drv_) return SL_RESULT_OPERATION_FAIL;

    sl_result ans =(lidar_drv_)->connect(channel_);

    if (SL_IS_FAIL(ans)) {
        printf("Error, cannot bind to the specified serial port %s.\n"
            , port);
        return false;
    }
    
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

    is_connected_ = true;
    status_msg_ = "Connected to RPLidar";

    lidar_drv_->setMotorSpeed();
    lidar_drv_->startScan(0,1);
    return true;
}

void
RPLidarDevice::on_disconnect()
{
    status_msg_ = "Disconnecting from RPLidar";
    if (is_connected_) {
        lidar_drv_->stop();
        lidar_drv_->setMotorSpeed(0);
    }
    is_connected_ = false;
    delete lidar_drv_;
    lidar_drv_ = NULL;
    delete channel_;
    channel_ = NULL;
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

std::string RPLidarDevice::get_device_status()
{
    return status_msg_;
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

void RPLidarDevice::scan(float min_dist, float max_dist)
{
    sl_lidar_response_measurement_node_hq_t nodes[8192];
    size_t   count = _countof(nodes);
    
    op_result_ = lidar_drv_->getScanDataWithIntervalHq(nodes, count);

    if (SL_IS_OK(op_result_)) {
        for (int pos = 0; pos < (int)count ; ++pos) {

            double tempAngle = nodes[pos].angle_z_q14 * 90.f / 16384.f;
            if (tempAngle > 360.0f || tempAngle < 0.0f)
            {
                // skip bad angles
                continue;
            }
            
            int halfAngle = floor(tempAngle * 2);
            float distance = nodes[pos].dist_mm_q2 / 4.0f;
            if (distance > max_dist || distance < min_dist)
            {
                // skip bad distances
                data_[halfAngle].distance = 0;
                continue;
            }
            data_[halfAngle].distance = distance;
            data_[halfAngle].angle = halfAngle;
            data_[halfAngle].quality = nodes[pos].quality;
            data_[halfAngle].flag = nodes[pos].flag;
        }
    }
}

void RPLidarDevice::init_data()
{
    for(int i =0; i < 720; i++)
    {
        data_[i].distance = 0;
        data_[i].angle = i / 2;
        data_[i].quality = 0;
        data_[i].flag = 0;
    }
}


// TODO:
// 1. Add a set motor speed if it available for current lidar
// virtual sl_result setMotorSpeed(sl_u16 speed = DEFAULT_MOTOR_SPEED) = 0;
// 2. AAdd reset for lidar
// irtual sl_result reset(sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;
// 3. Add a set scan mode
// virtual sl_result getAllSupportedScanModes(std::vector<LidarScanMode>& outModes, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;
// 4. Get current frequency of the lidar
// 5. add network mode and switch for it
// 6. check the quality of data and reducing bad data
// 7. refactor all code