set(SOURCES
    TimeoutQueue.cpp
    )

add_library(${PROJECT_NAME}_lib ${SOURCES})

target_include_directories(${PROJECT_NAME}_lib PUBLIC ../include)
target_link_libraries(${PROJECT_NAME}_lib PRIVATE fmt::fmt)

