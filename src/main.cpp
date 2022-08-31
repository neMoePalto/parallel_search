#include <iostream>

#include "async_task_runner.h"
#include "command_line_args_handler.h"

int main(int argc, char *argv[]) {
  std::size_t USED_THREADS = 0;

  if (argc != 4) {
//  if (argc != 3) {
    std::cout << "Please check the argument's number (required 2: input file and search pattern). Stop" << std::endl;
    return 0;
  }

  A::command_line_args_handler args_handler(argv[1], argv[2]);

  A::async_task_runner task_runner(args_handler.file_content(), args_handler.pattern());

//  auto used_threads = task_runner.calc_used_threads();
  USED_THREADS = std::stoul(argv[3]);
  auto used_threads = USED_THREADS;
  task_runner.run_tasks(used_threads);

  const auto& summary_sequence = task_runner.merge_results();

  print_results(summary_sequence);

  return 0;
}
