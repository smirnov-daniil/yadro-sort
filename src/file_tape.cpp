#include "file_tape.h"

#include <chrono>
#include <thread>

file_tape::file_tape(const std::filesystem::path& file_path)
    : _file(file_path, std::ios::binary | std::ios::in | std::ios::out | std::ios::app) {
  std::ifstream cfg("config.txt");
  cfg >> _delays.read_ms >> _delays.write_ms >> _delays.step_ms >> _delays.rewind_ms;
}

void file_tape::delay(std::size_t ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

bool file_tape::read(std::int32_t& value) {
  delay(_delays.read_ms);
  if (!_file.read(reinterpret_cast<char*>(&value), sizeof(value))) {
    return false;
  }
  delay(_delays.step_ms);
  return true;
}

bool file_tape::write(std::int32_t value) {
  delay(_delays.write_ms);
  if (!_file.write(reinterpret_cast<char*>(&value), sizeof(value))) {
    return false;
  }
  delay(_delays.step_ms);
  return true;
}

bool file_tape::step(bool forward) {
  delay(_delays.step_ms);
  std::streamoff off = sizeof(std::int32_t);
  off = forward ? off : -off;
  _file.seekg(off, std::ios::cur);
  _file.seekp(off, std::ios::cur);
  return static_cast<bool>(_file);
}

void file_tape::rewind() {
  delay(_delays.rewind_ms);
  _file.clear();
  _file.seekg(0);
  _file.seekp(0);
}
