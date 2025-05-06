#pragma once
#ifndef __file_tape_h_
#define __file_tape_h_

#include "tape.h"

#include <cstdint>
#include <filesystem>
#include <fstream>

class file_tape : public tape {
private:
  struct delays {
    std::size_t read_ms;
    std::size_t write_ms;
    std::size_t step_ms;
    std::size_t rewind_ms;
  };

public:
  explicit file_tape(const std::filesystem::path& file_path);
  ~file_tape() override = default;

  bool read(std::int32_t& value) override;
  bool write(std::int32_t value) override;
  bool step(bool forward) override;
  void rewind() override;

private:
  static void delay(std::size_t ms);

private:
  delays _delays{};
  std::fstream _file;
};

#endif // !__file_tape_h_
