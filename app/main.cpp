#include "file_tape.h"
#include "tape_sorter.h"

#include <cstddef>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <in> <out>\n";
    return 1;
  }
  std::string in_file = argv[1], out_file = argv[2];
  tape_sorter sorter(static_cast<std::size_t>(1024) * 1024 * 50, 4);
  std::filesystem::create_directories("tmp");
  sorter.sort(std::make_shared<file_tape>(in_file), std::make_shared<file_tape>(out_file), "tmp");
  return 0;
}
