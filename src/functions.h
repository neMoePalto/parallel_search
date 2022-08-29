#pragma once

#include <string>
#include <vector>
#include <future>

namespace A {

struct substring {
  std::string num;
  std::size_t pos;
  std::string content;
};

std::ostream& operator<<(std::ostream& os, const substring& s);

using search_future = std::future<std::vector<A::substring>>;


class command_line_args_handler {
public:
  command_line_args_handler(std::string file_path, std::string pattern);

  const std::string& file_content() const noexcept;
  const std::string& pattern() const noexcept;

private:
  void read_file(const std::string& path, std::string& out);

private:
  std::string file_content_;
  std::string pattern_;
};


std::size_t calc_used_threads(std::size_t file_size, std::size_t pattern_size);

std::vector<substring> merge_results(std::vector<search_future>& futures) noexcept;

void print_results(const std::vector<substring>& res) noexcept;

}
