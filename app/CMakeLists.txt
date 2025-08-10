set(EXAMPLE_SOURCES
    main.cpp
)
add_executable(${PROJECT_NAME}_example ${EXAMPLE_SOURCES})
target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME}_lib)