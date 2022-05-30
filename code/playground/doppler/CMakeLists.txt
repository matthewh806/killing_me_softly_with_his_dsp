set(DopplerTestSources
    ${CMAKE_SOURCE_DIR}/playground/doppler/main.cpp
)
source_group("Source" FILES ${DopplerTestSources})

add_executable(DopplerTest ${DopplerTestSources})
target_sources(DopplerTest PRIVATE
    ${DopplerTestSources}
)