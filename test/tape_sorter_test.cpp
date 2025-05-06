#include "tape_sorter.h"

#include "file_tape.h"

#include <catch2/catch_all.hpp>

#include <filesystem>
#include <random>

namespace {
std::vector<std::int32_t> read_all(const std::shared_ptr<file_tape>& t) {
  std::vector<std::int32_t> out;
  std::int32_t v;
  while (t->read(v)) {
    out.emplace_back(v);
  }
  return out;
}
} // namespace

TEST_CASE("Empty input tape", "[tape_sorter][empty]") {
  std::filesystem::remove_all("tmp");
  std::filesystem::create_directories("tmp");
  const std::filesystem::path in = "tmp/empty_in.bin";
  const std::filesystem::path out = "tmp/empty_out.bin";
  { file_tape t(in); }
  tape_sorter sorter(4 * sizeof(int32_t), 3);
  auto inTape = std::make_shared<file_tape>(in);
  auto outTape = std::make_shared<file_tape>(out);
  sorter.sort(inTape, outTape, "tmp");
  auto result = read_all(outTape);
  REQUIRE(result.empty());
}

TEST_CASE("Single element", "[tape_sorter][single]") {
  std::filesystem::remove_all("tmp");
  std::filesystem::create_directories("tmp");
  const auto in = "tmp/single_in.bin";
  const auto out = "tmp/single_out.bin";
  {
    file_tape t(in);
    t.write(42);
  }
  tape_sorter sorter(sizeof(int32_t), 2);
  auto inT = std::make_shared<file_tape>(in);
  auto outT = std::make_shared<file_tape>(out);
  sorter.sort(inT, outT, "tmp");
  auto v = read_all(outT);
  REQUIRE(v.size() == 1);
  CHECK(v[0] == 42);
}

TEST_CASE("All equal elements", "[tape_sorter][duplicates]") {
  std::filesystem::remove_all("tmp");
  std::filesystem::create_directories("tmp");
  constexpr auto n = 10;
  const auto in = "tmp/dup_in.bin";
  const auto out = "tmp/dup_out.bin";
  {
    file_tape t(in);
    for (int i = 0; i < n; ++i) {
      t.write(7);
    }
  }
  tape_sorter sorter(4 * sizeof(int32_t), 4);
  auto inT = std::make_shared<file_tape>(in);
  auto outT = std::make_shared<file_tape>(out);
  sorter.sort(inT, outT, "tmp");
  auto v = read_all(outT);
  REQUIRE(v.size() == std::size_t(n));
  for (auto x : v) {
    CHECK(x == 7);
  }
}

TEST_CASE("Reverse-sorted input", "[tape_sorter][reverse]") {
  std::filesystem::remove_all("tmp");
  std::filesystem::create_directories("tmp");
  const auto in = "tmp/rev_in.bin";
  const auto out = "tmp/rev_out.bin";
  {
    file_tape t(in);
    for (int i = 10; i >= 1; --i) {
      t.write(i);
    }
  }
  tape_sorter sorter(5 * sizeof(int32_t), 3);
  auto inT = std::make_shared<file_tape>(in);
  auto outT = std::make_shared<file_tape>(out);
  sorter.sort(inT, outT, "tmp");
  auto v = read_all(outT);
  REQUIRE(v.size() == 10);
  for (int i = 0; i < 10; ++i) {
    CHECK(v[i] == i + 1);
  }
}

TEST_CASE("Memory smaller than one run (forces many small runs)", "[tape_sorter][small-memory]") {
  std::filesystem::remove_all("tmp");
  std::filesystem::create_directories("tmp");
  const auto in = "tmp/smallmem_in.bin";
  const auto out = "tmp/smallmem_out.bin";
  {
    file_tape t(in);
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) {
      t.write(x);
    }
  }
  tape_sorter sorter(sizeof(int32_t), 4);
  auto inT = std::make_shared<file_tape>(in);
  auto outT = std::make_shared<file_tape>(out);
  sorter.sort(inT, outT, "tmp");
  auto v = read_all(outT);
  std::vector<int> expect = {1, 1, 2, 3, 4, 5, 6, 9};
  REQUIRE(v.size() == expect.size());
  for (size_t i = 0; i < expect.size(); ++i) {
    CHECK(v[i] == expect[i]);
  }
}

TEST_CASE("Negative and positive mix", "[tape_sorter][negpos]") {
  std::filesystem::remove_all("tmp");
  std::filesystem::create_directories("tmp");
  const auto in = "tmp/negpos_in.bin";
  const auto out = "tmp/negpos_out.bin";
  {
    file_tape t(in);
    for (int x : {-10, 0, 5, -3, 2, -1, 8}) {
      t.write(x);
    }
  }
  tape_sorter sorter(3 * sizeof(int32_t), 3);
  auto inT = std::make_shared<file_tape>(in);
  auto outT = std::make_shared<file_tape>(out);
  sorter.sort(inT, outT, "tmp");
  auto v = read_all(outT);
  std::vector<int> expect = {-10, -3, -1, 0, 2, 5, 8};
  REQUIRE(v == expect);
}

TEST_CASE("Large input (stress small random)", "[tape_sorter][stress]") {
  std::filesystem::remove_all("tmp");
  std::filesystem::create_directories("tmp");
  constexpr auto N = 1000;
  const auto in = "tmp/stress_in.bin";
  const auto out = "tmp/stress_out.bin";
  {
    std::mt19937_64 rnd(Catch::rngSeed());
    std::uniform_int_distribution dist(-10000, 10000);
    file_tape t(in);
    for (int i = 0; i < N; ++i) {
      t.write(dist(rnd));
    }
  }
  tape_sorter sorter(50 * sizeof(int32_t), 5);
  auto inT = std::make_shared<file_tape>(in);
  auto outT = std::make_shared<file_tape>(out);
  sorter.sort(inT, outT, "tmp");
  auto v = read_all(outT);
  REQUIRE(v.size() == N);
  REQUIRE(std::is_sorted(v.begin(), v.end()));
}
