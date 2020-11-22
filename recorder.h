#ifndef RECORDER_H
#define RECORDER_H
#include <RtAudio.h>

#include <string>

#include "my_type.h"

struct InputData {
  MY_TYPE* buffer;
  unsigned long bufferBytes;
  unsigned long totalFrames;
  unsigned long frameCounter;
  unsigned int channels;
};
int input(void* /*outputBuffer*/, void* inputBuffer, unsigned int nBufferFrames,
          double /*streamTime*/, RtAudioStreamStatus /*status*/, void* data);

class recorder {
 public:
  explicit recorder(RtAudio& audio, int deviceId);
  ~recorder();
  void startAsync();
  void write(const std::string& fn);
  void waitSync() const;
  std::string make_sox_command(const std::string& raw,
                               const std::string& wav) const;

 private:
  void destroy();
  RtAudio& audio;
  RtAudio::DeviceInfo deviceInfo;

  RtAudio::StreamParameters iParams;
  InputData data;
  int deviceId;
  int milliseconds;
  bool active;

  RtAudioFormat format;
  unsigned int channels;
  unsigned int sampleRate;
  unsigned int bufferFrames;
};
#endif