set(RTMIDI_VERSION "4.0.0")

find_path(
    RTMIDI_INCLUDE_DIR
    NAMES
        RtMidi.h
    PATHS
        "/usr/local/include"
        "/usr/local/Cellar/rtmidi/${RTMIDI_VERSION}/include"
        "/opt/local/include"
)

set(RTMIDI_NAMES RtMidi rtmidi)
find_library(
    RTMIDI_LIBRARY
    NAMES
        ${RTMIDI_NAMES}
    PATHS
        "/usr/local/lib"
        "/usr/local/Cellar/rtmidi/${RTMIDI_VERSION}/lib"
        "/opt/local/lib"
)
mark_as_advanced(RTMIDI_LIBRARY RTMIDI_INCLUDE_DIR)
set(RTMIDI_INCLUDE_DIRS "${RTMIDI_INCLUDE_DIR}")
set(RTMIDI_LIBRARIES    "${RTMIDI_LIBRARY}")

if(NOT RTMIDI_INCLUDE_DIRS OR RTMIDI_INCLUDE_DIRS MATCHES "NOTFOUND")
    set(RTMIDI_FOUND false)
else()
    set(RTMIDI_FOUND true)
endif()