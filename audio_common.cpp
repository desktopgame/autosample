#include "audio_common.h"

RtAudioFormat select_format(RtAudioFormat formats, RtAudioFormat& oneFormat,
                            int& formatN, std::string& name) {
  RtAudioFormat ret = 0;
  name = "signed-integer";
  if (formats | RTAUDIO_SINT32) {
    ret = RTAUDIO_SINT32;
    formatN = 32;
  } else if (formats | RTAUDIO_SINT24) {
    ret = RTAUDIO_SINT24;
    formatN = 24;
  } else if (formats | RTAUDIO_SINT16) {
    ret = RTAUDIO_SINT16;
    formatN = 16;
  } else if (formats | RTAUDIO_SINT8) {
    ret = RTAUDIO_SINT8;
    formatN = 8;
  } else if (formats | RTAUDIO_FLOAT32) {
    ret = RTAUDIO_FLOAT32;
    formatN = 32;
    name = "floating-point";
  } else if (formats | RTAUDIO_FLOAT64) {
    ret = RTAUDIO_FLOAT64;
    formatN = 64;
    name = "floating-point";
  }
  oneFormat = ret;
  return ret;
}