#include "RPLidarDevice.h"
#include <thread>
#include <chrono>
#include <Windows.h>

// Debug timestamp helper for driver layer
static long long getDriverTimestampMs() {
	static auto start = std::chrono::steady_clock::now();
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

static void driver_debug(const char* message) {
	char buffer[512];
	snprintf(buffer, sizeof(buffer), "[%lldms] RPLidarDevice :: %s\n", getDriverTimestampMs(), message);
	printf("%s", buffer);
	OutputDebugStringA(buffer);
}

static const int baudRateLists[] = {
    115200,
    256000,
    460800,
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
    driver_debug("RPLidarDevice destructor called");
    on_disconnect();  // This now properly joins the thread
    // lidar_drv_ is already deleted in on_disconnect()
    driver_debug("RPLidarDevice destructor complete");
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
    driver_debug("thr_connect THREAD STARTED");

    driver_debug("Creating channel...");
    if(serial)
        channel_ = (*createSerialPortChannel(address_1, baudRateLists[address_2]));
    else
    {
        if(udp)
            channel_ = *createUdpChannel(address_1, address_2);
        else
            channel_ = *createTcpChannel(address_1, address_2);
    }
    driver_debug("Channel created");

    if (!lidar_drv_)
        lidar_drv_ = *createLidarDriver();

    if (!(bool)lidar_drv_)
    {
        driver_debug("ERROR: Failed to create lidar driver");
        is_busy_ = false;
        return SL_RESULT_OPERATION_FAIL == 1;
    }

    driver_debug("Calling lidar_drv_->connect()...");
    sl_result ans =(lidar_drv_)->connect(channel_);
    driver_debug("connect() returned");

    if (SL_IS_FAIL(ans)) {
        driver_debug("ERROR: connect() failed");
        status_msg_ = "Error, cannot bind to the specified address: " + _address_1;
        is_busy_ = false;
        return false;
    }

    driver_debug("Getting device info...");
    ans = lidar_drv_->getDeviceInfo(devinfo_);
    driver_debug(("getDeviceInfo returned: " + std::to_string(ans)).c_str());
    if (SL_IS_FAIL(ans)) {
        driver_debug("ERROR: getDeviceInfo() failed");
        status_msg_ = "Failed to get device info. code: " + std::to_string(ans);
        is_busy_ = false;
        return false;
    }
    // Skip getMotorInfo for now - it hangs on some models (S2)
    // TODO: Add timeout or make this optional
    // driver_debug("Getting motor info...");
    // ans = lidar_drv_->getMotorInfo(motorinfo_);
    // driver_debug(("getMotorInfo returned: " + std::to_string(ans)).c_str());

    update_status();

    driver_debug("Checking device health...");
    if(!check_device_health())
    {
        driver_debug("ERROR: Device health check failed");
        is_busy_ = false;
        return false;
    }
    driver_debug("Device health OK");

    driver_debug("Getting scan modes...");
    get_scan_modes();
    driver_debug("Scan modes retrieved");

    driver_debug("Starting motor/scan...");

    // Check model to determine motor/scan approach
    // Models 24+ (S1/S2/S3) have internal motor control
    // Models < 24 (A1/A2/A3) need setMotorSpeed()
    bool is_s_series = (devinfo_.model >= 24);
    driver_debug(("Model ID: " + std::to_string(devinfo_.model) + " (S-series: " + (is_s_series ? "yes" : "no") + ")").c_str());

    if (serial && !is_s_series) {
        // A1/A2/A3 need motor speed set
        driver_debug("Calling setMotorSpeed() for A-series...");
        lidar_drv_->setMotorSpeed();
        driver_debug("setMotorSpeed() returned");
    }

    // Try startScanExpress first (works for most models)
    driver_debug("Calling startScanExpress()...");
    sl_result scan_result = lidar_drv_->startScanExpress(false, 0, 0, &currentScanMode);
    driver_debug(("startScanExpress returned: " + std::to_string(scan_result)).c_str());

    // If express scan fails on older models, fall back to legacy scan
    if (SL_IS_FAIL(scan_result) && !is_s_series) {
        driver_debug("Express scan failed, trying legacy startScan()...");
        scan_result = lidar_drv_->startScan(false, true, 0, &currentScanMode);
        driver_debug(("startScan returned: " + std::to_string(scan_result)).c_str());
    }

    driver_debug("=== CONNECTION COMPLETE, is_connected_ = true ===");
    is_connected_ = true;
    status_msg_ = "Connected to RPLidar on " + _address_1;
    is_busy_ = false;
    return false;
}

bool
RPLidarDevice::on_connect()
{
    driver_debug("on_connect() called");
    if (is_connected_ || is_busy_) {
        driver_debug("on_connect() early return - already connected or busy");
        return true;
    }
    if(_channelTypeSerial)
    {
        status_msg_ = "Connecting to RPLidar on PORT: " + _address_1 + " BAUDRATE: " + std::to_string(_address_2);
    }else
    {
        status_msg_ = "Connecting to RPLidar TCP on IP: " + std::string(_address_1) + " PORT: " + std::to_string(_address_2);
    }

    driver_debug("Spawning connection thread...");
    is_busy_ = true;
    _stop_requested = false;

    // Join any previous thread first
    if (_lidarThread.joinable()) {
        _lidarThread.join();
    }

    _lidarThread = std::thread([this] {this->thr_connect(_channelTypeSerial, _address_1, _address_2, _standart, _udp);});
    driver_debug("Thread started (joinable), on_connect() returning");

    return true;
}

void
RPLidarDevice::on_disconnect()
{
    driver_debug("on_disconnect() called");
    status_msg_ = "Disconnecting from RPLidar";

    // Signal thread to stop and wait for it (with timeout)
    _stop_requested = true;
    if (_lidarThread.joinable()) {
        driver_debug("Waiting for connection thread to finish (max 2 sec)...");
        // Use a timed wait approach - detach if thread doesn't finish in time
        auto start = std::chrono::steady_clock::now();
        while (is_busy_) {
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed > std::chrono::seconds(2)) {
                driver_debug("Thread timeout - detaching");
                _lidarThread.detach();
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (_lidarThread.joinable()) {
            _lidarThread.join();
            driver_debug("Connection thread finished");
        }
    }

    if (is_connected_ && lidar_drv_) {
        driver_debug("Stopping lidar...");
        lidar_drv_->stop();
        // Skip setMotorSpeed(0) - it can hang on S2
    }
    is_connected_ = false;
    is_busy_ = false;
    driver_debug("Deleting lidar_drv_ and channel_...");
    if (lidar_drv_) {
        delete lidar_drv_;
        lidar_drv_ = nullptr;
    }
    if (channel_) {
        delete channel_;
        channel_ = nullptr;
    }
    init_data();
    driver_debug("on_disconnect() complete");
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

    // Try grabScanDataHq instead - it waits for valid data
    op_result_ = lidar_drv_->grabScanDataHq(nodes, count, 0);  // 0 = don't wait/timeout
    if (SL_IS_FAIL(op_result_)) {
        // Fall back to interval method if grab fails
        count = _countof(nodes);
        op_result_ = lidar_drv_->getScanDataWithIntervalHq(nodes, count);
    }
    data_count_ = count;

    static int debug_counter = 0;
    debug_counter++;

    if (SL_IS_OK(op_result_)) {
        int write_count = 0;
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
                write_count++;
                data_[halfAngle].distance = distance;
                data_[halfAngle].angle = halfAngle;
                data_[halfAngle].quality = nodes[pos].quality;
                data_[halfAngle].flag = nodes[pos].flag;
            }

        }

        // Log debug info every 60 frames (~1 second)
        if (debug_counter % 60 == 0) {
            char buf[256];
            snprintf(buf, sizeof(buf), "scan: result=0x%x, count=%d, write_count=%d, min=%.0f, max=%.0f",
                op_result_, (int)count, write_count, min_dist, max_dist);
            driver_debug(buf);
            // Log first non-zero sample
            if (count > 0) {
                snprintf(buf, sizeof(buf), "sample[0]: angle_q14=%d, dist_q2=%d, quality=%d, flag=%d",
                    nodes[0].angle_z_q14, nodes[0].dist_mm_q2, nodes[0].quality, nodes[0].flag);
                driver_debug(buf);
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
    is_busy_ = false;  // Reset busy flag to allow new connections
}


// TODO:
// 1. Add a set motor speed if it available for current lidar
// 2. Auto search for baudrate
// 3. Refactor all code
// 4. Move data and connection into separate thread to not lock TD while it trying to connect or any problem happens