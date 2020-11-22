#ifndef PLAYER_H
#define PLAYER_H
#include <RtAudio.h>

#include "my_type.h"
struct OutputData {
  FILE* fd;
  unsigned int channels;
};
int output(void* outputBuffer, void* /*inputBuffer*/,
           unsigned int nBufferFrames, double /*streamTime*/,
           RtAudioStreamStatus /*status*/, void* data);
class player {
 public:
  explicit player(RtAudio& audio, int deviceId);
  ~player();
  void startSync(const std::string& fn);

 private:
  void destroy();
  RtAudio& audio;
  RtAudio::DeviceInfo deviceInfo;
  RtAudio::StreamParameters oParams;
  int deviceId;
  bool active;

  RtAudioFormat format;
  unsigned int channels;
  unsigned int sampleRate;
  unsigned int bufferFrames;
};
#endif