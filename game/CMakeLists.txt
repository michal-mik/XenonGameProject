add_executable(xenon_game
    src/main.cpp
    src/XenonGame.cpp
    src/ShipPawn.cpp
)

target_link_libraries(xenon_game
    PRIVATE xenon_engine
)

target_include_directories(xenon_game
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_custom_command(TARGET xenon_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/graphics
            $<TARGET_FILE_DIR:xenon_game>/graphics
)

