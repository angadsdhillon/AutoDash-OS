# Test configuration
find_package(Catch2 REQUIRED)

# Test executable
add_executable(AutoDash-Tests
    test_config_load.cpp
    test_mock_i2c.cpp
    test_usb_monitor.cpp
    test_bluetooth_sim.cpp
    test_logger.cpp
)

# Link libraries
target_link_libraries(AutoDash-Tests
    Catch2::Catch2
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

# Include directories
target_include_directories(AutoDash-Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/system
    ${CMAKE_SOURCE_DIR}/src/ui
)

# Add tests
include(CTest)
include(Catch)
catch_discover_tests(AutoDash-Tests) 