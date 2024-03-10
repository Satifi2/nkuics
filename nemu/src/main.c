int init_monitor(int, char *[]);
void ui_mainloop(int);

void testunion();
void testaunion();
void testenum();

int main(int argc, char *argv[]) {
  testunion();
  testaunion();
  testenum();

  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
