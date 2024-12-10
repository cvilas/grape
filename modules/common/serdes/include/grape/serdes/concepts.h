//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <cstddef>

namespace grape::serdes {

//-------------------------------------------------------------------------------------------------
// Concept of a generic writable buffer stream
template <typename Stream>
concept WritableStream = requires(Stream strm, const char* data, std::size_t size) {
  { strm.write(data, size) } -> std::same_as<bool>;
  /* Writes 'size' bytes from 'data' into stream 's'. Return false on failure  */
};

//-------------------------------------------------------------------------------------------------
// Concept of a generic readable buffer stream
template <typename Stream>
concept ReadableStream = requires(Stream strm, char* const data, std::size_t size) {
  { strm.read(data, size) } -> std::same_as<bool>;
  /* Reads 'size' bytes into 'data' from stream 's'. Return false on failure  */
};

}  // namespace grape::serdes
