#include "recorder.h"

#include <sstream>
#include <thread>

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
  memcpy(iData->buffer + offset, inputBuffer, iData->bufferBytes);
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
  if (deviceInfo.nativeFormats | RTAUDIO_SINT32) {
    this->format = RTAUDIO_SINT32;
  } else if (deviceInfo.nativeFormats | RTAUDIO_SINT24) {
    this->format = RTAUDIO_SINT24;
  } else if (deviceInfo.nativeFormats | RTAUDIO_SINT16) {
    this->format = RTAUDIO_SINT16;
  } else if (deviceInfo.nativeFormats | RTAUDIO_SINT8) {
    this->format = RTAUDIO_SINT8;
  } else if (deviceInfo.nativeFormats | RTAUDIO_FLOAT32) {
    this->format = RTAUDIO_FLOAT32;
  } else if (deviceInfo.nativeFormats | RTAUDIO_FLOAT64) {
    this->format = RTAUDIO_FLOAT64;
  } else {
    this->active = false;
  }
  this->channels = deviceInfo.inputChannels;
  this->sampleRate = deviceInfo.preferredSampleRate;
  this->bufferFrames = 512;
}
recorder::~recorder() { audio.closeStream(); }
void recorder::start() {
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
bool recorder::is_active() const { return active; }
bool recorder::is_running() const { return audio.isStreamRunning(); }
std::string recorder::make_sox_command(const std::string& raw,
                                       const std::string& wav) const {
  std::stringstream ss;
  int formatN = 0;
  std::string formatName = "signed-integer";

  if (deviceInfo.nativeFormats | RTAUDIO_SINT32) {
    formatN = 32;
  } else if (deviceInfo.nativeFormats | RTAUDIO_SINT24) {
    formatN = 24;
  } else if (deviceInfo.nativeFormats | RTAUDIO_SINT16) {
    formatN = 16;
  } else if (deviceInfo.nativeFormats | RTAUDIO_SINT8) {
    formatN = 8;
  } else if (deviceInfo.nativeFormats | RTAUDIO_FLOAT32) {
    formatN = 32;
    formatName = "floating-point";
  } else if (deviceInfo.nativeFormats | RTAUDIO_FLOAT64) {
    formatN = 64;
    formatName = "floating-point";
  }
  ss << "sox -t raw -c " << channels << " -r " << sampleRate << " -b "
     << formatN << " -e " << formatName << " " << raw << " -t wav " << wav
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