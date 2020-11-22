cmake_minimum_required(VERSION 3.16)
project(autosample CXX)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

find_package(RtMidi REQUIRED)
find_package(RtAudio REQUIRED)

add_executable(autosample)

install(TARGETS autosample RUNTIME DESTINATION bin CONFIGURATIONS Release)
install(TARGETS autosample RUNTIME DESTINATION bin CONFIGURATIONS Debug)
target_sources(autosample PRIVATE main.cpp recorder.cpp player.cpp audio_common.cpp)
target_include_directories(autosample PRIVATE ${RTMIDI_INCLUDE_DIRS} ${RTAUDIO_INCLUDE_DIRS})
target_link_libraries(autosample PRIVATE ${RTMIDI_LIBRARIES} ${RTAUDIO_LIBRARIES})