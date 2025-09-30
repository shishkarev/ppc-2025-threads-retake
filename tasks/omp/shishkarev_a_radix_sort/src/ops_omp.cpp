#include "omp/shishkarev_a_radix_sort/include/ops_omp.hpp"

#include <omp.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <vector>

bool shishkarev_a_radix_sort_omp::TestTaskOpenMP::PreProcessingImpl() {
  // Получаем размер входных данных
  size_t input_size = task_data->inputs_count[0];
  auto* input_ptr = reinterpret_cast<int*>(task_data->inputs[0]);

  // Копируем входные данные
  input_ = std::vector<int>(input_ptr, input_ptr + input_size);

  // Инициализируем выходной массив
  size_t output_size = task_data->outputs_count[0];
  output_.resize(output_size);

  return true;
}

bool shishkarev_a_radix_sort_omp::TestTaskOpenMP::ValidationImpl() {
  // Проверяем, что количество входов и выходов совпадает
  return task_data->inputs_count.size() == 1 && task_data->outputs_count.size() == 1 &&
         task_data->inputs_count[0] == task_data->outputs_count[0];
}

bool shishkarev_a_radix_sort_omp::TestTaskOpenMP::RunImpl() {
  if (input_.empty()) {
    return true;
  }

  // Копируем входные данные в выходной массив
  output_ = input_;

  // Выполняем поразрядную сортировку
  RadixSort(output_);

  return true;
}

bool shishkarev_a_radix_sort_omp::TestTaskOpenMP::PostProcessingImpl() {
  // Копируем результат в выходной буфер
  auto* output_ptr = reinterpret_cast<int*>(task_data->outputs[0]);
  size_t copy_size = std::min(output_.size(), static_cast<size_t>(task_data->outputs_count[0]));
  if (copy_size > 0) {
    std::memcpy(output_ptr, output_.data(), copy_size * sizeof(int));
  }
  return true;
}

// Вспомогательные функции для поразрядной сортировки

int shishkarev_a_radix_sort_omp::TestTaskOpenMP::GetMax(const std::vector<int>& arr) {
  if (arr.empty()) {
    return 0;
  }

  int max_val = arr[0];

#pragma omp parallel
  {
    int local_max = max_val;

#pragma omp for nowait
    for (int i = 1; i < static_cast<int>(arr.size()); i++) {
      local_max = std::max(arr[i], local_max);
    }

#pragma omp critical
    { max_val = std::max(local_max, max_val); }
  }

  return max_val;
}

void shishkarev_a_radix_sort_omp::TestTaskOpenMP::CountSort(std::vector<int>& arr, int exp) {
  size_t n = arr.size();
  std::vector<int> output(n);
  std::vector<int> count(10, 0);

  // Подсчитываем количество вхождений каждой цифры
#pragma omp parallel for
  for (int i = 0; i < static_cast<int>(n); i++) {
    int digit = (std::abs(arr[i]) / exp) % 10;
#pragma omp atomic
    count[digit]++;
  }

  // Изменяем count[i] так, чтобы он содержал позицию этой цифры в output[]
  for (int i = 1; i < 10; i++) {
    count[i] += count[i - 1];
  }

  // Строим выходной массив
  for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
    int digit = (std::abs(arr[i]) / exp) % 10;
    output[count[digit] - 1] = arr[i];
    count[digit]--;
  }

  // Копируем отсортированный массив обратно в arr[]
#pragma omp parallel for
  for (int i = 0; i < static_cast<int>(n); i++) {
    arr[i] = output[i];
  }
}

// Четно-нечетное слияние Бэтчера (параллельная версия)
void shishkarev_a_radix_sort_omp::TestTaskOpenMP::BatcherOddEvenMerge(std::vector<int>& arr, int left, int right) {
  int n = right - left + 1;

  if (n <= 1) {
    return;
  }

  // Параллельная реализация с исправленным условием цикла
  for (int gap = n / 2; gap > 0; gap /= 2) {
    int iterations = right - left - gap + 1;
    if (iterations <= 0) {
      continue;
    }

#pragma omp parallel for
    for (int j = 0; j < iterations; j++) {
      int i = left + j;
      if (arr[i] > arr[i + gap]) {
        std::swap(arr[i], arr[i + gap]);
      }
    }
  }
}

void shishkarev_a_radix_sort_omp::TestTaskOpenMP::RadixSort(std::vector<int>& arr) {
  if (arr.size() <= 1) {
    return;
  }

  // Разделяем положительные и отрицательные числа
  std::vector<int> negative;
  std::vector<int> positive;

  // Предварительно резервируем память для оптимизации
  negative.reserve(arr.size());
  positive.reserve(arr.size());

  // Разделение чисел на положительные и отрицательные
#pragma omp parallel
  {
    std::vector<int> local_negative;
    std::vector<int> local_positive;

#pragma omp for nowait
    for (int i = 0; i < static_cast<int>(arr.size()); i++) {
      if (arr[i] < 0) {
        local_negative.push_back(-arr[i]);  // Работаем с модулями для отрицательных
      } else {
        local_positive.push_back(arr[i]);
      }
    }

#pragma omp critical
    {
      negative.insert(negative.end(), local_negative.begin(), local_negative.end());
      positive.insert(positive.end(), local_positive.begin(), local_positive.end());
    }
  }

  // Сортируем отрицательные числа (по модулю) в обратном порядке
  if (!negative.empty()) {
    int max_neg = GetMax(negative);
    for (int exp = 1; max_neg / exp > 0; exp *= 10) {
      CountSort(negative, exp);
    }
    // Разворачиваем отсортированные по модулю отрицательные числа
    std::ranges::reverse(negative);
    // Возвращаем знак минус
#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(negative.size()); i++) {
      negative[i] = -negative[i];
    }
  }

  // Сортируем положительные числа
  if (!positive.empty()) {
    int max_pos = GetMax(positive);
    for (int exp = 1; max_pos / exp > 0; exp *= 10) {
      CountSort(positive, exp);
    }
  }

  // Объединяем результаты
  size_t index = 0;
  for (int num : negative) {
    arr[index++] = num;
  }
  for (int num : positive) {
    arr[index++] = num;
  }

  // Применяем четно-нечетное слияние Бэтчера для финальной корректировки
  if (arr.size() > 1) {
    BatcherOddEvenMerge(arr, 0, static_cast<int>(arr.size()) - 1);
  }
}