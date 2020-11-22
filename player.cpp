#include "player.h"

#include <thread>

#include "audio_common.h"
// Interleaved buffers
int output(void *outputBuffer, void * /*inputBuffer*/,
           unsigned int nBufferFrames, double /*streamTime*/,
           RtAudioStreamStatus /*status*/, void *data) {
  OutputData *oData = (OutputData *)data;

  // In general, it's not a good idea to do file input in the audio
  // callback function but I'm doing it here because I don't know the
  // length of the file we are reading.
  unsigned int count = fread(outputBuffer, oData->channels * sizeof(MY_TYPE),
                             nBufferFrames, oData->fd);
  if (count < nBufferFrames) {
    unsigned int bytes =
        (nBufferFrames - count) * oData->channels * sizeof(MY_TYPE);
    unsigned int startByte = count * oData->channels * sizeof(MY_TYPE);
    memset((char *)(outputBuffer) + startByte, 0, bytes);
    return 1;
  }

  return 0;
}
player::player(RtAudio &audio, int deviceId)
    : audio(audio),
      deviceInfo(audio.getDeviceInfo(deviceId)),
      oParams(),
      deviceId(deviceId),
      active(true) {
  audio.showWarnings(true);
  int formatN;
  std::string name;
  select_format(deviceInfo.nativeFormats, format, formatN, name);
  this->channels = deviceInfo.outputChannels;
  this->sampleRate = deviceInfo.preferredSampleRate;
  this->bufferFrames = 512;
}
void player::start(const std::string &fn) {
  oParams.deviceId = deviceId;
  oParams.nChannels = channels;
  oParams.firstChannel = 0;
  OutputData data;
  data.fd = ::fopen(fn.c_str(), "rb");
  if (data.fd == nullptr) {
    destroy();
    return;
  }

  data.channels = channels;
  try {
    audio.openStream(&oParams, NULL, format, sampleRate, &bufferFrames, &output,
                     (void *)&data);
    audio.startStream();
  } catch (RtAudioError &e) {
    std::cout << '\n' << e.getMessage() << '\n' << std::endl;
    ::fclose(data.fd);
    destroy();
    return;
  }
  while (is_running()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  ::fclose(data.fd);
  destroy();
}
bool player::is_active() const { return active; }
bool player::is_running() const { return audio.isStreamRunning(); }
// private
void player::destroy() {
  if (audio.isStreamOpen()) audio.closeStream();
  // if (data.buffer) ::free(data.buffer);
  // data.buffer = nullptr;
  this->active = false;
}