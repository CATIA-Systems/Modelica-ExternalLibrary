cmake_minimum_required (VERSION 3.14)

project (ExternalLibrary)

set(EXTERNAL_LANGUAGE "CPP" CACHE STRING "Language of the external functions")
set_property(CACHE EXTERNAL_LANGUAGE PROPERTY STRINGS CPP Python)

set(PYTHON_HOME "E:/Anaconda3/envs/py37_64" CACHE STRING "Python home")

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
    target_include_directories(ExternalLibrary PUBLIC ${PYTHON_HOME}/include)
    file(GLOB PYTHON_LIB ${PYTHON_HOME}/libs/python3?*.lib)
    target_link_libraries(ExternalLibrary ${PYTHON_LIB})
    file(GLOB PYTHON_DLL ${PYTHON_HOME}/python3?*.dll)
    get_filename_component(PYTHON_DLL ${PYTHON_DLL} NAME)
    set(CMAKE_SHARED_LINKER_FLAGS " /DELAYLOAD:${PYTHON_LIBRARY_NAME}.dll")
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
  PYTHON_HOME=\"${PYTHON_HOME}\"
)

target_include_directories(ExternalLibraryTest PUBLIC
  ExternalLibrary/Resources/Include
  C/include
)

if (UNIX)
  target_link_libraries(ExternalLibraryTest dl)
endif ()

add_dependencies(ExternalLibraryTest ExternalLibrary)
