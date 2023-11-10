file(GLOB_RECURSE TEST_SRC_FILES CONFIGURE_DEPENDS
"./**/*.cpp"
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

foreach(TEST_FILE ${TEST_SRC_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    set(TEST_TARGET "test_${TEST_NAME}")
    add_executable(${TEST_TARGET} ${TEST_FILE})
    message("Test target: ${TEST_NAME}")
    target_compile_features(${TEST_TARGET} PUBLIC cxx_std_20)
    set_target_properties(${TEST_TARGET} PROPERTIES CXX_EXTENSIONS OFF)
    
    target_include_directories(${TEST_TARGET} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
    
    target_link_libraries(${TEST_TARGET} PUBLIC piped GTest::gtest_main)
    
    gtest_discover_tests(${TEST_TARGET})

endforeach()