int init_monitor(int, char* []);
void ui_mainloop(int);
void sscanfTest();
void strtokTest();
void test_regex();
void powTest() ;
int main(int argc, char* argv[]) {
  powTest() ;
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
