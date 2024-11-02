//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/script/script.h"

#include "grape/exception.h"
#include "lua.hpp"

namespace grape::script {

//=================================================================================================
ConfigScript::ConfigScript(const std::string& script_string) : ConfigScript() {
  auto* const state = lua_state_.get();
  if (luaL_dostring(state, script_string.c_str()) != LUA_OK) {
    panic<Exception>(
        std::format("{}: {}", lua_tostring(state, -1), toString(ConfigScript::Error::Unloadable)));
  }
}

//-------------------------------------------------------------------------------------------------
ConfigScript::ConfigScript(const std::filesystem::path& script_path) : ConfigScript() {
  auto* const state = lua_state_.get();
  if (luaL_dofile(state, script_path.c_str()) != LUA_OK) {
    panic<Exception>(
        std::format("{}: {}", lua_tostring(state, -1), toString(ConfigScript::Error::Unloadable)));
  }
}

//-------------------------------------------------------------------------------------------------
ConfigScript::ConfigScript()
  : lua_state_(std::shared_ptr<lua_State>(luaL_newstate(), [](lua_State* s) { exitLua(s); })) {
  luaL_openlibs(lua_state_.get());
}

//-------------------------------------------------------------------------------------------------
void ConfigScript::exitLua(lua_State* state) {
  // std::println(stderr, "Cleaning up Lua state ({} items on stack).", lua_gettop(state));
  lua_close(state);
}

//-------------------------------------------------------------------------------------------------
auto ConfigScript::table() const -> ConfigTable {
  // Get reference to global table and return it
  auto* const state_ptr = lua_state_.get();
  lua_pushglobaltable(state_ptr);
  const int global_table_ref = luaL_ref(state_ptr, LUA_REGISTRYINDEX);
  return ConfigTable(lua_state_, global_table_ref);
}
}  // namespace grape::script

namespace {

//=================================================================================================
void clearLuaStack(lua_State* state) {
  auto sz = lua_gettop(state);
  while (sz > 0) {
    --sz;
    lua_pop(state, 1);
  }
}

// A helper. Contains details with which to construct a ConfigTable later
struct ConfigTableDetail {
  int table_reference;
};

using ConfigTable = grape::script::ConfigTable;

//-------------------------------------------------------------------------------------------------
// Splits the key into tokens and recursively reads all the way to the last token (leaf) in the
// key, putting the item at the token location on the stack. On success, the stack is prepared with
// the leaf item, and the name of the token is returned. The stack can then be read by
// readLeaf(). If unsuccessful, the error code is returned.
auto readToLeaf(lua_State* state, int table_ref,
                const std::string& key) -> std::expected<std::string, ConfigTable::Error> {
  // push our table into stack
  std::ignore = lua_rawgeti(state, LUA_REGISTRYINDEX, table_ref);
  // Break the key into tokens and recurse through sub-tables until the last token, then read it
  static constexpr auto DELIMITER = ".";
  size_t start_pos = 0;
  size_t end_pos = 0;
  std::string token;
  while (true) {
    // get next token in the key chain
    end_pos = key.find(DELIMITER, start_pos);
    token = key.substr(start_pos, end_pos - start_pos);
    // retrieve item named 'token' into stack. After this operation, the token is
    // removed from stack and replaced with the item itself (stack size=2)
    std::ignore = lua_pushstring(state, token.c_str());
    const auto object_type = lua_gettable(state, -2);
    if (object_type == LUA_TNIL) {
      clearLuaStack(state);
      return std::unexpected(ConfigTable::Error::NotFound);
    }

    // reached final leaf
    if (end_pos == std::string::npos) {
      // The item is at the top of lua stack. Let readLeaf..() handle it.
      break;
    }

    // handle the sub-table case.
    lua_remove(state, -2);  // delete previously retrieved item, leaving current item on stack.
    start_pos = end_pos + 1;
    if (object_type != LUA_TTABLE) {
      clearLuaStack(state);
      return std::unexpected(ConfigTable::Error::Unparsable);
    }  // not table
  }  // while
  return token;
}

//-------------------------------------------------------------------------------------------------
// Parses the item already in the stack as specified type. The item must be placed in stack by
// calling readToLeaf()
template <typename T>
auto readLeaf(lua_State* state) -> std::expected<T, ConfigTable::Error> {
  const auto object_type = lua_type(state, -1);
  // boolean
  if constexpr (std::is_same_v<T, bool>) {
    const auto result =
        ((object_type == LUA_TBOOLEAN) ?
             std::expected<T, ConfigTable::Error>{ static_cast<T>(lua_toboolean(state, -1)) } :
             std::unexpected(ConfigTable::Error::Unparsable));
    clearLuaStack(state);
    return result;
  }  // boolean

  // number (int/float)
  else if constexpr (std::is_same_v<T, int> or std::is_same_v<T, float>) {
    const auto result =
        ((object_type == LUA_TNUMBER) ?
             std::expected<T, ConfigTable::Error>{ static_cast<T>(lua_tonumber(state, -1)) } :
             std::unexpected(ConfigTable::Error::Unparsable));
    clearLuaStack(state);
    return result;
  }  // number

  // string
  else if constexpr (std::is_same_v<T, std::string>) {
    const auto result = ((object_type == LUA_TSTRING) ?
                             std::expected<T, ConfigTable::Error>{ lua_tostring(state, -1) } :
                             std::unexpected(ConfigTable::Error::Unparsable));
    clearLuaStack(state);
    return result;
  }  // string

  // table
  else if constexpr (std::is_same_v<T, ConfigTableDetail>) {
    const auto result = ((object_type == LUA_TTABLE) ?
                             std::expected<T, ConfigTable::Error>{ ConfigTableDetail{
                                 .table_reference = luaL_ref(state, LUA_REGISTRYINDEX) } } :
                             std::unexpected(ConfigTable::Error::Unparsable));
    clearLuaStack(state);
    return result;
  }  // table

  // unsupported type
  clearLuaStack(state);
  return std::unexpected(ConfigTable::Error::Unparsable);
}

//-------------------------------------------------------------------------------------------------
template <typename T>
auto readIndex(lua_State* state, const int table_ref,
               const size_t index) -> std::expected<T, ConfigTable::Error> {
  // read table into stack and check its a table, not a value-type
  const auto object_type = lua_rawgeti(state, LUA_REGISTRYINDEX, table_ref);
  if (object_type != LUA_TTABLE) {
    clearLuaStack(state);
    return std::unexpected(ConfigTable::Error::Unparsable);
  }

  // confirm index is valid
  const auto size = lua_rawlen(state, -1);
  const auto lua_index = index + 1;
  if (lua_index > size) {
    clearLuaStack(state);
    return std::unexpected(ConfigTable::Error::NotFound);
  }

  // read value at index into stack
  std::ignore = lua_rawgeti(state, -1, static_cast<lua_Integer>(lua_index));

  // parse it
  return readLeaf<T>(state);
}

//-------------------------------------------------------------------------------------------------
template <typename T>
auto readKey(lua_State* state, int table_ref,
             const std::string& key) -> std::expected<T, ConfigTable::Error> {
  const auto read_leaf = [state](const auto& /*token*/) -> std::expected<T, ConfigTable::Error> {
    return readLeaf<T>(state);
  };
  const auto return_error = [](const auto& error) -> std::expected<T, ConfigTable::Error> {
    return std::unexpected(error);
  };
  return readToLeaf(state, table_ref, key)  //
      .and_then(read_leaf)                  //
      .or_else(return_error);
}

}  // namespace

