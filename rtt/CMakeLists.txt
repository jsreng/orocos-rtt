
include (CMakeDependentOption)
include (CreatePCFlags)

###
### List user options first before traversing sub-directories:
### All user options must be defined in this file, sub-directories can
### only read the options, not change them (Variable scope limits in cmake).
###

### OS Abstraction
OPTION(OS_AGNOSTIC "Do not use asm/system.h and asm/atomic.h global system heades, but use RTT implementations." ON)
OPTION(OS_HAVE_IOSTREAM "Use C++ iostream library." ON)
OPTION(OS_HAVE_STREAMS "Use C++ streams library." ON)
OPTION(OS_HAVE_MAIN "Provide main() function in rtt library, which sets up the OS. The user implements ORO_main()." ON)
OPTION(ORO_OS_USE_BOOST_THREAD "Use the Boost.Thread library. Currently only the mutex implementation is used." OFF)

if (ORO_OS_USE_BOOST_THREAD)
  # Look for boost threads
  find_package(Boost 1.36 COMPONENTS thread date_time REQUIRED)

  if (NOT Boost_THREAD_FOUND)
    message(SEND_ERROR "ORO_OS_USE_BOOST_THREAD turned on, but Boost thread library is not installed or wrong version.")
  endif()

  # As Boost_THREAD_LIBRARY may be a list of libraries, and we can't deal
  # with that in pkgconfig, we take 1) the library for this build type, or
  # 2) the library if only one is listed, or 3) we error out. 
  STRING(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
  if (DEFINED Boost_THREAD_LIBRARY_${CMAKE_BUILD_TYPE_UPPER})
	LIST(APPEND OROCOS-RTT_LIBRARIES ${Boost_THREAD_LIBRARY_${CMAKE_BUILD_TYPE_UPPER}})
  else (DEFINED Boost_THREAD_LIBRARY_${CMAKE_BUILD_TYPE_UPPER})
	LIST(LENGTH Boost_THREAD_LIBRARY COUNT_Boost_THREAD_LIBRARY)
	if (NOT WIN32)
		if (1 LESS COUNT_Boost_THREAD_LIBRARY)
			MESSAGE(FATAL_ERROR "Found multiple boost thread libraries, but not one specific to the current build type '${CMAKE_BUILD_TYPE_UPPER}'.")
		endif (1 LESS COUNT_Boost_THREAD_LIBRARY)
	endif()
	list(APPEND OROCOS-RTT_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_DATE_TIME_LIBRARY} )
  endif (DEFINED Boost_THREAD_LIBRARY_${CMAKE_BUILD_TYPE_UPPER})

endif()

SET(OS_MAX_CONC_ACCESS 8 CACHE STRING  "The maximum number of threads that will access a lock-free resource at exactly the same time. The Default (8) is very conservative. Your application may have more threads than this number.")
SET(OS_MAX_THREADS ${OS_MAX_CONC_ACCESS})
MARK_AS_ADVANCED(FORCE OS_MAX_CONC_ACCESS)

OPTION(OS_THREAD_SCOPE "Enable to monitor thread execution times through ThreadScope API." OFF)
OPTION(CONFIG_FORCE_UP "Enable to optimise for single core/cpu systems." OFF)

# Notify unit tests that no assembly must be tested.
SET(TESTS_OS_NO_ASM ${OS_NO_ASM} PARENT_SCOPE)
  
### Embedded
OPTION(OS_EMBEDDED "Build/Customise for embedded Operating System" OFF)
CMAKE_DEPENDENT_OPTION(OS_NOEXCEPTIONS "Disable exceptions." ON "OS_EMBEDDED" OFF)
CMAKE_DEPENDENT_OPTION(OS_EMBEDDED_SCRIPTING "Enable Scripting for embedded Operating System. (requires EXCEPTIONS)" OFF "NOT OS_NOEXCEPTIONS" OFF)

#Set build-local CFLAGS.
IF ( OS_NOEXCEPTIONS )
  SET( CMAKE_CXX_FLAGS_ADD "${CMAKE_CXX_FLAGS_ADD} -fno-exceptions")
ENDIF ( OS_NOEXCEPTIONS )

IF ( OS_EMBEDDED )
  SET( CMAKE_CXX_FLAGS_ADD "${CMAKE_CXX_FLAGS_ADD} -ffunction-sections -fdata-sections")
