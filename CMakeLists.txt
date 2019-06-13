cmake_minimum_required(VERSION 3.8)
set(This Timekeeping)

set(Headers
    include/Timekeeping/Clock.hpp
    include/Timekeeping/Scheduler.hpp
)

set(Sources
    src/Scheduler.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC include)

target_link_libraries(${This}
)

add_subdirectory(test)
