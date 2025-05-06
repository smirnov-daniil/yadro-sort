#pragma once
#ifndef __tape_sorter_h_
#define __tape_sorter_h_

#include "file_tape.h"

#include <queue>

#include <filesystem>
#include <memory>
#include <vector>

class tape_sorter {
public:
  explicit tape_sorter(std::size_t memory, std::size_t tmp_tapes = 3);

  void sort(
      const std::shared_ptr<tape>& in,
      const std::shared_ptr<tape>& out,
      const std::filesystem::path& tmp_dir_path
  ) const;

private:
  [[nodiscard]] std::vector<std::filesystem::path>
  split_to_tmps(const std::shared_ptr<tape>& in, const std::filesystem::path& tmp_dir_path) const;

  template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, std::filesystem::path>
  std::filesystem::path
  merge(const R& group, const std::filesystem::path& tmp_dir_path, std::size_t pass_idx, std::size_t group_idx) const;

private:
  std::size_t mem_limit;
  std::size_t temp_tapes_limit;
};

template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, std::filesystem::path>
std::filesystem::path
tape_sorter::merge(const R& group, const std::filesystem::path& tmp_dir_path, std::size_t pass_idx, std::size_t group_idx)
    const {
  struct node {
    std::int32_t value;
    std::size_t idx;
  };

  auto cmp = [](const node& a, const node& b) {
    return a.value > b.value;
  };
  std::priority_queue<node, std::vector<node>, decltype(cmp)> queue(cmp);

  std::vector<std::shared_ptr<file_tape>> tapes;
  tapes.reserve(std::ranges::distance(group));

  for (auto& fn : group) {
    auto t = std::make_shared<file_tape>(fn);
    t->rewind();
    if (std::int32_t v; t->read(v)) {
      queue.emplace(v, tapes.size());
    }
    tapes.emplace_back(t);
  }

  std::filesystem::path out_path =
      tmp_dir_path / ("run_p" + std::to_string(pass_idx) + "_g" + std::to_string(group_idx) + ".bin");
  file_tape out_t(out_path);

  while (!queue.empty()) {
    auto [val, idx] = queue.top();
    queue.pop();
    out_t.write(val);
    if (std::int32_t next; tapes[idx]->read(next)) {
      queue.emplace(next, idx);
    }
  }
  out_t.rewind();
  return out_path;
}

#endif // !__tape_sorter_h_
