#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, // 无类型
  TK_EQ,           // 等于
  TK_ADD,          // 加号
  TK_SUB,          // 减号
  TK_MINUS,        // 负号或减法
  TK_MUL,          // 乘号
  TK_DIV,          // 除号
  TK_AND,          // 逻辑与
  TK_OR,           // 逻辑或
  TK_NOT,          // 逻辑非
  TK_NUM,          // 数字
  TK_HEX,          // 十六进制数字
  TK_REG,          // 寄存器名
  TK_SYMB,         // 符号
  TK_LPAREN,         // 左括号
  TK_RPAREN,        // 右括号
  TK_POINT,        // 点
  TK_NOTEQUAL,     // 不等号
};


static struct rule {
  char* regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},         // equal
  {"\\+", TK_ADD},         // plus
  {"-", TK_SUB},
  {"\\*", TK_MUL},
  {"/", TK_DIV},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!", TK_NOT},
  {"[0-9]+", TK_NUM},
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"\\$[a-zA-Z]+", TK_REG},
  {"[a-zA-Z_][a-zA-Z0-9_]*", TK_SYMB},
  {"\\(", TK_LPAREN},
  {"\\)", TK_RPAREN},
  {"\\.", TK_POINT},
  {"!=", TK_NOTEQUAL},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char* e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char* substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
          i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_NOTYPE:
          break;
        case TK_NUM:
          tokens[nr_token].type = rules[i].token_type;
          int num = 0;
          for (int j = position - substr_len; j < position; ++j) {
            num = num * 10 + (int)(e[j] - '0');
          }
          uint32_t* puint = (uint32_t*)tokens[nr_token++].str;
          *puint = (uint32_t)num;
          break;
        default: 					   
          tokens[nr_token].type = rules[i].token_type;
					nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char* e, bool* success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
