//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <expected>
#include <filesystem>
#include <memory>
#include <vector>

#include "grape/exception.h"

struct lua_State;  //!< Internal detail. Don't worry about it!

namespace grape::script {

//=================================================================================================
/// Exception triggered by errors in script execution
class ScriptException : public grape::Exception {
public:
  ScriptException(const std::string& msg, std::source_location loc) : Exception(msg, loc) {
  }
};

class ConfigTable;  //!< Forward declaration. Interface description is further down

//=================================================================================================
/// Concept defines data types that can be parsed from a configuration script
template <typename T>
concept Parsable = std::is_same_v<T, bool> ||         //
                   std::is_same_v<T, int> ||          //
                   std::is_same_v<T, float> ||        //
                   std::is_same_v<T, std::string> ||  //
                   std::is_same_v<T, ConfigTable>;

/// Concept defines data types that can be configured from a ConfigTable defined here
template <typename T>
concept Configurable = requires(T obj, const ConfigTable& table) {
  { obj.configure(table) } -> std::same_as<void>;
};

//=================================================================================================
/// Opens a configuration script, parses it and provides access to the root configuration table.
/// Use ConfigTable to access values in the table.
class ConfigScript {
public:
  explicit ConfigScript(const std::string& script_string);
  explicit ConfigScript(const std::filesystem::path& script_path);

  /// @return top level configuration table
  [[nodiscard]] auto table() const -> ConfigTable;

private:
  ConfigScript();
  static void exitLua(lua_State* state);
  std::shared_ptr<lua_State> lua_state_;
};

//=================================================================================================
/// A configuration table is a collection of key-value pairs or an array of values. See example
/// programs for usage.
class ConfigTable {
public:
  enum class Error : std::uint8_t {
    NotFound,   //!< Key or index not found
    Unparsable  //!< Value could not be converted into user requested data type
  };

  /// @brief Reads a key-value pair from the table
  /// @tparam T The value data type
  /// @param key The key to read
  /// @return Value of the specified key if found, error code otherwise.
  template <Parsable T>
  [[nodiscard]] auto read(const std::string& key) const -> std::expected<T, Error>;

  /// If this table holds an array of values instead of key-value pairs, read the value at
  /// a specific index.
  /// @tparam T The value data type
  /// @param index The location within the array to read from. Indices start at zero.
  /// @return Value at the specified index, if found; error code otherwise.
  template <Parsable T>
  [[nodiscard]] auto read(size_t index) const -> std::expected<T, Error>;

  /// Returns the number of elements in the table if the table is an array, else 0
  [[nodiscard]] auto size() const -> size_t;

  ~ConfigTable();
  ConfigTable(ConfigTable&& other) noexcept;

  auto operator=(ConfigTable&& other) noexcept -> ConfigTable& = delete;
  ConfigTable(const ConfigTable&) = delete;
  auto operator=(const ConfigTable&) -> ConfigTable& = delete;

private:
  friend ConfigScript;

  /// Private constructor. Objects typically created by ConfigScript or objects of this class.
  /// @param lua_state Lua interpreter state
  /// @param lua_table_ref Reference to the table in the registry
  explicit ConfigTable(std::shared_ptr<lua_State> lua_state, int lua_table_ref);

  [[nodiscard]] auto
  readBool(const std::string& key) const -> std::expected<bool, ConfigTable::Error>;

  [[nodiscard]] auto
  readInt(const std::string& key) const -> std::expected<int, ConfigTable::Error>;

  [[nodiscard]] auto
  readFloat(const std::string& key) const -> std::expected<float, ConfigTable::Error>;

  [[nodiscard]] auto
  readString(const std::string& key) const -> std::expected<std::string, ConfigTable::Error>;

  [[nodiscard]] auto
  readTable(const std::string& key) const -> std::expected<ConfigTable, ConfigTable::Error>;

  [[nodiscard]] auto readBool(size_t index) const -> std::expected<bool, ConfigTable::Error>;

  [[nodiscard]] auto readInt(size_t index) const -> std::expected<int, ConfigTable::Error>;

  [[nodiscard]] auto readFloat(size_t index) const -> std::expected<float, ConfigTable::Error>;

  [[nodiscard]] auto
  readString(size_t index) const -> std::expected<std::string, ConfigTable::Error>;

  [[nodiscard]] auto
  readTable(size_t index) const -> std::expected<ConfigTable, ConfigTable::Error>;

  std::shared_ptr<lua_State> lua_state_;
  int lua_table_ref_;
  size_t size_{};
};

//-------------------------------------------------------------------------------------------------
inline constexpr auto toString(ConfigTable::Error e) -> std::string_view {
  switch (e) {
    case ConfigTable::Error::NotFound:
      return "NotFound";
    case ConfigTable::Error::Unparsable:
      return "Unparsable";
  };
}

//-------------------------------------------------------------------------------------------------
template <Parsable T>
inline auto
ConfigTable::read(const std::string& key) const -> std::expected<T, ConfigTable::Error> {
  if constexpr (std::is_same_v<T, bool>) {
    return readBool(key);
  } else if constexpr (std::is_same_v<T, int>) {
    return readInt(key);
  } else if constexpr (std::is_same_v<T, float>) {
    return readFloat(key);
  } else if constexpr (std::is_same_v<T, std::string>) {
    return readString(key);
  } else if constexpr (std::is_same_v<T, ConfigTable>) {
    return readTable(key);
  }
}

//-------------------------------------------------------------------------------------------------
template <Parsable T>
inline auto ConfigTable::read(size_t index) const -> std::expected<T, ConfigTable::Error> {
  if constexpr (std::is_same_v<T, bool>) {
    return readBool(index);
  } else if constexpr (std::is_same_v<T, int>) {
    return readInt(index);
  } else if constexpr (std::is_same_v<T, float>) {
    return readFloat(index);
  } else if constexpr (std::is_same_v<T, std::string>) {
    return readString(index);
  } else if constexpr (std::is_same_v<T, ConfigTable>) {
    return readTable(index);
  }
}

}  // namespace grape::script