ENDIF ( OS_EMBEDDED )

### Logger
OPTION(OROBLD_DISABLE_LOGGING "Disable Logging Infrastructure" OFF)
CMAKE_DEPENDENT_OPTION(OROSEM_PRINTF_LOGGING "Logger uses printf()/fprintf() instead of std::iostream." OFF "NOT OROBLD_DISABLE_LOGGING" OFF)
CMAKE_DEPENDENT_OPTION(OROSEM_FILE_LOGGING "Logger logs to orocos.log file." ON "NOT OROBLD_DISABLE_LOGGING" OFF)
CMAKE_DEPENDENT_OPTION(OROSEM_REMOTE_LOGGING "Logger allows remote log retrieval." ON "NOT OROBLD_DISABLE_LOGGING" OFF)

IF (OROSEM_REMOTE_LOGGING AND NOT OROBLD_DISABLE_LOGGING)
  SET(ORONUM_LOGGING_BUFSIZE 1000 CACHE STRING "Maximum number of lines kept for remote log retrieval.")
ELSE (OROSEM_REMOTE_LOGGING AND NOT OROBLD_DISABLE_LOGGING)
  SET(ORONUM_LOGGING_BUFSIZE CACHE INTERNAL "")
ENDIF (OROSEM_REMOTE_LOGGING AND NOT OROBLD_DISABLE_LOGGING)

### Execution Engine
OPTION(OROPKG_EXECUTION_ADVANCED "Advanced Execution Engine configuration" OFF)
CMAKE_DEPENDENT_OPTION(OROPKG_EXECUTION_ENGINE_EVENTS "Enable Event Processing." ON "OROPKG_EXECUTION_ADVANCED" ON)
CMAKE_DEPENDENT_OPTION(OROPKG_EXECUTION_ENGINE_COMMANDS "Enable Command Processing." ON "OROPKG_EXECUTION_ADVANCED" ON)
CMAKE_DEPENDENT_OPTION(OROPKG_EXECUTION_ENGINE_PROGRAMS "Enable Program Processing." ON "OROPKG_EXECUTION_ADVANCED" ON)
CMAKE_DEPENDENT_OPTION(OROPKG_EXECUTION_ENGINE_STATEMACHINES "Enable Statemachine Processing." ON "OROPKG_EXECUTION_ADVANCED" ON)
IF (OROPKG_EXECUTION_ADVANCED)
  SET(ORONUM_EXECUTION_PROC_QUEUE_SIZE 16 CACHE STRING "Default Command Queue size.")
ELSE (OROPKG_EXECUTION_ADVANCED)
  SET(ORONUM_EXECUTION_PROC_QUEUE_SIZE 16 CACHE STRING "Default Command Queue size.")
ENDIF (OROPKG_EXECUTION_ADVANCED)

### Marshalling
OPTION(PLUGINS_ENABLE_MARSHALLING "Enable reading and writing Orocos Properties to XML." ON)

### Scripting
OPTION(PLUGINS_ENABLE_SCRIPTING "Enable loading Orocos program scripts." ON)

### RTT-Typekit
OPTION(PLUGINS_ENABLE_TYPEKIT "Enable default RTT typekit plugin." ON)

### TaskContext & Activities
OPTION(ORO_ACT_DEFAULT_ACTIVITY "Create default Activity object (thread) for each created TaskContext." ON)
CMAKE_DEPENDENT_OPTION(ORO_ACT_DEFAULT_SEQUENTIAL "Make the default activity a SequentialActivity (not-threaded) for each created TaskContext." OFF "ORO_ACT_DEFAULT_ACTIVITY" OFF)

### POSIX Message queues for IPC dataflow
OPTION(ENABLE_MQ "Enable real-time posix message queues for data-flow." ON)

### TLSF
CMAKE_DEPENDENT_OPTION(OS_RT_MALLOC "Enable RT memory management" ON "OS_HAS_TLSF" OFF)

