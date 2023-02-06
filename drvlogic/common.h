#pragma once

#include "sl_lidar.h" 
#include "sl_lidar_driver.h"
#include <thread>
#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

using namespace sl;

typedef struct lidar_data
{
    int         angle;
    float       distance;
    int         quality;
    int         flag;
} __attribute__((packed)) lidar_data;