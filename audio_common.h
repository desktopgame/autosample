#ifndef COMMON_H
#define COMMON_H
#include <RtAudio.h>

#include <string>

RtAudioFormat select_format(RtAudioFormat formats, RtAudioFormat& oneFormat,
                            int& formatN, std::string& name);
#endif