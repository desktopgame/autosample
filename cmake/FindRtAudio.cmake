set(RTAUDIO_VERSION "6.1.0")

find_path(
    RTAUDIO_INCLUDE_DIR
    NAMES
        RtAudio.h
    PATHS
        "/usr/local/include"
        "/opt/local/include"
)

set(RTAUDIO_NAMES RtAudio rtaudio)
find_library(
    RTAUDIO_LIBRARY
    NAMES
        ${RTAUDIO_NAMES}
    PATHS
        "/usr/local/lib"
        "/opt/local/lib"
)
mark_as_advanced(RTAUDIO_LIBRARY RTAUDIO_INCLUDE_DIR)
set(RTAUDIO_INCLUDE_DIRS "${RTAUDIO_INCLUDE_DIR}")
set(RTAUDIO_LIBRARIES    "${RTAUDIO_LIBRARY}")

if(NOT RTAUDIO_INCLUDE_DIRS OR RTAUDIO_INCLUDE_DIRS MATCHES "NOTFOUND")
    set(RTAUDIO_FOUND false)
else()
    set(RTAUDIO_FOUND true)
endif()