IF ( OS_RT_MALLOC )
    SET(TLSF_FLAGS "")
    OPTION( OS_RT_MALLOC_SBRK "Enable RT memory management with sbrk support" ON)
    OPTION( OS_RT_MALLOC_MMAP "Enable RT memory management with mmap support" ON)
    OPTION( OS_RT_MALLOC_STATS "Enable RT memory management with statistics" ON)
    OPTION( OS_RT_MALLOC_DEBUG "Enable RT memory management debugging" OFF)
    
    IF (OS_RT_MALLOC_SBRK)
        SET( TLSF_FLAGS "${TLSF_FLAGS} -DUSE_SBRK")
    ENDIF (OS_RT_MALLOC_SBRK)
    IF (OS_RT_MALLOC_MMAP)
        SET( TLSF_FLAGS "${TLSF_FLAGS} -DUSE_MMAP -D_GNU_SOURCE")
    ENDIF (OS_RT_MALLOC_MMAP)
    IF (OS_RT_MALLOC_STATS)
        SET( TLSF_FLAGS "${TLSF_FLAGS} -DTLSF_STATISTIC")
    ENDIF (OS_RT_MALLOC_STATS)
    IF (OS_RT_MALLOC_DEBUG)
        SET( TLSF_FLAGS "${TLSF_FLAGS} -D_DEBUG_TLSF_")
    ENDIF (OS_RT_MALLOC_DEBUG)
    
    SET_SOURCE_FILES_PROPERTIES( os/tlsf/tlsf.c PROPERTIES
                                COMPILE_FLAGS "${TLSF_FLAGS}")

    GET_SOURCE_FILE_PROPERTY( TLSF os/tlsf/tlsf.c COMPILE_FLAGS)
    MESSAGE( "TLSF settings: ${TLSF}")

ENDIF (OS_RT_MALLOC )


###
#   Collect Sources:
#
### Subdirs must append srcs inhere.
SET(ENV{GLOBAL_LIBRARY_SRCS})

FILE( GLOB CPPS [^.]*.cpp )
FILE( GLOB HPPS [^.]*.hpp [^.]*.h [^.]*.inl)

GLOBAL_ADD_INCLUDE( rtt ${HPPS})
GLOBAL_ADD_INCLUDE( rtt ${CMAKE_CURRENT_BINARY_DIR}/rtt-config.h)

GLOBAL_ADD_SRC( ${CPPS})

IF (OS_RT_MALLOC )
    GLOBAL_ADD_SRC( os/tlsf/tlsf.c )
    GLOBAL_ADD_INCLUDE( rtt/os/tlsf os/tlsf/tlsf.h )
ENDIF (OS_RT_MALLOC )

ADD_SUBDIRECTORY( os )
ADD_SUBDIRECTORY( interface )
ADD_SUBDIRECTORY( base )
ADD_SUBDIRECTORY( internal )
ADD_SUBDIRECTORY( types )
ADD_SUBDIRECTORY( typekit )
ADD_SUBDIRECTORY( transports/corba )
ADD_SUBDIRECTORY( transports/mqueue )
ADD_SUBDIRECTORY( scripting )
ADD_SUBDIRECTORY( marsh )
ADD_SUBDIRECTORY( plugin )
ADD_SUBDIRECTORY( extras )

# Reduce debugging info in scripting dir.
#IF ( NOT OS_EMBEDDED OR OS_EMBEDDED_SCRIPTING AND CMAKE_BUILD_TYPE STREQUAL "Debug" )
#  MESSAGE("Changing flags for Scripting files")
# SET_SOURCE_FILES_PROPERTIES( $ENV{GLOBAL_SCRIPTING_SRCS} PROPERTIES COMPILE_FLAGS "-g -O1" )
#ENDIF ( NOT OS_EMBEDDED OR OS_EMBEDDED_SCRIPTING AND CMAKE_BUILD_TYPE STREQUAL "Debug" )
#SET_SOURCE_FILES_PROPERTIES( Timer.cpp PROPERTIES COMPILE_FLAGS "-g")

####
#   Build Libraries:
#

# Due to generation of some .h files in build directories, we also need to include some build dirs in our include paths.
INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR} ${PROJ_SOURCE_DIR}/rtt ${PROJ_SOURCE_DIR}/rtt/os/${OROCOS_TARGET} )
INCLUDE_DIRECTORIES(${PROJ_BINARY_DIR}/rtt ${PROJ_BINARY_DIR}/rtt/os ${PROJ_BINARY_DIR}/rtt/os/${OROCOS_TARGET} )

