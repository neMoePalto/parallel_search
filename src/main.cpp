#include <algorithm>
#include <fstream>
#include <future>
#include <iostream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/regex.hpp>

#include "functions.h"


std::vector<A::substring> find_pattern(const std::string& str,
                                       boost::iterator_range<std::string::const_iterator> search_range,
                                       std::string pattern) {
  auto it = search_range.begin();
  for (; it > str.begin(); --it) {
    if (*it == '\n') {
      break;
    }
  }

  std::vector<A::substring> res;

  const std::string digit = "^[[:digit:]]*[[:digit:]]";
  std::string curr_pattern = (it == str.begin()) ? digit : pattern;
  auto search_from = it;
  boost::smatch what;

  std::string substr_num;
  std::string::const_iterator substr_begin;
  while (boost::regex_search(search_from,
                             search_range.end(),
                             what,
                             boost::regex{curr_pattern, boost::regex_constants::extended})) {
    if (what[0] == '\n') {
      curr_pattern = digit;
    } else if (curr_pattern == digit) {
      substr_num = what[0];
      substr_begin = what[0].end();
      curr_pattern = pattern;
    } else {
//      std::cout << substr_num << " " << what[0].begin() - substr_begin - 1 << " " << what[0] << std::endl;
      res.push_back({substr_num, static_cast<size_t>(what[0].begin() - substr_begin - 1), what[0]});
    }
    search_from = what[0].end();
  }

  return res;
}


int main(int argc, char *argv[]) {
  std::size_t USED_THREADS = 0;

  if (argc != 4) {
//  if (argc != 3) {
    std::cout << "Please check the argument's number (required 2: input file and search pattern). Stop" << std::endl;
    return 0;
  }

  A::command_line_args_handler args_handler(argv[1], argv[2]);
  const std::string& str = args_handler.file_content();
  const std::string& pattern = args_handler.pattern();

//  auto used_threads = A::calc_used_threads(str.size(), pattern.size());
  USED_THREADS = std::stoul(argv[3]);
  auto used_threads = USED_THREADS;

  std::size_t chunk_size = str.size() / used_threads;

  std::vector<A::search_future> futures;

  for (unsigned i = 0; i < used_threads; ++i) {
    const auto it_begin = str.begin() + i * chunk_size;
    const auto it_end = (i == used_threads - 1) ? str.end() : it_begin + chunk_size;
    boost::iterator_range<std::string::const_iterator> range{it_begin, it_end};
//    for (auto it = it_begin; it != it_end; ++it) {
//      std::cout << *it;
//    }
//    std::cout << std::endl;
    A::search_future fut = std::async(std::launch::async, find_pattern, std::ref(str), range, pattern);
    futures.emplace_back(std::move(fut));
    std::cout << "----Запустил поток " << i << ":" << std::endl;
  }

  auto summary_sequence = A::merge_results(futures);
  A::print_results(summary_sequence);

  return 0;
}
