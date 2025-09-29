#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <vector>

#include "core/task/include/task.hpp"
#include "seq/shishkarev_a_radix_sort/include/ops_seq.hpp"

TEST(shishkarev_a_radix_sort_seq, test_sort_small) {
  constexpr size_t kCount = 10;

  std::vector<int> in(kCount);
  std::vector<int> out(kCount);

  for (size_t i = 0; i < kCount; i++) {
    in[i] = rand() % 1000;
  }

  auto task_data_seq = std::make_shared<ppc::core::TaskData>();
  task_data_seq->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_seq->inputs_count.emplace_back(in.size());
  task_data_seq->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_seq->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort::TestTaskSequential test_task_sequential(task_data_seq);

  // Правильный порядок вызовов: Validation -> PreProcessing -> Run -> PostProcessing
  ASSERT_EQ(test_task_sequential.Validation(), true);
  test_task_sequential.PreProcessing();
  test_task_sequential.Run();
  test_task_sequential.PostProcessing();

  // Проверяем что массив отсортирован
  std::vector<int> expected = in;
  std::ranges::sort(expected);

  for (size_t i = 0; i < kCount; i++) {
    EXPECT_EQ(out[i], expected[i]);
  }
}

TEST(shishkarev_a_radix_sort_seq, test_sort_medium) {
  constexpr size_t kCount = 1000;

  std::vector<int> in(kCount);
  std::vector<int> out(kCount);

  for (size_t i = 0; i < kCount; i++) {
    in[i] = rand() % 10000;
  }

  auto task_data_seq = std::make_shared<ppc::core::TaskData>();
  task_data_seq->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_seq->inputs_count.emplace_back(in.size());
  task_data_seq->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_seq->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort::TestTaskSequential test_task_sequential(task_data_seq);

  // Правильный порядок вызовов
  ASSERT_EQ(test_task_sequential.Validation(), true);
  test_task_sequential.PreProcessing();
  test_task_sequential.Run();
  test_task_sequential.PostProcessing();

  // Проверяем что массив отсортирован
  for (size_t i = 1; i < kCount; i++) {
    EXPECT_LE(out[i - 1], out[i]);
  }
}

TEST(shishkarev_a_radix_sort_seq, test_sort_negative_numbers) {
  constexpr size_t kCount = 20;

  std::vector<int> in = {5, -3, 2, -8, 10, -1, 0, -15, 7, -4, 12, -9, 3, -6, 11, -2, 8, -13, 1, -7};
  std::vector<int> out(kCount);

  auto task_data_seq = std::make_shared<ppc::core::TaskData>();
  task_data_seq->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_seq->inputs_count.emplace_back(in.size());
  task_data_seq->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_seq->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort::TestTaskSequential test_task_sequential(task_data_seq);

  // Правильный порядок вызовов
  ASSERT_EQ(test_task_sequential.Validation(), true);
  test_task_sequential.PreProcessing();
  test_task_sequential.Run();
  test_task_sequential.PostProcessing();

  // Проверяем что массив отсортирован
  std::vector<int> expected = in;
  std::ranges::sort(expected);

  for (size_t i = 0; i < kCount; i++) {
    EXPECT_EQ(out[i], expected[i]);
  }
}

TEST(shishkarev_a_radix_sort_seq, test_sort_from_file) {
  // Создаем тестовый файл
  std::ofstream test_file("test_radix.txt");
  test_file << "20\n";
  test_file << "542\n-123\n789\n0\n42\n-56\n999\n1234\n-789\n1000\n";
  test_file << "-42\n555\n-999\n777\n-1\n888\n-100\n333\n666\n";
  test_file.close();

  std::ifstream input_file("test_radix.txt");
  size_t count = 0;
  input_file >> count;

  std::vector<int> in(count);
  std::vector<int> out(count);

  for (size_t i = 0; i < count; i++) {
    input_file >> in[i];
  }
  input_file.close();

  auto task_data_seq = std::make_shared<ppc::core::TaskData>();
  task_data_seq->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_seq->inputs_count.emplace_back(in.size());
  task_data_seq->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_seq->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort::TestTaskSequential test_task_sequential(task_data_seq);

  // Правильный порядок вызовов
  ASSERT_EQ(test_task_sequential.Validation(), true);
  test_task_sequential.PreProcessing();
  test_task_sequential.Run();
  test_task_sequential.PostProcessing();

  // Проверяем что массив отсортирован
  for (size_t i = 1; i < count; i++) {
    EXPECT_LE(out[i - 1], out[i]);
  }
}