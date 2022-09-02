#pragma once

#include <string>


namespace A {


class command_line_args_handler {
public:
  command_line_args_handler(std::string file_path, std::string pattern);

  const std::string& file_content() const noexcept;
  const std::string& pattern() const noexcept;

private:
  void read_file(const std::string& path);

private:
  std::string file_content_;
  std::string pattern_;
};

}
