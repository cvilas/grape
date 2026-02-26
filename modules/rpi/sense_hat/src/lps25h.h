//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/rpi/sense_hat/i2c_bus.h"

namespace grape::rpi::sense_hat::lps25h {

//=================================================================================================
/// LPS25H register map and constants (from ST DocID025590)

/// I2C address (SDO/SA0 tied to Vdd on Sense HAT v2)
static constexpr DevAddr ADDR = DevAddr{ 0x5C };

/// WHO_AM_I expected value
static constexpr std::uint8_t WHO_AM_I_VAL = 0xBD;

/// Registers
static constexpr RegAddr WHO_AM_I = RegAddr{ 0x0F };
static constexpr RegAddr CTRL_REG1 = RegAddr{ 0x20 };     //!< PD, ODR
static constexpr RegAddr CTRL_REG2 = RegAddr{ 0x21 };     //!< ONE_SHOT, BOOT, FIFO_EN
static constexpr RegAddr STATUS_REG = RegAddr{ 0x27 };    //!< T_DA, P_DA
static constexpr RegAddr PRESS_OUT_XL = RegAddr{ 0x28 };  //!< Pressure LSB (burst start)
static constexpr RegAddr TEMP_OUT_L = RegAddr{ 0x2B };    //!< Temperature LSB (burst start)

/// CTRL_REG1 bits: PD=1 (power on), ODR=100 (25 Hz continuous)
static constexpr std::uint8_t CFG_25HZ = 0xB0;
/// CTRL_REG1 bits: PD=0 (power down)
static constexpr std::uint8_t POWER_DOWN = 0x00;
/// CTRL_REG2: ONE_SHOT trigger
static constexpr std::uint8_t ONE_SHOT = 0x01;

/// STATUS_REG bits
static constexpr std::uint8_t P_DA = 0x02;  //!< Pressure data available
static constexpr std::uint8_t T_DA = 0x01;  //!< Temperature data available

/// Auto-increment bit for burst reads (STMicro convention)
static constexpr std::uint8_t MULTI_BYTE = 0x80;

/// Conversion constants (from datasheet section 3.4)
static constexpr float PRESSURE_SCALE = 1.F / 4096.F;  //!< LSB to hPa
static constexpr float TEMP_OFFSET = 42.5F;            //!< Degrees C
static constexpr float TEMP_SCALE = 1.F / 480.F;       //!< LSB to degrees C

}  // namespace grape::rpi::sense_hat::lps25h
