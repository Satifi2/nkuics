#include "common.h"


extern _RegSet* schedule(_RegSet *prev);
extern _RegSet* do_syscall(_RegSet *r);

static _RegSet* do_event(_Event e, _RegSet* r) {
  Log("do_event=%d", e.event);
  switch (e.event) {
    case _EVENT_SYSCALL: 
      return do_syscall(r);
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
