#include "common.h"
#include "syscall.h"

enum {
  SYS_none_,
  SYS_open_,
  SYS_read_,
  SYS_write_,
  SYS_exit_,
  SYS_kill_,
  SYS_getpid_,
  SYS_close_,
  SYS_lseek_,
  SYS_brk_,
  SYS_fstat_,
  SYS_time_,
  SYS_signal_,
  SYS_execve_,
  SYS_fork_,
  SYS_link_,
  SYS_unlink_,
  SYS_wait_,
  SYS_times_,
  SYS_gettimeofday_
};

extern char _end;
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);

uintptr_t sys_write(int fd, const void *buf, size_t count) {
  // Log("  test how it work \n");
	uintptr_t i = 0;
	if (fd == 1 || fd == 2) {
		for(; count > 0; count--) {
			_putc(((char*)buf)[i]);
			i++;;
		}
	}
	return i;
}

_RegSet* do_syscall(_RegSet *r) {
    
    uintptr_t a[4],result=-1;
    a[0] = SYSCALL_ARG1(r);
    a[1] = SYSCALL_ARG2(r);
    a[2] = SYSCALL_ARG3(r);
    a[3] = SYSCALL_ARG4(r);
    Log("do_syscall:%d,%d,%d,%d",a[0],a[1],a[2],a[3]);

    switch (a[0]) {
    case SYS_none_:
        Log("sys_none\n");
        result = 1;
        break;
    case SYS_exit_:
        Log("sys_eixt\n");
        _halt(a[1]);
        break;
    case SYS_write_: 
        Log("sys_write\n");
        // result = sys_write(a[1], (void *)a[2], a[3]);
        result = fs_write(a[1], (void *)a[2], a[3]);
        break;
    case SYS_brk_:
        Log("sys_brk\n");
        result = 0;
        break;
    case SYS_open_:
        result = fs_open((char *)a[1], a[2], a[3]);
        break;
    case SYS_read_:
        result = fs_read(a[1], (void *)a[2], a[3]);
        break;
    case SYS_lseek_:
        result = fs_lseek(a[1], a[2], a[3]);
        break;
    case SYS_close_:
		result = fs_close(a[1]);
		break;

    default: panic("Unhandled syscall ID = %d", a[0]);
    }

    SYSCALL_ARG1(r) = result;
    return NULL;
}
