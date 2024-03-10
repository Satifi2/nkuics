int init_monitor(int, char *[]);
void ui_mainloop(int);


#include <stdio.h>
#include <string.h>
union Data{
  int i;
  float f;
  char str[20];
};

void testunion(){
  union Data data;
  data.i=10;
  printf("data.i: %d\n",data.i);
  data.f=220.5;
  printf("data.f:%f\n",data.f);
  strcpy(data.str,"Test Union");
  printf("data.s:%s\n",data.str);
}

int main(int argc, char *argv[]) {
  testunion();

  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
