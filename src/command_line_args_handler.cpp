#include "command_line_args_handler.h"

#include <algorithm>
#include <fstream>
#include <iostream>


namespace A {


command_line_args_handler::command_line_args_handler(std::string file_path,
                                                     std::string pattern)
  : pattern_{pattern} {

  if (pattern_.empty()) {
    std::cout << "Your search pattern is empty. Stop" << std::endl;
    std::exit(0);
  } else if (pattern_.size() > 100) {
    std::cout << "Your search pattern size > 100. Stop" << std::endl;
    std::exit(0);
  }

  // Экранируем симоволы '.':
  replace(pattern_, '.', "[.]");
  // Заменяем знак вопроса на выражение "любой символ, кроме \n":
  replace(pattern_, '?', "[^\n]");

  pattern_.append("|\n");

  try {
    read_file(file_path);
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


void command_line_args_handler::read_file(const std::string& path) {
  file_content_.clear();

  constexpr auto read_size = std::size_t{4096};
  auto stream = std::ifstream{path};
  stream.exceptions(std::ios_base::badbit);

  auto buf = std::string(read_size, '\0');
  while (stream.read(& buf[0], read_size)) {
    file_content_.append(buf, 0, stream.gcount());
  }
  file_content_.append(buf, 0, stream.gcount());
}


void command_line_args_handler::replace(std::string& str, char elem, const std::string& substr) const noexcept {
  std::size_t pos = 0;
  while (pos < str.size()) {
    pos = str.find(elem, pos);
    if (pos == std::string::npos) {
      break;
    }
    str.replace(pos, 1, substr);
    pos = pos + substr.size();
  }
}

}