namespace grape::script {

//=================================================================================================
ConfigTable::ConfigTable(std::shared_ptr<lua_State> lua_state, int lua_table_ref)
  : lua_state_(std::move(lua_state)), lua_table_ref_(lua_table_ref) {
  // read the table into stack just to read the size
  auto* state = lua_state_.get();
  const auto object_type = lua_rawgeti(state, LUA_REGISTRYINDEX, lua_table_ref_);
  if (object_type == LUA_TTABLE) {
    size_ = lua_rawlen(state, -1);
    // std::println(stderr, "Table size='{}'", size_);
  }
  clearLuaStack(state);
}

//-------------------------------------------------------------------------------------------------
ConfigTable::ConfigTable(ConfigTable&& other) noexcept
  : lua_state_(std::move(other.lua_state_))
  , lua_table_ref_(other.lua_table_ref_)
  , size_(other.size_) {
  // reset internal data to flag moved object does not own resources anymore
  other.lua_table_ref_ = 0;
  other.size_ = 0;
}

//-------------------------------------------------------------------------------------------------
ConfigTable::~ConfigTable() {
  if (lua_table_ref_ != 0) {
    luaL_unref(lua_state_.get(), LUA_REGISTRYINDEX, lua_table_ref_);
  }
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::size() const -> size_t {
  return size_;
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readBool(const std::string& key) const
    -> std::expected<bool, ConfigTable::Error> {
  return readKey<bool>(lua_state_.get(), lua_table_ref_, key);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readInt(const std::string& key) const -> std::expected<int, ConfigTable::Error> {
  return readKey<int>(lua_state_.get(), lua_table_ref_, key);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readFloat(const std::string& key) const
    -> std::expected<float, ConfigTable::Error> {
  return readKey<float>(lua_state_.get(), lua_table_ref_, key);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readString(const std::string& key) const
    -> std::expected<std::string, ConfigTable::Error> {
  return readKey<std::string>(lua_state_.get(), lua_table_ref_, key);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readTable(const std::string& key) const
    -> std::expected<ConfigTable, ConfigTable::Error> {
  const auto on_success = [this](const ConfigTableDetail& d) -> std::expected<ConfigTable, Error> {
    return ConfigTable(this->lua_state_, d.table_reference);
  };
  const auto on_fail = [](const ConfigTable::Error& error) -> std::expected<ConfigTable, Error> {
    return std::unexpected(error);
  };
  return readKey<ConfigTableDetail>(lua_state_.get(), lua_table_ref_, key)  //
      .and_then(on_success)                                                 //
      .or_else(on_fail);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readBool(size_t index) const -> std::expected<bool, ConfigTable::Error> {
  return readIndex<bool>(lua_state_.get(), lua_table_ref_, index);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readInt(size_t index) const -> std::expected<int, ConfigTable::Error> {
  return readIndex<int>(lua_state_.get(), lua_table_ref_, index);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readFloat(size_t index) const -> std::expected<float, ConfigTable::Error> {
  return readIndex<float>(lua_state_.get(), lua_table_ref_, index);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readString(size_t index) const -> std::expected<std::string, ConfigTable::Error> {
  return readIndex<std::string>(lua_state_.get(), lua_table_ref_, index);
}

//-------------------------------------------------------------------------------------------------
auto ConfigTable::readTable(size_t index) const -> std::expected<ConfigTable, ConfigTable::Error> {
  const auto on_success = [this](const ConfigTableDetail& d) -> std::expected<ConfigTable, Error> {
    return ConfigTable(this->lua_state_, d.table_reference);
  };
  const auto on_fail = [](const auto& error) -> std::expected<ConfigTable, Error> {
    return std::unexpected(error);
  };
  return readIndex<ConfigTableDetail>(lua_state_.get(), lua_table_ref_, index)  //
      .and_then(on_success)                                                     //
      .or_else(on_fail);
}
}  // namespace grape::script
