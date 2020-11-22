
#include <RtMidi.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "CLI11.hpp"
#include "player.h"
#include "recorder.h"

std::string sampleRates2str(const std::vector<unsigned int>& rates) {
  std::stringstream ss;
  ss << "[";
  for (int i = 0; i < rates.size(); i++) {
    ss << rates.at(i);
    if (i < rates.size() - 1) {
      ss << ",";
    }
  }
  ss << "]";
  return ss.str();
}

std::string bits2str(unsigned int mask) {
  std::vector<std::string> vec;
  if (mask | RTAUDIO_SINT8) {
    vec.push_back("8bit signed int");
  }
  if (mask | RTAUDIO_SINT16) {
    vec.push_back("16bit signed int");
  }
  if (mask | RTAUDIO_SINT24) {
    vec.push_back("24bit signed int");
  }
  if (mask | RTAUDIO_SINT32) {
    vec.push_back("32bit signed int");
  }
  if (mask | RTAUDIO_FLOAT32) {
    vec.push_back("32bit float");
  }
  if (mask | RTAUDIO_FLOAT64) {
    vec.push_back("64bit float");
  }
  std::stringstream ss;
  ss << "[";
  for (int i = 0; i < vec.size(); i++) {
    ss << vec.at(i);
    if (i < vec.size() - 1) {
      ss << ",";
    }
  }
  ss << "]";
  return ss.str();
}

std::string make_directory(std::string dirName) {
  ::time_t time = ::time(NULL);
  struct ::tm* time_data = ::localtime(&time);
  if (dirName == "") {
    char buff[512] = {};
    ::sprintf(buff, "y%d_m%d_d%d_h%d_m%d_s%d", 1900 + time_data->tm_year,
              1 + time_data->tm_mon, time_data->tm_mday, time_data->tm_hour,
              time_data->tm_min, time_data->tm_sec);
    dirName = std::string(buff);
  }
  std::filesystem::create_directory(dirName);
  return dirName;
}

std::shared_ptr<RtMidiOut> new_midi_out() {
  try {
    auto midi = std::make_shared<RtMidiOut>();
    return midi;
  } catch (RtMidiError e) {
    return nullptr;
  }
}

int main(int argc, char* argv[]) {
  // parse command line arguments
  using namespace CLI;
  App app{"autosample is recorder for synthesizer"};
  App* cmdLs = app.add_subcommand("ls", "list devices");
  App* cmdRec = app.add_subcommand("rec", "start recording");
  app.require_subcommand();

  int optMidi = 0;
  cmdRec->add_option("--midi", optMidi, "index for midi device", true);

  int optAudioIn = 0;
  cmdRec->add_option("--audioIn", optAudioIn, "index for audio input device.",
                     true);

  int optAudioOut = 0;
  cmdRec->add_option("--audioOut", optAudioOut,
                     "index for audio output device.", true);

  int optLow = 0;
  cmdRec->add_option("--low", optLow, "most lowest key in range", true);

  int optHigh = 127;
  cmdRec->add_option("--high", optHigh, "most highest key in range", true);

  int optVelocity = 127;
  cmdRec->add_option("--velocity", optVelocity, "sound velocity", true);

  int optMilliseconds = 1000;
  cmdRec->add_option("--milliseconds", optMilliseconds, "sound length", true);

  std::string optName = "";
  cmdRec->add_option("-n,--name", optName, "sound name", true);

  CLI11_PARSE(app, argc, argv);
  // initialize midi
  auto midi_out = new_midi_out();
  if (!midi_out) {
    std::cerr << "failed for initialize midi system." << std::endl;
    return 1;
  }
  // autosample ls
  RtAudio inputA;
  if (cmdLs->count()) {
    for (int i = 0; i < midi_out->getPortCount(); i++) {
      std::string name = midi_out->getPortName(i);
      std::cout << "MIDI OUT [" << i << "] " << name << std::endl;
    }
    for (int i = 0; i < inputA.getDeviceCount(); i++) {
      RtAudio::DeviceInfo device = inputA.getDeviceInfo(i);
      std::cout << "AUDIO [" << i << "] " << device.name << std::endl;
      std::cout << " InputChannels=" << device.inputChannels << std::endl;
      std::cout << " OutputChannels=" << device.outputChannels << std::endl;
      std::cout << " Format=" << bits2str(device.nativeFormats) << std::endl;
      std::cout << " SampleRate=" << sampleRates2str(device.sampleRates)
                << std::endl;
      std::cout << " PreferredSampleRate=" << device.preferredSampleRate
                << std::endl;
      std::cout << std::endl;
    }
  } else if (cmdRec->count()) {
    // make directory name if not specified
    std::string dirName = make_directory(optName);
    // recording
    midi_out->openPort(optMidi);
    int ammount = optHigh - optLow;
    std::filesystem::path baseDir = std::filesystem::path(dirName);
    std::filesystem::path bash = baseDir / std::filesystem::path("_sox.sh");
    std::vector<std::string> bashScript;
    std::vector<std::string> keyboards = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
    };
    RtAudio outputA;

    for (int i = optLow; i < optHigh; i++) {
      std::vector<unsigned char> messages;
      messages.push_back(0);
      messages.push_back(0);
      messages.push_back(0);
      recorder rec(inputA, optAudioIn);
      player pl(outputA, 1);
      int oct = i / 12;
      std::string key =
          std::to_string(i) + "_" + keyboards.at(i % 12) + std::to_string(oct);
      try {
        rec.start();
      } catch (RtAudioError& e) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        break;
      }
      if (!rec.is_active()) {
        break;
      }
      // NOTE ON
      messages[0] = 144;
      messages[1] = i;
      messages[2] = optVelocity;
      midi_out->sendMessage(&messages);
      std::this_thread::sleep_for(std::chrono::milliseconds(optMilliseconds));
      // NOTE OFF
      messages[0] = 128;
      messages[1] = i;
      messages[2] = 0;
      midi_out->sendMessage(&messages);
      // wait for recorder running
      while (rec.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      std::filesystem::path raw = std::filesystem::path(key + ".raw");
      std::filesystem::path wav = std::filesystem::path(key + ".wav");
      std::filesystem::path out = baseDir / raw;
      std::filesystem::path conv = baseDir / wav;
      rec.write(out.string());
      pl.start(out.string());
      bashScript.push_back(rec.make_sox_command(key + ".raw", key + ".wav"));
      std::this_thread::sleep_for(std::chrono::milliseconds(optMilliseconds));
      // all sound off
      messages[0] = 120;
      messages[1] = 120;
      messages[2] = 0;
      midi_out->sendMessage(&messages);
    }
    std::ofstream output_file(bash.string());
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");
    std::copy(bashScript.begin(), bashScript.end(), output_iterator);
    midi_out->closePort();
    // outputA.closeStream();
  }
  return 0;
}