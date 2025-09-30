#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

#include "core/perf/include/perf.hpp"
#include "core/task/include/task.hpp"
#include "tbb/shishkarev_a_radix_sort/include/ops_tbb.hpp"

TEST(shishkarev_a_radix_sort_tbb, test_pipeline_run) {
  constexpr int kCount = 1000000;

  std::vector<int> in(kCount);
  std::vector<int> out(kCount);
  std::vector<int> expected(kCount);  // Для проверки результатов

  for (int i = 0; i < kCount; ++i) {
    in[i] = rand() % 10000000;
  }

  // Создаем копию для эталонной сортировки
  expected = in;
  std::ranges::sort(expected);

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  auto test_task_tbb = std::make_shared<shishkarev_a_radix_sort_tbb::TestTaskTBB>(task_data_tbb);

  auto perf_attr = std::make_shared<ppc::core::PerfAttr>();
  perf_attr->num_running = 50;
  const auto t0 = std::chrono::high_resolution_clock::now();
  perf_attr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  auto perf_results = std::make_shared<ppc::core::PerfResults>();

  auto perf_analyzer = std::make_shared<ppc::core::Perf>(test_task_tbb);
  perf_analyzer->PipelineRun(perf_attr, perf_results);
  ppc::core::Perf::PrintPerfStatistic(perf_results);

  // Проверка результатов
  bool sorted_correctly = true;
  for (size_t i = 0; i < out.size(); ++i) {
    if (out[i] != expected[i]) {
      sorted_correctly = false;
      break;
    }
  }
  EXPECT_TRUE(sorted_correctly);

  // Дополнительная проверка: убедимся, что массив действительно отсортирован
  bool is_sorted = true;
  for (size_t i = 1; i < out.size(); ++i) {
    if (out[i] < out[i - 1]) {
      is_sorted = false;
      break;
    }
  }
  EXPECT_TRUE(is_sorted);
}

TEST(shishkarev_a_radix_sort_tbb, test_task_run) {
  constexpr int kCount = 100000;

  std::vector<int> in(kCount);
  std::vector<int> out(kCount);
  std::vector<int> expected(kCount);  // Для проверки результатов

  for (int i = 0; i < kCount; ++i) {
    in[i] = rand() % 10000000;
  }

  // Создаем копию для эталонной сортировки
  expected = in;
  std::ranges::sort(expected);

  auto task_data_tbb = std::make_shared<ppc::core::TaskData>();
  task_data_tbb->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_tbb->inputs_count.emplace_back(in.size());
  task_data_tbb->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_tbb->outputs_count.emplace_back(out.size());

  auto test_task_tbb = std::make_shared<shishkarev_a_radix_sort_tbb::TestTaskTBB>(task_data_tbb);

  auto perf_attr = std::make_shared<ppc::core::PerfAttr>();
  perf_attr->num_running = 100;
  const auto t0 = std::chrono::high_resolution_clock::now();
  perf_attr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  auto perf_results = std::make_shared<ppc::core::PerfResults>();

  auto perf_analyzer = std::make_shared<ppc::core::Perf>(test_task_tbb);
  perf_analyzer->TaskRun(perf_attr, perf_results);
  ppc::core::Perf::PrintPerfStatistic(perf_results);

  // Проверка результатов
  bool sorted_correctly = true;
  for (size_t i = 0; i < out.size(); ++i) {
    if (out[i] != expected[i]) {
      sorted_correctly = false;
      break;
    }
  }
  EXPECT_TRUE(sorted_correctly);

  // Дополнительная проверка: убедимся, что массив действительно отсортирован
  bool is_sorted = true;
  for (size_t i = 1; i < out.size(); ++i) {
    if (out[i] < out[i - 1]) {
      is_sorted = false;
      break;
    }
  }
  EXPECT_TRUE(is_sorted);
}