#include "async_task_runner.h"

#include <iostream>
#include <thread>

#include <boost/range/algorithm_ext.hpp>
#include <boost/regex.hpp>


namespace A {


std::ostream& operator<<(std::ostream& os, const substring& s) {
  os << s.str_num << " " << s.pos << " " << s.content;
  return os;
}


// Для отладки:
bool operator==(const substring& lhs, const substring& rhs) noexcept {
  return lhs.str_num == rhs.str_num &&
         lhs.pos     == rhs.pos;
}


search_result async_task_runner::search_by_pattern(const std::string& str,
                                                   boost::iterator_range<std::string::const_iterator> search_range,
                                                   std::string pattern) const noexcept {
  auto it_from = search_range.begin();
  if (it_from != str.cbegin()) { // Для первого "куска" str it_from остаётся в начале str
    for (; it_from != search_range.end(); ++it_from) {
      if (*it_from == '\n') {
        break;
      }
    }
    ++it_from; // Найденный символ '\n' оставляем другому обработчику
  }

  search_result res;

  if (it_from == search_range.end() + 1) {
    // Потоку не досталось работы. Он проверил свой "кусок" str и не нашёл
    // ни одной новой строки, чтобы корректно начать работу. Такая ситуация
    // возникает, если маленький файл делится между большим количеством потоков.
//    std::cout << "====================================>[NO DATA FOR THIS THREAD]" << std::endl;
  } else {
    auto it_till = search_range.end();
    for (; it_till != str.cend(); ++it_till) {
      if (*it_till == '\n') {
        break;
      }
    }

    boost::smatch what;

    std::size_t str_num = 1;
    bool search_after_new_line = false;
    std::string::const_iterator substr_begin = it_from;
    while (boost::regex_search(it_from,
                               it_till,
                               what,
                               boost::regex{pattern, boost::regex_constants::extended})) {
      if (what[0] == '\n') {
        search_after_new_line = true;
        ++str_num;
        substr_begin = what[0].begin();
      } else {
        std::size_t pos = what[0].begin() - substr_begin;
        if (!search_after_new_line) {
          ++pos;
        }
        res.searched_subs.push_back({str_num, pos, what[0]});
      }
      it_from = what[0].end();
    }

    res.str_count = str_num;
  }

  return res;
}


async_task_runner::async_task_runner(const std::string& file_content,
                                     const std::string& pattern)
  : file_content_(file_content)
  , pattern_(pattern) {
}


std::size_t async_task_runner::calc_used_threads() const noexcept {
  const std::size_t hw_threads = std::thread::hardware_concurrency();
  return hw_threads != 0 ? hw_threads : 1;
}


void async_task_runner::run_tasks(std::size_t used_threads) {
  futures_.clear();

  std::size_t chunk_size = file_content_.size() / used_threads;
  for (unsigned i = 0; i < used_threads; ++i) {
    const auto it_begin = file_content_.cbegin() + i * chunk_size;
    const auto it_end = (i == used_threads - 1) ? file_content_.cend() : it_begin + chunk_size;
    boost::iterator_range<std::string::const_iterator> range{it_begin, it_end};

    auto fut = std::async(std::launch::async,
                          &async_task_runner::search_by_pattern,
                          this,
                          std::ref(file_content_),
                          range,
                          pattern_);
    futures_.emplace_back(std::move(fut));
  }
}


const std::vector<substring>& async_task_runner::merge_results() noexcept {
  summary_sequence_.clear();
  std::size_t end_to_end_numbering = 0;

  for (auto& f : futures_) {
    auto task_res = f.get();
    for (auto& s : task_res.searched_subs) {
      s.str_num += end_to_end_numbering;
    }

    boost::push_back(summary_sequence_,
                     boost::iterator_range<std::vector<substring>::const_iterator>
                     {task_res.searched_subs.cbegin(), task_res.searched_subs.cend()});

    end_to_end_numbering += task_res.str_count;
  }

  return summary_sequence_;
}


void print_results(const std::vector<substring>& res) noexcept {
  std::cout << res.size() << std::endl;
  for (const auto& s : res) {
    std::cout << s << std::endl;
  }
}

}
