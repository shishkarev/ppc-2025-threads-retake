#include "seq/shishkarev_a_radix_sort/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

bool shishkarev_a_radix_sort::TestTaskSequential::PreProcessingImpl() {
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

bool shishkarev_a_radix_sort::TestTaskSequential::ValidationImpl() {
  // Проверяем, что количество входов и выходов совпадает
  return task_data->inputs_count.size() == 1 && task_data->outputs_count.size() == 1 &&
         task_data->inputs_count[0] == task_data->outputs_count[0];
}

bool shishkarev_a_radix_sort::TestTaskSequential::RunImpl() {
  if (input_.empty()) {
    return true;
  }

  // Копируем входные данные в выходной массив
  output_ = input_;

  // Выполняем поразрядную сортировку
  RadixSort(output_);

  return true;
}

bool shishkarev_a_radix_sort::TestTaskSequential::PostProcessingImpl() {
  // Копируем результат в выходной буфер
  auto* output_ptr = reinterpret_cast<int*>(task_data->outputs[0]);
  for (size_t i = 0; i < output_.size(); ++i) {
    output_ptr[i] = output_[i];
  }

  return true;
}

// Вспомогательные функции для поразрядной сортировки

int shishkarev_a_radix_sort::TestTaskSequential::GetMax(const std::vector<int>& arr) {
  if (arr.empty()) {
    return 0;
  }

  int max_val = arr[0];
  for (size_t i = 1; i < arr.size(); i++) {
    max_val = std::max(arr[i], max_val);
  }
  return max_val;
}

void shishkarev_a_radix_sort::TestTaskSequential::CountSort(std::vector<int>& arr, int exp) {
  size_t n = arr.size();
  std::vector<int> output(n);
  std::vector<int> count(10, 0);

  // Подсчитываем количество вхождений каждой цифры
  for (int i = 0; i < static_cast<int>(n); i++) {
    int digit = (std::abs(arr[i]) / exp) % 10;
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
  for (int i = 0; i < static_cast<int>(n); i++) {
    arr[i] = output[i];
  }
}

void shishkarev_a_radix_sort::TestTaskSequential::RadixSort(std::vector<int>& arr) {
  if (arr.size() <= 1) {
    return;
  }

  // Разделяем положительные и отрицательные числа
  std::vector<int> negative;
  std::vector<int> positive;
  for (int num : arr) {
    if (num < 0) {
      negative.push_back(-num);  // Работаем с модулями для отрицательных
    } else {
      positive.push_back(num);
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
    for (size_t i = 0; i < negative.size(); i++) {
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

// Четно-нечетное слияние Бэтчера
void shishkarev_a_radix_sort::TestTaskSequential::BatcherOddEvenMerge(std::vector<int>& arr, int left, int right) {
  int n = right - left + 1;

  if (n <= 1) {
    return;
  }

  // Упрощенная реализация
  for (int gap = n / 2; gap > 0; gap /= 2) {
    for (int i = left; i + gap <= right; i++) {
      if (arr[i] > arr[i + gap]) {
        std::swap(arr[i], arr[i + gap]);
      }
    }
  }
}