#include "functions.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <thread>

#include <boost/range/algorithm_ext.hpp>


namespace A {


command_line_args_handler::command_line_args_handler(std::string file_path,
                                                     std::string pattern)
  : pattern_{pattern} {

  if (pattern_.empty()) {
    std::cout << "Your search pattern is empty. Stop" << std::endl;
    std::exit(0);
  } else if (pattern.size() > 100) {
    std::cout << "Your search pattern size > 100. Stop" << std::endl;
    std::exit(0);
  }

//    std::cout << pattern << ", и после: ";
  std::replace(pattern_.begin(), pattern_.end(), '?', '.'); // Что с пунктуац. точкой делать? Экранировать? Почитать в posix regex!
  pattern_.append("|\n");
//    std::cout << pattern << std::endl;

  try {
    read_file(file_path, file_content_);
  } catch (const std::exception& e) {
    std::cout << "Error while file reading: " << e.what() << std::endl;
    std::exit(0);
  }
}


const std::string& command_line_args_handler::file_content() const noexcept {
  return file_content_;
}


const std::string& command_line_args_handler::pattern() const noexcept {
  return pattern_;
}


std::ostream& operator<<(std::ostream& os, const substring& s) {
  os << s.num << " " << s.pos << " " << s.content;
  return os;
}


void command_line_args_handler::read_file(const std::string& path, std::string& out) {
  constexpr auto read_size = std::size_t{4096};
  auto stream = std::ifstream{path};
  stream.exceptions(std::ios_base::badbit);

//  auto out = std::string{};
  auto buf = std::string(read_size, '\0');
  while (stream.read(& buf[0], read_size)) {
    out.append(buf, 0, stream.gcount());
  }
  out.append(buf, 0, stream.gcount());
//  return out;
}


std::size_t calc_used_threads(std::size_t file_size, std::size_t pattern_size) {
  const std::size_t hw_threads = std::thread::hardware_concurrency();
  const auto available_hw_threads = hw_threads != 0 ? hw_threads : 2;
  // Для файлов, размер которых сопоставим с размером искомой подстроки, оставляем 1 поток:
  const auto reasonable_threads = std::max((file_size / pattern_size) / 2, std::size_t{1});
  return std::min(reasonable_threads, available_hw_threads);
}


std::vector<substring> merge_results(std::vector<search_future>& futures) noexcept {
  std::vector<A::substring> summary_sequence;

  for (auto& f : futures) {
    auto sequence = f.get();
//    std::cout << "--------------------------" << std::endl;
//    for (const auto& s : sequence) {
//      std::cout << s.num << " " << s.pos << " " << s.content << std::endl;
//    }

    if (!sequence.empty()) {
      auto head_it = sequence.cbegin();
      if (!summary_sequence.empty()) {
        auto tail_it = summary_sequence.crbegin();
        for (; tail_it != summary_sequence.crend() || head_it != sequence.cend(); ++tail_it, ++head_it) {
          // Каждая следующая последовательность элементов может "захватывать" часть предыдущей
          // последовательности. Т.е. каждый поток начинает анализ с того, что ищет начало
          // первой строки "своего куска" файла. Из-за этого возникает дублирование элементов.
          // Сдвигаем итератор вправо, чтобы пропустить дублирующиеся элементы:
          if (tail_it->num != head_it->num) {
            break;
          } else {
            std::cout << "Удаляю дубликат для элем: " << *head_it << std::endl;
          }
        }
      }

      boost::push_back(summary_sequence,
                       boost::iterator_range<std::vector<A::substring>::const_iterator>{head_it, sequence.cend()});
    }
  }

  return summary_sequence;
}


void print_results(const std::vector<substring>& res) noexcept {
  std::cout << res.size() << std::endl;
  for (const auto& s : res) {
    std::cout << s << std::endl;
  }
}

}
