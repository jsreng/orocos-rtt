
  ADD_LIBRARY(typekit_plugin SHARED plugins_test_types.cpp)
  SET_TARGET_PROPERTIES( typekit_plugin PROPERTIES
    SOVERSION "${RTT_VERSION_MAJOR}.${RTT_VERSION_MINOR}"
    VERSION "${RTT_VERSION}"
    OUTPUT_NAME typekit_plugin
    DEBUG_OUTPUT_NAME typekit_plugin${CMAKE_DEBUG_LIG_SUFFIX}
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "-Wl,-zdefs"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}"
    )
  target_link_libraries(typekit_plugin orocos-rtt-${OROCOS_TARGET}_dynamic)

