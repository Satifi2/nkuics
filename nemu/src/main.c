int init_monitor(int, char* []);
void ui_mainloop(int);
void sscanfTest();
void strtokTest();
void test_regex();
void powTest() ;
#include <stdio.h>

#define DEFINE_PRINT_FUNCTION(type, specifier) \
  void print_##type(type value) { \
    printf(#specifier "\n", value); \
  }

// 使用宏定义来生成具体的打印函数
DEFINE_PRINT_FUNCTION(int, %d)
DEFINE_PRINT_FUNCTION(float, %f)
DEFINE_PRINT_FUNCTION(char, %c)
int main(int argc, char* argv[]) {

  print_int(123);
  print_float(456.789);
  print_char('A');

  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
