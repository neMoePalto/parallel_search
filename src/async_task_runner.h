#pragma once

#include <future>
#include <string>
#include <vector>

#include <boost/range.hpp>


namespace A {

struct substring {
  std::size_t str_num;
  std::size_t pos;
  std::string content;
};

std::ostream& operator<<(std::ostream& os, const substring& s);

bool operator==(const substring& lhs, const substring& rhs) noexcept;


struct sequence_info /*sequence - хорошее имя*/ {
  std::vector<substring> sequence;
  std::size_t            str_count = 0;
//  bool                   range_includes_at_least_one_str = false;
};


class async_task_runner {
private:
  sequence_info search_by_pattern(const std::string& str,
                                  boost::iterator_range<std::string::const_iterator> search_range,
                                  std::string pattern) const noexcept;

public:
  async_task_runner(const std::string& file_content,
                    const std::string& pattern);

  std::size_t calc_used_threads() const noexcept;
  void run_tasks(std::size_t threads_count, bool enable_prints);
  const std::vector<substring>& merge_results(bool enable_prints) noexcept;

private:
  const std::string&                      file_content_;
  const std::string&                      pattern_;
  std::vector<std::future<sequence_info>> futures_;
  std::vector<substring>                  summary_sequence_;
};


void print_results(const std::vector<substring>& res) noexcept;

}
