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


bool operator==(const substring& lhs, const substring& rhs) noexcept {
  return lhs.str_num == rhs.str_num &&
         lhs.pos     == rhs.pos;
}


search_result async_task_runner::search_by_pattern(const std::string& str,
                                                   boost::iterator_range<std::string::const_iterator> search_range,
                                                   std::string pattern) const noexcept {
  auto it = search_range.begin();
  for (; it > str.begin(); --it) {
    if (*it == '\n') {
      break;
    }
  }

  A::search_result res;

  auto search_from = it; // TODO: it убрать, оставить только search_from
  boost::smatch what;
  std::size_t str_num = search_range.begin() == str.begin() ? 1 : 0;
  std::string::const_iterator substr_begin;
  while (boost::regex_search(search_from,
                             search_range.end(),
                             what,
                             boost::regex{pattern, boost::regex_constants::extended})) {
    if (what[0] == '\n') {
      ++str_num;
      substr_begin = what[0].begin();
    } else {
      if (what[0].end() > search_range.begin()) {
//      std::cout << substr_num << " " << what[0].begin() - substr_begin - 1 << " " << what[0] << std::endl;
        res.sequence.push_back({str_num, static_cast<size_t>(what[0].begin() - substr_begin), what[0], what[0].end()});
        if (it < search_range.begin() /*&& what[0].begin() < search_range.begin()*/) {
          res.continue_to_use_prev_sequence_str_num = true;
        }
      }
    }
    search_from = what[0].end();
  }

  res.str_count = str_num;
  if (search_range.end() == str.end()) {
    --res.str_count;
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
  const auto available_hw_threads = hw_threads != 0 ? hw_threads : 1;

  std::size_t reasonable_threads = 0;
  auto it_from = file_content_.cbegin();
  while (reasonable_threads < available_hw_threads) {
    auto it = std::find_if(it_from, file_content_.cend(), [](const auto& symb) {
      return symb == '\n';
    });
    if (it != file_content_.cend()) {
      ++reasonable_threads;
      it_from = ++it;
    } else {
      break;
    }
  }

  std::cout << "THREADS = " << reasonable_threads << std::endl;

  // Для файлов, размер которых сопоставим с размером искомой подстроки, оставляем 1 поток:
//  const auto reasonable_threads = std::max((file_content.size() / pattern_size) / 2, std::size_t{1});

//  return std::min(reasonable_threads, available_hw_threads);
  return reasonable_threads;
}


void async_task_runner::run_tasks(std::size_t used_threads) {
  futures_.clear();

  std::size_t chunk_size = file_content_.size() / used_threads;

  for (unsigned i = 0; i < used_threads; ++i) {
    const auto it_begin = file_content_.cbegin() + i * chunk_size;
    const auto it_end = (i == used_threads - 1) ? file_content_.cend() : it_begin + chunk_size;
    boost::iterator_range<std::string::const_iterator> range{it_begin, it_end};
    for (auto it = it_begin; it != it_end; ++it) {
      std::cout << *it;
    }
    std::cout << std::endl;
    auto fut = std::async(std::launch::async,
                          &async_task_runner::search_by_pattern,
                          this,
                          std::ref(file_content_),
                          range,
                          pattern_);
    futures_.emplace_back(std::move(fut));
    std::cout << "----Запустил поток " << i << ":" << std::endl;
  }
}


const std::vector<substring>& async_task_runner::merge_results() noexcept {
  summary_sequence_.clear();

  std::size_t end_to_end_numbering = 0;

  for (auto& f : futures_) {
    auto seq = f.get();
    std::cout << "-------------------------- seq = " << seq.sequence.size() << ", strings = " << seq.str_count
              << " :" << std::endl;

    for (const auto& s : seq.sequence) {
      std::cout << s << std::endl;
    }
    std::cout << std::endl;


    auto prev_sequence_str_num = end_to_end_numbering;
    std::cout << "prev_seq_str_num = " << prev_sequence_str_num << std::endl;

    for (auto& s : seq.sequence) {
      if (seq.continue_to_use_prev_sequence_str_num) {
        auto a = s.str_num;
        s.str_num += prev_sequence_str_num;
        std::cout << " 2. use_OWN_number, изм. его с " << a << " до " << s.str_num << std::endl;
      }
    }

    boost::push_back(summary_sequence_,
                     boost::iterator_range<std::vector<A::substring>::const_iterator>
                     {seq.sequence.cbegin(), seq.sequence.cend()});

    end_to_end_numbering += seq.str_count;
//    if (//seq.continue_to_use_prev_sequence_str_num ||
//        //(seq.sequence.empty() && seq.str_count > 0))
//        seq.str_count > 1)
    {
      --end_to_end_numbering;
      std::cout << "      end_to_end_numbering = " << end_to_end_numbering << std::endl;
    }
  }

  return summary_sequence_;
}


void print_results(const std::vector<substring>& res) noexcept {
  std::cout << res.size() << std::endl;
  for (const auto& s : res) {
    std::cout << s << std::endl;
  }

  if (res == std::vector<substring>{
  {5, 5 }, {6, 6}, {7, 6}}
//  {2, 5 }, {3, 6}, {4, 6}}
      ) {
    std::cout << " SUCCESS\n";
  } else {
    std::cout << " \tFAIL !\n";
  }
}

}
