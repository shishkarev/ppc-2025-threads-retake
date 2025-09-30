#include "tbb/shishkarev_a_radix_sort/include/ops_tbb.hpp"

#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_reduce.h>
#include <oneapi/tbb/spin_mutex.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_invoke.h>
#include <tbb/task_arena.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <vector>

bool shishkarev_a_radix_sort_tbb::TestTaskTBB::PreProcessingImpl() {
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

bool shishkarev_a_radix_sort_tbb::TestTaskTBB::ValidationImpl() {
  // Проверяем, что количество входов и выходов совпадает
  return task_data->inputs_count.size() == 1 && task_data->outputs_count.size() == 1 &&
         task_data->inputs_count[0] == task_data->outputs_count[0];
}

bool shishkarev_a_radix_sort_tbb::TestTaskTBB::RunImpl() {
  if (input_.empty()) {
    return true;
  }

  // Копируем входные данные в выходной массив
  output_ = input_;

  // Выполняем поразрядную сортировку
  RadixSort(output_);

  return true;
}

bool shishkarev_a_radix_sort_tbb::TestTaskTBB::PostProcessingImpl() {
  // Копируем результат в выходной буфер
  auto* output_ptr = reinterpret_cast<int*>(task_data->outputs[0]);
  size_t copy_size = std::min(output_.size(), static_cast<size_t>(task_data->outputs_count[0]));
  if (copy_size > 0) {
    std::memcpy(output_ptr, output_.data(), copy_size * sizeof(int));
  }
  return true;
}

// Вспомогательные функции для поразрядной сортировки

int shishkarev_a_radix_sort_tbb::TestTaskTBB::GetMax(const std::vector<int>& arr) {
  if (arr.empty()) {
    return 0;
  }

  return tbb::parallel_reduce(
      tbb::blocked_range<size_t>(1, arr.size()), arr[0],
      [&](const tbb::blocked_range<size_t>& r, int local_max) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
          local_max = std::max(arr[i], local_max);
        }
        return local_max;
      },
      [](int x, int y) { return std::max(x, y); });
}

void shishkarev_a_radix_sort_tbb::TestTaskTBB::CountSort(std::vector<int>& arr, int exp) {
  int n = static_cast<int>(arr.size());
  std::vector<int> output(n);
  std::vector<int> count(10, 0);

  // Альтернативная реализация подсчета без атомарных операций
  tbb::parallel_for(tbb::blocked_range<int>(0, n), [&](const tbb::blocked_range<int>& r) {
    std::vector<int> local_count(10, 0);
    for (int i = r.begin(); i != r.end(); ++i) {
      int digit = (std::abs(arr[i]) / exp) % 10;
      local_count[digit]++;
    }
    // Используем мьютекс для безопасного объединения счетчиков
    static tbb::spin_mutex mutex;
    tbb::spin_mutex::scoped_lock lock(mutex);
    for (int j = 0; j < 10; j++) {
      count[j] += local_count[j];
    }
  });

  // Изменяем count[i] так, чтобы он содержал позицию этой цифры в output[]
  for (int i = 1; i < 10; i++) {
    count[i] += count[i - 1];
  }

  // Строим выходной массив
  for (int i = n - 1; i >= 0; i--) {
    int digit = (std::abs(arr[i]) / exp) % 10;
    output[count[digit] - 1] = arr[i];
    count[digit]--;
  }

  // Копируем отсортированный массив обратно в arr[]
  tbb::parallel_for(tbb::blocked_range<int>(0, n), [&](const tbb::blocked_range<int>& r) {
    for (int i = r.begin(); i != r.end(); ++i) {
      arr[i] = output[i];
    }
  });
}

// Четно-нечетное слияние Бэтчера (TBB версия)
void shishkarev_a_radix_sort_tbb::TestTaskTBB::BatcherOddEvenMerge(std::vector<int>& arr, int left, int right) {
  int n = right - left + 1;

  if (n <= 1) {
    return;
  }

  // TBB реализация
  for (int gap = n / 2; gap > 0; gap /= 2) {
    int iterations = right - left - gap + 1;
    if (iterations <= 0) {
      continue;
    }

    tbb::parallel_for(tbb::blocked_range<int>(0, iterations), [&](const tbb::blocked_range<int>& r) {
      for (int j = r.begin(); j != r.end(); ++j) {
        int i = left + j;
        if (arr[i] > arr[i + gap]) {
          std::swap(arr[i], arr[i + gap]);
        }
      }
    });
  }
}

void shishkarev_a_radix_sort_tbb::TestTaskTBB::RadixSort(std::vector<int>& arr) {
  if (arr.size() <= 1) {
    return;
  }

  // Разделяем положительные и отрицательные числа
  std::vector<int> negative;
  std::vector<int> positive;

  // Используем мьютексы для безопасного доступа к векторам
  tbb::spin_mutex neg_mutex;
  tbb::spin_mutex pos_mutex;

  // Разделение чисел на положительные и отрицательные с использованием TBB
  tbb::parallel_for(tbb::blocked_range<size_t>(0, arr.size()), [&](const tbb::blocked_range<size_t>& r) {
    std::vector<int> local_negative;
    std::vector<int> local_positive;

    for (size_t i = r.begin(); i != r.end(); ++i) {
      if (arr[i] < 0) {
        local_negative.push_back(-arr[i]);  // Работаем с модулями для отрицательных
      } else {
        local_positive.push_back(arr[i]);
      }
    }

    // Безопасно добавляем локальные результаты в общие векторы
    if (!local_negative.empty()) {
      tbb::spin_mutex::scoped_lock lock(neg_mutex);
      negative.insert(negative.end(), local_negative.begin(), local_negative.end());
    }
    if (!local_positive.empty()) {
      tbb::spin_mutex::scoped_lock lock(pos_mutex);
      positive.insert(positive.end(), local_positive.begin(), local_positive.end());
    }
  });

  // Сортируем отрицательные числа (по модулю) в обратном порядке
  if (!negative.empty()) {
    int max_neg = GetMax(negative);
    for (int exp = 1; max_neg / exp > 0; exp *= 10) {
      CountSort(negative, exp);
    }
    // Разворачиваем отсортированные по модулю отрицательные числа
    std::ranges::reverse(negative);
    // Возвращаем знак минус
    tbb::parallel_for(tbb::blocked_range<size_t>(0, negative.size()), [&](const tbb::blocked_range<size_t>& r) {
      for (size_t i = r.begin(); i != r.end(); ++i) {
        negative[i] = -negative[i];
      }
    });
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