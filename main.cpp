#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <cmath>
#include <cstring>

struct WavHeader {
  char chunk_ID[4];
  uint32_t chunk_size;
  char format[4];

  char sub_chunk1_ID[4];
  uint32_t sub_chunk1_size;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;

  char sub_chunk2_ID[4];
  uint32_t sub_chunk2_size;
};

bool read_wav_file(const std::string& fname, WavHeader& header, std::vector<int16_t>& audio_data) {
  std::ifstream wavfile(fname, std::ios::binary);
  if (!wavfile.is_open()) {
    std::cerr << "Error: Could not open file " << fname << std::endl;
    return false;
  }

  wavfile.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

  if (std::string(header.format, 4) != "WAVE" || std::string(header.chunk_ID, 4) != "RIFF") {
    std::cerr << "Error: Not a valid WAVE/RIFF file!" << std::endl;
    return false;
  }

  std::cout << "File: " << fname << std::endl;
  std::cout << "File size: " << header.chunk_size + 8 << " bytes" << std::endl;
  std::cout << "Channels: " << header.num_channels << std::endl;
  std::cout << "Sample Rate: " << header.sample_rate << " Hz" << std::endl;
  std::cout << "Bits Per Sample: " << header.bits_per_sample << " bits" << std::endl;

  audio_data.resize(header.sub_chunk2_size / sizeof(int16_t));
  wavfile.read(reinterpret_cast<char*>(audio_data.data()), header.sub_chunk2_size);

  return true;
}

bool write_wav_file(const std::string& fname, const WavHeader& header, const std::vector<int16_t>& audio_data) {
  std::ofstream wavfile(fname, std::ios::binary);
  if (!wavfile.is_open()) {
    std::cerr << "Error: Could not create file " << fname << std::endl;
    return false;
  }

  wavfile.write(reinterpret_cast<const char *>(&header), sizeof(WavHeader));
  wavfile.write(reinterpret_cast<const char *>(audio_data.data()), header.sub_chunk2_size);

  return true;
}

WavHeader create_wav_header(uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t num_samples) {
  WavHeader header;

  std::memcpy(header.chunk_ID, "RIFF", 4);
  std::memcpy(header.format, "WAVE", 4);

  std::memcpy(header.sub_chunk1_ID, "fmt ", 4);
  header.sub_chunk1_size = 16;
  header.audio_format = 1;
  header.num_channels = num_channels;
  header.sample_rate = sample_rate;
  header.bits_per_sample = bits_per_sample;
  header.byte_rate = sample_rate * num_channels * bits_per_sample / 8;
  header.block_align = num_channels * bits_per_sample / 8;

  std::memcpy(header.sub_chunk2_ID, "data", 4);
  header.sub_chunk2_size = num_samples * num_channels * bits_per_sample / 8;

  header.chunk_size = 36 + header.sub_chunk2_size;

  return header;
}

int main() {
  uint32_t sample_rate = 44100;
  uint16_t channels = 1;
  uint16_t bits = 16;
  double duration_seconds = 2.0;
  uint32_t num_samples = static_cast<uint32_t>(sample_rate * duration_seconds);

  double frequency = 440.0;
  double amplitude = 30000;

  std::vector<int16_t> sine_wave(num_samples);

  for (uint32_t i = 0; i < num_samples; i++) {
    double t = static_cast<double>(i) / sample_rate;
    sine_wave[i] = static_cast<int16_t>(amplitude * std::sin(2.0 * M_PI * frequency * t));
  }

  WavHeader new_header =
      create_wav_header(sample_rate, channels, bits, num_samples);

  if (write_wav_file("output.wav", new_header, sine_wave)) {
    std::cout << "Created output.wav with a sine wave successfully!"
              << std::endl;
  }

  WavHeader header;
  std::vector<int16_t> audio_data;
  if (read_wav_file("./output.wav", header, audio_data)) {
    std::cout << "First 20 samples:" << std::endl;
    for (size_t i = 0; i < 20 && i < audio_data.size(); ++i) {
      std::cout << i << ": " << audio_data[i] << std::endl;
    }
  }

  return 0;
}