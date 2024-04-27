#include "common.h"
#include "syscall.h"

enum {
  SYS_none
};

_RegSet* do_syscall(_RegSet *r) {
  
    uintptr_t a[4],result=-1;
    a[0] = SYSCALL_ARG1(r);
    a[1] = SYSCALL_ARG2(r);
    a[2] = SYSCALL_ARG3(r);
    a[3] = SYSCALL_ARG4(r);

    switch (a[0]) {
    case SYS_none:
      printf("sys_none,%d,%d",SYS_none,SYS_exit);
        result = 1;
        break;
    case SYS_exit:
        printf("sys_eixt");
        _halt(a[1]);
        break;
    default: panic("Unhandled syscall ID = %d", a[0]);
    }

    SYSCALL_ARG1(r) = result;
    return NULL;
}
