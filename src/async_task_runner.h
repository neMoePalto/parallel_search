#pragma once

#include <future>
#include <string>
#include <vector>

#include <boost/range.hpp>


namespace A {

struct substring {
   std::size_t                 str_num;
   std::size_t                 pos;
   std::string                 content;
   std::string::const_iterator end_iter;
};

std::ostream& operator<<(std::ostream& os, const substring& s);

bool operator==(const substring& lhs, const substring& rhs) noexcept;


struct search_result /*sequence*/ {
  std::vector<A::substring> sequence;
  bool                      continue_to_use_prev_sequence_str_num = false;
  std::size_t               str_count = 0;
};

using search_future = std::future<search_result>;


class async_task_runner {
private:
  search_result search_by_pattern(const std::string& str,
                                  boost::iterator_range<std::string::const_iterator> search_range,
                                  std::string pattern) const noexcept;

public:
  async_task_runner(const std::string& file_content,
                    const std::string& pattern);

  std::size_t calc_used_threads() const noexcept;
  void run_tasks(std::size_t threads_count);
  const std::vector<substring>& merge_results() noexcept;

private:
  const std::string&            file_content_;
  const std::string&            pattern_;
  std::vector<A::search_future> futures_;
  std::vector<A::substring>     summary_sequence_;
};


void print_results(const std::vector<substring>& res) noexcept;

}
