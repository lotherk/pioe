cmake_minimum_required(VERSION 3.0)

project (pioe LANGUAGES C VERSION 0.1.3)

set(CMAKE_BUILD_TYPE Debug)
set(PIOE_HOMEPAGE "https://github.com/lotherk/pioe")
set(PIOE_WIKIPAGE "https://github.com/lotherk/pioe/wiki")

set(LOGGER_FORMAT_DEFAULT "%D %T.%X.%F [%N] %L %f:%m:%l: %M")
set(LOGGER_FORMAT_DATE    "%Y-%m-%d")
set(LOGGER_FORMAT_TIME    "%H:%M:%S")

option(LOGGER_DEBUG "Enable logger debug messages" ON)
option(LOGGER_ENABLE "Enable logger messages" ON)
option(BUNDLE "Bundle Release Package for current platform" OFF)
option(SDL "Build SDL Plugin" ON)
option(VJOY "Build VJOY Plugin" OFF)
option(RUBY_ENGINE "Build Ruby Engine" ON)
option(PYTHON_ENGINE "Build Python Engine (EXPERIMENTAL! UNSUPPORTED!)" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

CONFIGURE_FILE(cmdline.ggo.in cmdline.ggo @ONLY)
CONFIGURE_FILE(config.h.in ../include/config.h @ONLY)

execute_process(COMMAND gengetopt -C --input cmdline.ggo --output-dir ../src --unamed-opts=SCRIPTS)
execute_process(COMMAND git log OUTPUT_FILE ChangeLog.txt)
execute_process(COMMAND doxygen WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_definitions("-Wfatal-errors")

add_library(pioengine SHARED 
		src/util.c
		src/logger.c
		src/error.c
		src/queue.c
		src/thread.c
		src/plugin.c
		src/engine.c
)

add_library(pioetestlib SHARED 
		src/testlib.c
)

target_compile_definitions(pioengine PUBLIC PIOENGINE_DLL_H)
target_compile_definitions(pioetestlib PUBLIC PIOENGINE_DLL_H)



if(CYGWIN)
	target_compile_definitions(pioengine PUBLIC _CYGWIN)
endif(CYGWIN)

if(WIN32)
	set(DLL ON)
	target_link_libraries(pioengine "winmm")
else(WIN32)
	target_link_libraries(pioengine "dl")
	target_link_libraries(pioengine "pthread")
	target_link_libraries(pioengine "m")
endif(WIN32)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src) # for gengetopt...


add_executable(pioe src/main.c src/cmdline.c)
add_executable(ptest src/ptest.c)

target_link_libraries(pioe pioengine)
target_link_libraries(ptest pioengine)
target_link_libraries(ptest pioetestlib)

install(TARGETS pioe pioengine
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)

install(DIRECTORY include/pioe DESTINATION include/)
install(DIRECTORY examples DESTINATION share/pioe)
install(DIRECTORY doc DESTINATION share/pioe)

install(FILES
	COPYING	
	build/ChangeLog.txt
	DESTINATION share/pioe)
if(SDL)
	include(${CMAKE_CURRENT_SOURCE_DIR}/plugins/sdl/CMakeLists.txt)
endif(SDL)
if(VJOY)
	include(${CMAKE_CURRENT_SOURCE_DIR}/plugins/vjoy/CMakeLists.txt)
endif(VJOY)

message(STATUS "")
if(RUBY_ENGINE)
    #FindRuby()
    find_package(Ruby)

	if(NOT RUBY_FOUND)
		message(FATAL_ERROR "Could not find ruby")
	else(NOT RUBY_FOUND)

		if(WIN32)
			set(DLL ON)
		endif(WIN32)

		message(STATUS "Ruby Engine enabled")
		message(STATUS "")

		add_library(pioerubyengine SHARED src/engine/ruby.c)
		target_link_libraries(pioerubyengine pioengine)

		message(STATUS "RUBY_VERSION: ${RUBY_VERSION}")
		message(STATUS "RUBY_LIBRARIES: ${RUBY_LIBRARIES}")
		message(STATUS "RUBY_LIBRARY: ${RUBY_LIBRARY}")
		message(STATUS "RUBY_INCLUDE_DIRS: ${RUBY_INCLUDE_DIRS}")
		message(STATUS "RUBY_CFLAGS_OTHER: ${RUBY_CFLAGS_OTHER}")

		target_include_directories(pioerubyengine PUBLIC ${RUBY_INCLUDE_DIRS})
		target_compile_options(pioerubyengine PUBLIC ${RUBY_CFLAGS_OTHER})
		if(NOT RUBY_LIBRARIES AND RUBY_LIBRARY)
			set(RUBY_LINK_TO ${RUBY_LIBRARY})
			#target_link_libraries(pioerubyengine ${RUBY_LIBRARY})
		else()
			set(RUBY_LINK_TO ${RUBY_LIBRARIES})
			#		target_link_libraries(pioerubyengine ${RUBY_LIBRARIES})
		endif()
		message(STATUS "RUBY_LINK_TO: ${RUBY_LINK_TO}")
		target_link_libraries(pioerubyengine ${RUBY_LINK_TO})
		install(TARGETS pioerubyengine
			RUNTIME DESTINATION bin
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib/static)

		message(STATUS "")
		target_compile_definitions(pioe PUBLIC HAVE_RUBY_ENGINE)
	endif(NOT RUBY_FOUND)
endif(RUBY_ENGINE)

if(PYTHON_ENGINE)
	if(WIN32)
		find_package(PythonInterp)
		set(PYTHON_FOUND ${PYTHONINTERP_FOUND})
		#        pkg_check_modules(PythonLibs 35REQUIRED python35)
		get_filename_component(PYTHON_BIN_PATH ${PYTHON_EXECUTABLE} DIRECTORY)

		set(PYTHON_LIBRARIES "${PYTHON_BIN_PATH}/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.dll")
		set(PYTHON_INCLUDE_DIRS "${PYTHON_BIN_PATH}/include")
		set(PYTHONLIBS_VERSION_STRING ${PYTHON_VERSION_STRING})
		set(DLL ON)
		#    set(PYTHON_CFLAGS_OTHER "-L 
	else(WIN32)
		find_package(PythonLibs)
		set(PYTHON_FOUND ${PYTHONLIBS_FOUND})
	endif(WIN32)

	if(NOT PYTHON_FOUND)
		message(FATAL_ERROR "Could not find python!")
	endif()

	message(STATUS "Python Engine enabled (EXPERIMENTAL!)")
	message(STATUS "")
	message(STATUS "PYTHON_VERSION: ${PYTHONLIBS_VERSION_STRING}")
	message(STATUS "")

	add_library(pioepythonengine SHARED src/engine/python.c)
	target_link_libraries(pioepythonengine pioengine)

	#string(REGEX REPLACE "\\." "" PYTHON_VER ${PYTHON_VERSION})
	#get_filename_component(PYTHON_BIN_PATH ${PYTHON_EXECUTABLE} DIRECTORY)

	message(STATUS "PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")
	message(STATUS "PYTHON_LIBRARY: ${PYTHON_LIBRARY}")
	message(STATUS "PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
	message(STATUS "PYTHON_CFLAGS_OTHER: ${PYTHON_CFLAGS_OTHER}")

	target_include_directories(pioepythonengine PUBLIC ${PYTHON_INCLUDE_DIRS})
	target_compile_options(pioepythonengine PUBLIC ${PYTHON_CFLAGS_OTHER})
	target_link_libraries(pioepythonengine ${PYTHON_LIBRARIES})

	install(TARGETS pioepythonengine
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)

	message(STATUS "")
	target_compile_definitions(pioe PUBLIC HAVE_PYTHON_ENGINE)
endif(PYTHON_ENGINE)

if(BUNDLE)
	if(CYGWIN)
		message(STATUS "Bundle for CYGWIN")
	endif(CYGWIN)
endif(BUNDLE)


set(CPACK_PACKAGE_CONTACT "Konrad Lother <k@hiddenbox.org>")
SET(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Description.txt")

if(WIN32)
	set(CPACK_WIX_UPGRADE_GUID "91C11B85-620B-4C5B-BF87-158277FA63C0")
	set(CPACK_GENERATOR "WIX;ZIP")
else(WIN32)
	set(CPACK_GENERATOR "DEB")
endif(WIN32)

set(CPACK_PACKAGE_FILE_NAME
	"${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_HOST_SYSTEM_PROCESSOR}")
set(CPACK_SOURCE_IGNORE_FILES
	"/build/;/.bzr/;/.git*;~$;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)


if(LOGGER_ENABLE)
	message(STATUS "Enabling logger messages")
	if(LOGGER_DEBUG)
		message(STATUS "Enabling logger debug messages")
		target_compile_definitions(pioengine PUBLIC LOGGER_DEBUG)
	endif(LOGGER_DEBUG)
else(LOGGER_ENABLE)
	target_compile_definitions(pioengine PUBLIC LOGGER_DISABLE)
endif(LOGGER_ENABLE)

message(STATUS "")

include(CTest)
add_test(logger ptest logger)
add_test(error ptest error)
add_test(linked_list ptest linked_list)
add_test(pe_sleep ptest pe_sleep)
add_test(pe_thread ptest pe_thread)
add_test(pe_engine ptest pe_engine)
#add_test(lukrop ptest lukrop)
