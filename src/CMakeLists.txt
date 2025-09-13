set(MODULE_SOURCES
    webpp.cppm
    errors.cppm
    basic.cppm
    strings.cppm
    event_loop.cppm
    js_object.cppm
    dom.cppm
    network.cppm
    coroutine.cppm
)
set(SOURCES
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_sources(${PROJECT_NAME} PUBLIC FILE_SET CXX_MODULES FILES ${MODULE_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${stdModule})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti -fno-exceptions)
