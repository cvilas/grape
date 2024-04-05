# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

set(LUA_SOURCES_DIR ./lua-5.4.6/src)
set(LUA_SOURCES
    ${LUA_SOURCES_DIR}/lapi.c
    ${LUA_SOURCES_DIR}/lapi.h
    ${LUA_SOURCES_DIR}/lauxlib.c
    ${LUA_SOURCES_DIR}/lauxlib.h
    ${LUA_SOURCES_DIR}/lbaselib.c
    ${LUA_SOURCES_DIR}/lcode.c
    ${LUA_SOURCES_DIR}/lcode.h
    ${LUA_SOURCES_DIR}/lcorolib.c
    ${LUA_SOURCES_DIR}/lctype.c
    ${LUA_SOURCES_DIR}/lctype.h
    ${LUA_SOURCES_DIR}/ldblib.c
    ${LUA_SOURCES_DIR}/ldebug.c
    ${LUA_SOURCES_DIR}/ldebug.h
    ${LUA_SOURCES_DIR}/ldo.c
    ${LUA_SOURCES_DIR}/ldo.h
    ${LUA_SOURCES_DIR}/ldump.c
    ${LUA_SOURCES_DIR}/lfunc.c
    ${LUA_SOURCES_DIR}/lfunc.h
    ${LUA_SOURCES_DIR}/lgc.c
    ${LUA_SOURCES_DIR}/lgc.h
    ${LUA_SOURCES_DIR}/linit.c
    ${LUA_SOURCES_DIR}/liolib.c
    ${LUA_SOURCES_DIR}/ljumptab.h
    ${LUA_SOURCES_DIR}/llex.c
    ${LUA_SOURCES_DIR}/llex.h
    ${LUA_SOURCES_DIR}/llimits.h
    ${LUA_SOURCES_DIR}/lmathlib.c
    ${LUA_SOURCES_DIR}/lmem.c
    ${LUA_SOURCES_DIR}/lmem.h
    ${LUA_SOURCES_DIR}/loadlib.c
    ${LUA_SOURCES_DIR}/lobject.c
    ${LUA_SOURCES_DIR}/lobject.h
    ${LUA_SOURCES_DIR}/lopcodes.c
    ${LUA_SOURCES_DIR}/lopcodes.h
    ${LUA_SOURCES_DIR}/lopnames.h
    ${LUA_SOURCES_DIR}/loslib.c
    ${LUA_SOURCES_DIR}/lparser.c
    ${LUA_SOURCES_DIR}/lparser.h
    ${LUA_SOURCES_DIR}/lprefix.h
    ${LUA_SOURCES_DIR}/lstate.c
    ${LUA_SOURCES_DIR}/lstate.h
    ${LUA_SOURCES_DIR}/lstring.c
    ${LUA_SOURCES_DIR}/lstring.h
    ${LUA_SOURCES_DIR}/lstrlib.c
    ${LUA_SOURCES_DIR}/ltable.c
    ${LUA_SOURCES_DIR}/ltable.h
    ${LUA_SOURCES_DIR}/ltablib.c
    ${LUA_SOURCES_DIR}/ltm.c
    ${LUA_SOURCES_DIR}/ltm.h
    ${LUA_SOURCES_DIR}/lua.h
    ${LUA_SOURCES_DIR}/lua.hpp
    ${LUA_SOURCES_DIR}/luaconf.h
    ${LUA_SOURCES_DIR}/lualib.h
    ${LUA_SOURCES_DIR}/lundump.c
    ${LUA_SOURCES_DIR}/lundump.h
    ${LUA_SOURCES_DIR}/lutf8lib.c
    ${LUA_SOURCES_DIR}/lvm.c
    ${LUA_SOURCES_DIR}/lvm.h
    ${LUA_SOURCES_DIR}/lzio.c
    ${LUA_SOURCES_DIR}/lzio.h)

add_library(lualib OBJECT ${LUA_SOURCES})
set_target_properties(lualib PROPERTIES LINKER_LANGUAGE C)
set_target_properties(lualib PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
set_target_properties(lualib PROPERTIES POSITION_INDEPENDENT_CODE ON) # always need this

add_executable(grape_lua ${LUA_SOURCES_DIR}/lua.c)
target_link_libraries(grape_lua lualib m)
set_target_properties(grape_lua PROPERTIES LINKER_LANGUAGE C)
set_target_properties(grape_lua PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
install(TARGETS grape_lua)
