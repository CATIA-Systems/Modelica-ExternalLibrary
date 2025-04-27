cmake_minimum_required (VERSION 3.14)

project (ExternalLibrary)

set(EXTERNAL_LANGUAGE "CPP" CACHE STRING "Language of the external functions")
set_property(CACHE EXTERNAL_LANGUAGE PROPERTY STRINGS CPP Python)

set(PYTHON_EXE CACHE FILEPATH "Python executable")

execute_process(
    COMMAND ${PYTHON_EXE} "-c" "import os, sysconfig; print(sysconfig.get_config_var('installed_base').replace(os.sep, '/'))"
    OUTPUT_VARIABLE PYTHON_INSTALLED_BASE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${PYTHON_EXE} "-c" "import sysconfig; print(sysconfig.get_config_var('py_version_nodot'))"
    OUTPUT_VARIABLE PYTHON_VERSION_NODOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${PYTHON_EXE} "-c" "import os, sysconfig; print(sysconfig.get_path('include').replace(os.sep, '/'))"
    OUTPUT_VARIABLE PYTHON_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "PYTHON_INSTALLED_BASE: ${PYTHON_INSTALLED_BASE}")
message(STATUS "PYTHON_VERSION_NODOT: ${PYTHON_VERSION_NODOT}")
message(STATUS "PYTHON_INCLUDE_DIR: ${PYTHON_INCLUDE_DIR}")

if (MSVC)
  # link statically against the the Visual C runtime
  string(REPLACE "/MD"  "/MT"  CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
  string(REPLACE "/MD"  "/MT"  CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}")

  # disable CRT deprecation warnings
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

if (WIN32)
  set(MODELICA_PLATFORM win)
elseif (APPLE)
  set(MODELICA_PLATFORM darwin)
else ()
  set(MODELICA_PLATFORM linux)
endif ()

if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set (MODELICA_PLATFORM ${MODELICA_PLATFORM}64)
else ()
  set (MODELICA_PLATFORM ${MODELICA_PLATFORM}32)
endif ()

set(SOURCE_FILES
  ExternalLibrary/Resources/Include/ModelicaUtilityFunctions.h
  C/include/ExternalLibrary.h
)

if ("${EXTERNAL_LANGUAGE}" STREQUAL "CPP")
  set (SOURCE_FILES ${SOURCE_FILES} C/src/ExternalLibraryCPP.cpp)
else ()
  set (SOURCE_FILES ${SOURCE_FILES} C/src/ExternalLibraryPython.c)
endif ()

add_library(ExternalLibrary SHARED ${SOURCE_FILES})

target_include_directories(ExternalLibrary PUBLIC
  ExternalLibrary/Resources/Include
  C/include
)

if ("${EXTERNAL_LANGUAGE}" STREQUAL "Python")
  if (WIN32)
    target_include_directories(ExternalLibrary PUBLIC ${PYTHON_INCLUDE_DIR})
    target_link_libraries(ExternalLibrary ${PYTHON_INSTALLED_BASE}/libs/python${PYTHON_VERSION_NODOT}.lib)
    set(CMAKE_SHARED_LINKER_FLAGS " /DELAYLOAD:python${PYTHON_VERSION_NODOT}.dll")
  else ()
    target_include_directories(ExternalLibrary PUBLIC /usr/include/python3.10)
  endif ()
endif ()

SET_TARGET_PROPERTIES(ExternalLibrary PROPERTIES PREFIX "")

add_custom_command(TARGET ExternalLibrary POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  "$<TARGET_FILE:ExternalLibrary>"
  "${CMAKE_CURRENT_SOURCE_DIR}/ExternalLibrary/Resources/Library/${MODELICA_PLATFORM}/${CMAKE_SHARED_LIBRARY_PREFIX}ExternalLibrary${CMAKE_SHARED_LIBRARY_SUFFIX}"
)

add_executable(ExternalLibraryTest 
    C/tests/catch_amalgamated.hpp
    C/tests/catch_amalgamated.cpp
    C/tests/ExternalLibrary_test.cpp
)

target_compile_definitions(ExternalLibraryTest PUBLIC
  DATA_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/ExternalLibrary/Resources/Data/data.txt\"
  PYTHON_EXE=\"${PYTHON_EXE}\"
  PYTHON_DLL=\"${PYTHON_INSTALLED_BASE}/python${PYTHON_VERSION_NODOT}.dll\"
#  PYTHON_HOME=\"${PYTHON_HOME}\"
)

target_include_directories(ExternalLibraryTest PUBLIC
  ExternalLibrary/Resources/Include
  C/include
)

if (UNIX)
  target_link_libraries(ExternalLibraryTest dl)
endif ()

add_dependencies(ExternalLibraryTest ExternalLibrary)
