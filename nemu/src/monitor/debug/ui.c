#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char* line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char* args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char* args) {
  return -1;
}

static int cmd_help(char* args);

static int cmd_si(char* args) {
  // 默认步数为1
  int n = 1;
  if (args != NULL) {
    // 尝试从args中解析出步数n
    if (sscanf(args, "%d", &n) == 1) {
      if (n > 0) {
        // 如果n是正整数，则执行n步指令
        cpu_exec(n);
      }
      else {
        // 如果n不是正整数
        printf("Exception: invalid input: %s \n", args);
      }
    }
    else {
      // 如果无法解析出步数，打印错误信息
      printf("Exception: invalid input: %s \n", args);
    }
  }
  else {
    // 如果args为NULL，执行1步指令
    cpu_exec(1);
  }
  return 0;
}


static int cmd_info(char* args) {
  char* arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Exception: subcommand r or w is required.\n");
  }
  else {
    if (strcmp(arg, "r") == 0) {
      printf("eip: 0x%08X\n", cpu.eip);
      for (int i = 0; i < 8; i++) {
        printf("%s: 0x%08X\n", reg_name(i, 4), reg_l(i));
      }
    }
    else if (strcmp(arg, "w") == 0) {
      //打印监视点，暂时不实现
    }
    else {
      printf("Exception: invalid subcommand \'%s\'.\n", arg);
    }
  }
  return 0;
}

static int cmd_p(char* args) {
  // printf("cmd_p is called\n");
  if (args == NULL) {
    puts("invalid inputs\n");
    return 0;
  }
  bool success;
  uint32_t res = expr(args, &success);
  if (!res)printf("the expression is invalid%s\n", args);
  else printf("%d\n", res);

  return 0;
}

static int cmd_x(char* args) {
  uint32_t N, EXPR;
  if (sscanf(args, "%d %x", &N, &EXPR) != 2) {
    printf("Exception: invalid command format \n");
    return 0;
  }
  if (N <= 0) {
    printf("Exception: N should be bigger than 0.\n");
    return 0;
  }

  printf("%d byte(s) of memory at 0x%08X:\n", 4 * N, EXPR);
  for (uint32_t i = 0; i < N; ++i) {
    uint32_t MemContent = vaddr_read(EXPR + i * 4, 4);
    printf("0x%08X: ", EXPR + i * 4);
    for (int j = 0; j < 4; ++j) {
      printf("%02X ", (MemContent >> (j * 8)) & 0xFF);
    }
    printf("\n");
  }

  return 0;
}

static int cmd_w() {
  printf("cmd_w is called\n");
  return 0;
}

static int cmd_d() {

  return 0;
}
static struct {
  char* name;
  char* description;
  int (*handler) (char*);
} cmd_table[] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si","Pause the execution after the program steps through N instructions; if N is not provided, the default is 1.",cmd_si},
  {"info","Print the status of the registers, print the information of the watchpoints",cmd_info},
  {"p","Calculate the value of the expression EXPR",cmd_p},
  {"x","Calculate the value of the expression EXPR, use the result as the starting memory address, and output N consecutive 4-byte groups in hexadecimal form.",cmd_x},
  {"w","Pause the program execution when the value of the expression EXPR changes.",cmd_w},
  {"d","Remove the watchpoint with the sequence number N.",cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char* args) {
  /* extract the first argument */
  char* arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char* str = rl_gets();
    char* str_end = str + strlen(str);

    /* extract the first token as the command */
    char* cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char* args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