#SET(COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD} ${RTT_CFLAGS} -DOROCOS_TARGET=${OROCOS_TARGET}")

IF ( BUILD_STATIC )
  ADD_LIBRARY(orocos-rtt-${OROCOS_TARGET}_static STATIC $ENV{GLOBAL_LIBRARY_SRCS})
  SET_TARGET_PROPERTIES( orocos-rtt-${OROCOS_TARGET}_static
  PROPERTIES DEFINE_SYMBOL "RTT_DLL_EXPORT"
  OUTPUT_NAME orocos-rtt-${OROCOS_TARGET}
  DEBUG_OUTPUT_NAME orocos-rtt-${OROCOS_TARGET}${CMAKE_DEBUG_LIB_SUFFIX}
# Set accumulated compilation flags: (.so and .a)
  COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
  COMPILE_DEFINITIONS OROCOS_TARGET=${OROCOS_TARGET}
  VERSION "${RTT_VERSION}"
  CLEAN_DIRECT_OUTPUT 1)
ENDIF ( BUILD_STATIC )

  ADD_LIBRARY(orocos-rtt-${OROCOS_TARGET}_dynamic SHARED $ENV{GLOBAL_LIBRARY_SRCS})
  SET_TARGET_PROPERTIES( orocos-rtt-${OROCOS_TARGET}_dynamic PROPERTIES
  DEFINE_SYMBOL "RTT_DLL_EXPORT"
  SOVERSION "${RTT_VERSION_MAJOR}.${RTT_VERSION_MINOR}"
  VERSION "${RTT_VERSION}"
  OUTPUT_NAME orocos-rtt-${OROCOS_TARGET}
  DEBUG_OUTPUT_NAME orocos-rtt-${OROCOS_TARGET}${CMAKE_DEBUG_LIB_SUFFIX}
# Set accumulated compilation flags: (.so and .a)
  COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
  LINK_FLAGS "-Wl,-zdefs"
  COMPILE_DEFINITIONS OROCOS_TARGET=${OROCOS_TARGET}
  CLEAN_DIRECT_OUTPUT 1)
  target_link_libraries(orocos-rtt-${OROCOS_TARGET}_dynamic ${OROCOS-RTT_LIBRARIES})

create_pc_flags( "${OROCOS-RTT_DEFINITIONS}" "${OROCOS-RTT_INCLUDE_DIRS}" "${OROCOS-RTT_LIBRARIES}" RTT_DEFINES RTT_CFLAGS RTT_LINKFLAGS)
create_pc_flags( "${RTT_DUMMY}" "${RTT_DUMMY}" "${OROCOS-RTT_USER_LINK_LIBS}" RTT_DUMMY RTT_DUMMY RTT_USER_LINK_LIBS)

### Generate rtt-config.h : HAS NO ACCESS TO SUBDIRECTORY VARIABLES !!!
CONFIGURE_FILE( ${PROJ_SOURCE_DIR}/rtt/rtt-config.h.in ${PROJ_BINARY_DIR}/rtt/rtt-config.h @ONLY)
CONFIGURE_FILE( ${PROJ_SOURCE_DIR}/rtt/os/targets/rtt-target.h.in ${PROJ_BINARY_DIR}/rtt/os/targets/rtt-target.h @ONLY)
CONFIGURE_FILE( ${PROJ_SOURCE_DIR}/rtt/os/targets/target-config.h.in ${PROJ_BINARY_DIR}/rtt/os/targets/${OROCOS_TARGET}-config.h @ONLY)
CONFIGURE_FILE( ${PROJ_SOURCE_DIR}/rtt/orocos-rtt.pc.in ${PROJ_BINARY_DIR}/rtt/orocos-rtt-${OROCOS_TARGET}.pc @ONLY)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/orocos-rtt-${OROCOS_TARGET}.pc DESTINATION  lib/pkgconfig )
INSTALL(TARGETS orocos-rtt-${OROCOS_TARGET}_dynamic LIBRARY DESTINATION lib RUNTIME DESTINATION bin )
IF(BUILD_STATIC)
  INSTALL(TARGETS orocos-rtt-${OROCOS_TARGET}_static ARCHIVE DESTINATION lib )
ENDIF(BUILD_STATIC)

