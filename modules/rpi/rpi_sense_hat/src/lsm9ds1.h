//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/rpi/sense_hat/i2c_bus.h"

namespace grape::rpi::sense_hat::lsm9ds1 {

//=================================================================================================
/// LSM9DS1 register map and constants (from ST DocID025715)

/// I2C addresses
static constexpr DevAddr AG_ADDR = DevAddr{ 0x6A };   //!< Accel + Gyro
static constexpr DevAddr MAG_ADDR = DevAddr{ 0x1C };  //!< Magnetometer

/// WHO_AM_I expected values
static constexpr std::uint8_t AG_WHO_AM_I = 0x68;
static constexpr std::uint8_t MAG_WHO_AM_I = 0x3D;

/// Accel/Gyro registers
namespace ag {
static constexpr RegAddr WHO_AM_I = RegAddr{ 0x0F };
static constexpr RegAddr CTRL_REG1_G = RegAddr{ 0x10 };   //!< Gyro ODR, full-scale, bandwidth
static constexpr RegAddr CTRL_REG6_XL = RegAddr{ 0x20 };  //!< Accel ODR, full-scale, bandwidth
static constexpr RegAddr CTRL_REG8 = RegAddr{ 0x22 };     //!< Control (SW reset, IF_ADD_INC, etc)
static constexpr RegAddr STATUS_REG = RegAddr{ 0x17 };
static constexpr RegAddr OUT_X_L_G = RegAddr{ 0x18 };   //!< Gyro data start (6 bytes)
static constexpr RegAddr OUT_X_L_XL = RegAddr{ 0x28 };  //!< Accel data start (6 bytes)

/// ODR register values (bits [7:5] of CTRL_REG1_G and CTRL_REG6_XL)
static constexpr std::uint8_t ODR_POWER_DOWN = 0x00;
static constexpr std::uint8_t ODR_476HZ = 0xA0;  //!< 101 << 5
static constexpr std::uint8_t ODR_952HZ = 0xC0;  //!< 110 << 5

static constexpr std::uint8_t IF_ADD_INC = 0x04;  //!< Auto-increment for burst reads
}  // namespace ag

/// Magnetometer registers
namespace mag {
static constexpr RegAddr WHO_AM_I = RegAddr{ 0x0F };
static constexpr RegAddr CTRL_REG1_M = RegAddr{ 0x20 };  //!< Temp comp, XY perf, ODR
static constexpr RegAddr CTRL_REG2_M = RegAddr{ 0x21 };  //!< Full-scale
static constexpr RegAddr CTRL_REG3_M = RegAddr{ 0x22 };  //!< Operating mode
static constexpr RegAddr CTRL_REG4_M = RegAddr{ 0x23 };  //!< Z-axis performance
static constexpr RegAddr STATUS_REG_M = RegAddr{ 0x27 };
static constexpr RegAddr OUT_X_L_M = RegAddr{ 0x28 };  //!< Mag data start (6 bytes)

/// CTRL_REG1_M: TEMP_COMP=1, XY ultra-high perf (11), ODR=80Hz (111), FAST_ODR=0
static constexpr std::uint8_t CFG_80HZ_ULTRA = 0xFC;
/// CTRL_REG3_M: Continuous conversion (bits 1:0 = 00)
static constexpr std::uint8_t MODE_CONTINUOUS = 0x00;
/// CTRL_REG3_M: Power-down (bits 1:0 = 11)
static constexpr std::uint8_t MODE_POWER_DOWN = 0x03;
/// CTRL_REG4_M: Z ultra-high performance (bits 3:2 = 11)
static constexpr std::uint8_t Z_ULTRA_HIGH = 0x0C;
}  // namespace mag

/// Auto-increment bit for burst reads
static constexpr std::uint8_t AUTO_INC = 0x80;

}  // namespace grape::rpi::sense_hat::lsm9ds1
