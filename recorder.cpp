#include "recorder.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <thread>

#include "audio_common.h"

int input(void* /*outputBuffer*/, void* inputBuffer, unsigned int nBufferFrames,
          double /*streamTime*/, RtAudioStreamStatus /*status*/, void* data) {
  InputData* iData = (InputData*)data;

  // Simply copy the data to our allocated buffer.
  unsigned int frames = nBufferFrames;
  if (iData->frameCounter + nBufferFrames > iData->totalFrames) {
    frames = iData->totalFrames - iData->frameCounter;
    iData->bufferBytes = frames * iData->channels * sizeof(MY_TYPE);
  }

  unsigned long offset = iData->frameCounter * iData->channels;
  ::memcpy(iData->buffer + offset, inputBuffer, iData->bufferBytes);
  iData->frameCounter += frames;

  if (iData->frameCounter >= iData->totalFrames) return 2;
  return 0;
}

recorder::recorder(RtAudio& audio, int deviceId)
    : audio(audio),
      deviceInfo(audio.getDeviceInfo(deviceId)),
      iParams(),
      data(),
      deviceId(deviceId),
      milliseconds(1000),
      active(true) {
  audio.showWarnings(true);
  int formatN;
  std::string name;
  select_format(deviceInfo.nativeFormats, format, formatN, name);
  this->channels = deviceInfo.inputChannels;
  this->sampleRate = deviceInfo.preferredSampleRate;
  this->bufferFrames = 512;
}
recorder::~recorder() { destroy(); }
void recorder::startAsync() {
  // RtAudio::DeviceInfo info = audio.getDeviceInfo(deviceId);
  iParams.deviceId = deviceId;
  iParams.nChannels = channels;
  iParams.firstChannel = 0;
  data.buffer = 0;
  try {
    audio.openStream(NULL, &iParams, format, sampleRate, &bufferFrames, &input,
                     (void*)&data);
  } catch (RtAudioError& e) {
    std::cout << '\n' << e.getMessage() << '\n' << std::endl;
    destroy();
    return;
  }
  data.bufferBytes = bufferFrames * channels * sizeof(MY_TYPE);
  data.totalFrames =
      (unsigned long)(sampleRate * ((double)milliseconds / 1000));
  data.frameCounter = 0;
  data.channels = channels;
  unsigned long totalBytes;
  totalBytes = data.totalFrames * channels * sizeof(MY_TYPE);
  // Allocate the entire data buffer before starting stream.
  data.buffer = (MY_TYPE*)::malloc(totalBytes);
  if (data.buffer == 0) {
    std::cout << "Memory allocation error ... quitting!\n";
    destroy();
  }
  audio.startStream();
}
void recorder::write(const std::string& fn) {
  ::FILE* fd = ::fopen(fn.c_str(), "wb");
  ::fwrite(data.buffer, sizeof(MY_TYPE), data.totalFrames * channels, fd);
  ::fclose(fd);
}
void recorder::waitSync() const {
  while (audio.isStreamRunning()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
std::string recorder::make_sox_command(const std::string& raw,
                                       const std::string& wav) const {
  std::stringstream ss;
  RtAudioFormat format;
  int formatN = 0;
  std::string name = "signed-integer";

  select_format(deviceInfo.nativeFormats, format, formatN, name);
  ss << "sox -t raw -c " << channels << " -r " << sampleRate << " -b "
     << formatN << " -e " << name << " " << raw << " -t wav " << wav
     << std::endl;
  return ss.str();
}
// private
void recorder::destroy() {
  if (audio.isStreamOpen()) audio.closeStream();
  if (data.buffer) ::free(data.buffer);
  data.buffer = nullptr;
  this->active = false;
}