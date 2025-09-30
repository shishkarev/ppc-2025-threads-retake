#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <vector>

#include "core/task/include/task.hpp"
#include "omp/shishkarev_a_radix_sort/include/ops_omp.hpp"

TEST(shishkarev_a_radix_sort_omp, test_sort_small) {
  constexpr size_t kCount = 10;

  std::vector<int> in(kCount);
  std::vector<int> out(kCount);

  for (size_t i = 0; i < kCount; i++) {
    in[i] = rand() % 1000;
  }

  auto task_data_omp = std::make_shared<ppc::core::TaskData>();
  task_data_omp->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_omp->inputs_count.emplace_back(in.size());
  task_data_omp->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_omp->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort_omp::TestTaskOpenMP test_task_openmp(task_data_omp);

  // Правильный порядок вызовов: Validation -> PreProcessing -> Run -> PostProcessing
  ASSERT_EQ(test_task_openmp.Validation(), true);
  test_task_openmp.PreProcessing();
  test_task_openmp.Run();
  test_task_openmp.PostProcessing();

  // Проверяем что массив отсортирован
  std::vector<int> expected = in;
  std::ranges::sort(expected);

  for (size_t i = 0; i < kCount; i++) {
    EXPECT_EQ(out[i], expected[i]);
  }
}

TEST(shishkarev_a_radix_sort_omp, test_sort_medium) {
  constexpr size_t kCount = 1000;

  std::vector<int> in(kCount);
  std::vector<int> out(kCount);

  for (size_t i = 0; i < kCount; i++) {
    in[i] = rand() % 10000;
  }

  auto task_data_omp = std::make_shared<ppc::core::TaskData>();
  task_data_omp->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_omp->inputs_count.emplace_back(in.size());
  task_data_omp->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_omp->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort_omp::TestTaskOpenMP test_task_openmp(task_data_omp);

  // Правильный порядок вызовов
  ASSERT_EQ(test_task_openmp.Validation(), true);
  test_task_openmp.PreProcessing();
  test_task_openmp.Run();
  test_task_openmp.PostProcessing();

  // Проверяем что массив отсортирован
  for (size_t i = 1; i < kCount; i++) {
    EXPECT_LE(out[i - 1], out[i]);
  }
}

TEST(shishkarev_a_radix_sort_omp, test_sort_negative_numbers) {
  constexpr size_t kCount = 20;

  std::vector<int> in = {5, -3, 2, -8, 10, -1, 0, -15, 7, -4, 12, -9, 3, -6, 11, -2, 8, -13, 1, -7};
  std::vector<int> out(kCount);

  auto task_data_omp = std::make_shared<ppc::core::TaskData>();
  task_data_omp->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_omp->inputs_count.emplace_back(in.size());
  task_data_omp->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_omp->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort_omp::TestTaskOpenMP test_task_openmp(task_data_omp);

  // Правильный порядок вызовов
  ASSERT_EQ(test_task_openmp.Validation(), true);
  test_task_openmp.PreProcessing();
  test_task_openmp.Run();
  test_task_openmp.PostProcessing();

  // Проверяем что массив отсортирован
  std::vector<int> expected = in;
  std::ranges::sort(expected);

  for (size_t i = 0; i < kCount; i++) {
    EXPECT_EQ(out[i], expected[i]);
  }
}

TEST(shishkarev_a_radix_sort_omp, test_sort_from_file) {
  // Создаем тестовый файл
  std::ofstream test_file("test_radix_omp.txt");
  test_file << "20\n";
  test_file << "542\n-123\n789\n0\n42\n-56\n999\n1234\n-789\n1000\n";
  test_file << "-42\n555\n-999\n777\n-1\n888\n-100\n333\n666\n";
  test_file.close();

  std::ifstream input_file("test_radix_omp.txt");
  size_t count = 0;
  input_file >> count;

  std::vector<int> in(count);
  std::vector<int> out(count);

  for (size_t i = 0; i < count; i++) {
    input_file >> in[i];
  }
  input_file.close();

  auto task_data_omp = std::make_shared<ppc::core::TaskData>();
  task_data_omp->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_omp->inputs_count.emplace_back(in.size());
  task_data_omp->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_omp->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort_omp::TestTaskOpenMP test_task_openmp(task_data_omp);

  // Правильный порядок вызовов
  ASSERT_EQ(test_task_openmp.Validation(), true);
  test_task_openmp.PreProcessing();
  test_task_openmp.Run();
  test_task_openmp.PostProcessing();

  // Проверяем что массив отсортирован
  for (size_t i = 1; i < count; i++) {
    EXPECT_LE(out[i - 1], out[i]);
  }
}

TEST(shishkarev_a_radix_sort_omp, test_sort_empty) {
  std::vector<int> in;
  std::vector<int> out;

  auto task_data_omp = std::make_shared<ppc::core::TaskData>();
  task_data_omp->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_omp->inputs_count.emplace_back(in.size());
  task_data_omp->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_omp->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort_omp::TestTaskOpenMP test_task_openmp(task_data_omp);

  ASSERT_EQ(test_task_openmp.Validation(), true);
  test_task_openmp.PreProcessing();
  test_task_openmp.Run();
  test_task_openmp.PostProcessing();

  // Для пустого массива просто проверяем что не упало
  EXPECT_TRUE(out.empty());
}

TEST(shishkarev_a_radix_sort_omp, test_sort_single_element) {
  std::vector<int> in = {42};
  std::vector<int> out(1);

  auto task_data_omp = std::make_shared<ppc::core::TaskData>();
  task_data_omp->inputs.emplace_back(reinterpret_cast<uint8_t*>(in.data()));
  task_data_omp->inputs_count.emplace_back(in.size());
  task_data_omp->outputs.emplace_back(reinterpret_cast<uint8_t*>(out.data()));
  task_data_omp->outputs_count.emplace_back(out.size());

  shishkarev_a_radix_sort_omp::TestTaskOpenMP test_task_openmp(task_data_omp);

  ASSERT_EQ(test_task_openmp.Validation(), true);
  test_task_openmp.PreProcessing();
  test_task_openmp.Run();
  test_task_openmp.PostProcessing();

  EXPECT_EQ(out[0], 42);
}