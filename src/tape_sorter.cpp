#include "tape_sorter.h"

#include "file_tape.h"

#include <algorithm>
#include <fstream>
#include <ranges>

namespace fs = std::filesystem;

tape_sorter::tape_sorter(std::size_t memory, std::size_t tmp_tapes)
    : mem_limit(memory)
    , temp_tapes_limit(tmp_tapes) {
  if (temp_tapes_limit < 1 || mem_limit < sizeof(std::int32_t) || mem_limit == sizeof(std::int32_t) && tmp_tapes == 1) {
    throw std::runtime_error("Insufficient limits");
  }
}

std::vector<std::filesystem::path>
tape_sorter::split_to_tmps(const std::shared_ptr<tape>& in, const std::filesystem::path& tmp_dir_path) const {
  std::vector<fs::path> paths;
  std::vector<std::int32_t> buffer;
  buffer.reserve(mem_limit / sizeof(std::int32_t));
  std::int32_t v;
  std::size_t runIdx = 0;

  auto process_buffer = [&tmp_dir_path](std::vector<std::int32_t>& buffer, const std::size_t idx) {
    std::ranges::sort(buffer);
    fs::path fn = tmp_dir_path / (std::string("run_") + std::to_string(idx) + ".bin");
    file_tape out(fn);
    for (const auto x : buffer) {
      out.write(x);
    }
    buffer.resize(0);
    return fn;
  };

  while (in->read(v)) {
    buffer.emplace_back(v);
    if (buffer.size() * sizeof(std::int32_t) >= mem_limit) {
      paths.emplace_back(process_buffer(buffer, runIdx++));
    }
  }
  if (!buffer.empty()) {
    paths.emplace_back(process_buffer(buffer, runIdx++));
  }
  return paths;
}

void tape_sorter::sort(
    const std::shared_ptr<tape>& in,
    const std::shared_ptr<tape>& out,
    const std::filesystem::path& tmp_dir_path
) const {
  auto paths = split_to_tmps(in, tmp_dir_path);
  std::size_t pass = 0;

  while (paths.size() > 1) {
    std::vector<fs::path> next;
    const std::size_t groups = (paths.size() + temp_tapes_limit - 1) / temp_tapes_limit;

    for (auto groupIdx : std::views::iota(static_cast<std::size_t>(0), groups)) {
      auto group_view = paths | std::views::drop(groupIdx * temp_tapes_limit) | std::views::take(temp_tapes_limit);
      next.emplace_back(merge(group_view, tmp_dir_path, pass, groupIdx));
    }

    for (auto& p : paths) {
      fs::remove(p);
    }

    paths = std::move(next);
    ++pass;
  }

  if (!paths.empty()) {
    {
      auto final_tape = std::make_shared<file_tape>(paths.front());
      final_tape->rewind();
      std::int32_t v;
      while (final_tape->read(v)) {
        out->write(v);
      }
    }
    fs::remove(paths.front());
  }
  out->rewind();
}
