//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once
#define ZENOHCXX_ZENOHC  // Use the C backend for Zenoh

#include <unordered_map>
#include <zenoh.hxx>

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
inline auto toString(const zenoh::ZResult& code) -> std::string_view {
  static const auto zenoh_error_strings = std::unordered_map<int, std::string_view>{
    { Z_OK, "OK" },                                       //
    { Z_EINVAL, "Invalid data or argument" },             //
    { Z_EPARSE, "Parsing error" },                        //
    { Z_EIO, "I/O error" },                               //
    { Z_ENETWORK, "Network/connection failure" },         //
    { Z_ENULL, "Null value" },                            //
    { Z_EUNAVAILABLE, "Unavailable" },                    //
    { Z_EDESERIALIZE, "Deserialisation error" },          //
    { Z_ESESSION_CLOSED, "Session closed" },              //
    { Z_EUTF8, "UTF8 error" },                            //
    { Z_EBUSY_MUTEX, "Mutex busy" },                      //
    { Z_EINVAL_MUTEX, "Mutex invalid" },                  //
    { Z_EAGAIN_MUTEX, "Mutex unavailable (try again)" },  //
    { Z_EPOISON_MUTEX, "Mutex poisoned" }                 //
  };
  return zenoh_error_strings.at(code);
}
}  // namespace grape::ipc
