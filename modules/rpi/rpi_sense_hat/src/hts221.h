//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/rpi/sense_hat/i2c_bus.h"

namespace grape::rpi::sense_hat::hts221 {

//=================================================================================================
/// HTS221 register map and constants

/// I2C address (SDO/SA0 tied to Vdd on Sense HAT v2)
static constexpr DevAddr ADDR = DevAddr{ 0x5F };

/// WHO_AM_I expected value
static constexpr std::uint8_t WHO_AM_I_VAL = 0xBC;

/// Registers
static constexpr RegAddr WHO_AM_I = RegAddr{ 0x0F };
static constexpr RegAddr CTRL_REG1 = RegAddr{ 0x20 };   //!< PD, BDU, ODR
static constexpr RegAddr STATUS_REG = RegAddr{ 0x27 };  //!< H_DA, T_DA
static constexpr RegAddr HUM_OUT_L = RegAddr{ 0x28 };   //!< Humidity LSB (burst start)
static constexpr RegAddr TEMP_OUT_L = RegAddr{ 0x2A };  //!< Temperature LSB (burst start)

/// Calibration register block: burst-read 16 bytes from CALIB_START to get all coefficients
/// Layout: [0]=H0_rH_x2 [1]=H1_rH_x2 [2]=T0_degC_x8 [3]=T1_degC_x8 [4]=rsvd
///         [5]=T1_T0_MSBs [6..7]=H0_T0_OUT [8..9]=rsvd [10..11]=H1_T0_OUT
///         [12..13]=T0_OUT [14..15]=T1_OUT
static constexpr RegAddr CALIB_START = RegAddr{ 0x30 };
static constexpr std::size_t CALIB_LEN = 16U;

/// CTRL_REG1 bits: PD=1 (power on), BDU=1, ODR=11 (12.5 Hz continuous)
static constexpr std::uint8_t CFG_12HZ = 0x87;
/// CTRL_REG1 bits: PD=0 (power down)
static constexpr std::uint8_t POWER_DOWN = 0x00;

/// STATUS_REG bits
static constexpr std::uint8_t H_DA = 0x02;  //!< Humidity data available
static constexpr std::uint8_t T_DA = 0x01;  //!< Temperature data available

/// Auto-increment bit for burst reads (STMicro convention)
static constexpr std::uint8_t MULTI_BYTE = 0x80;

}  // namespace grape::rpi::sense_hat::hts221
