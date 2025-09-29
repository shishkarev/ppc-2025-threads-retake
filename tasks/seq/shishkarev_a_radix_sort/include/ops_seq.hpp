#pragma once

#include <utility>
#include <vector>

#include "core/task/include/task.hpp"

namespace shishkarev_a_radix_sort {

class TestTaskSequential : public ppc::core::Task {
 public:
  explicit TestTaskSequential(ppc::core::TaskDataPtr task_data) : Task(std::move(task_data)) {}
  bool PreProcessingImpl() override;
  bool ValidationImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

 private:
  std::vector<int> input_;
  std::vector<int> output_;

  static void RadixSort(std::vector<int>& arr);
  static int GetMax(const std::vector<int>& arr);
  static void CountSort(std::vector<int>& arr, int exp);
  static void BatcherOddEvenMerge(std::vector<int>& arr, int left, int right);
};

}  // namespace shishkarev_a_radix_sort