#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "cpu/reg.h"//因为需要用字符串获取寄存器值，所以需要引入

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
  TK_LPAREN,         // 左括号
  TK_RPAREN,        // 右括号
  TK_POINTER,        // 点
  TK_NOTEQUAL,     // 不等号
};


static struct rule {
  char* regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"!=", TK_NOTEQUAL},
  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},         // equal
  {"\\+", TK_ADD},         // plus
  {"-", TK_SUB},
  {"\\*", TK_MUL},
  {"/", TK_DIV},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!", TK_NOT},
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM},
  {"\\$[a-z]+", TK_REG},
  {"\\(", TK_LPAREN},
  {"\\)", TK_RPAREN},
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
        uint32_t* pointer_uint = (uint32_t*)tokens[nr_token].str;
        *pointer_uint = (uint32_t)num;
        nr_token++;
        break;
        case TK_HEX:
        tokens[nr_token].type = rules[i].token_type;
        unsigned int hex_num = 0;
        for (int j = position - substr_len + 2; j < position; ++j) {// 跳过前缀 '0x' 或 '0X'，所以从索引 2 开始
          hex_num *= 16; // 乘以 16，因为进制为 16
          if (e[j] >= '0' && e[j] <= '9') {
            hex_num += e[j] - '0';
          }
          else if (e[j] >= 'a' && e[j] <= 'f') {
            hex_num += e[j] - 'a' + 10; // 'a' 代表 10
          }
          else if (e[j] >= 'A' && e[j] <= 'F') {
            hex_num += e[j] - 'A' + 10; // 'A' 代表 10
          }
        }
        uint32_t* pointer_hex = (uint32_t*)tokens[nr_token].str;
        *pointer_hex = hex_num;
        nr_token++;
        case TK_REG:
        tokens[nr_token].type = rules[i].token_type;
        for (int j = position - substr_len; j < position; ++j) {
          tokens[nr_token].str[j - (position - substr_len)] = e[j];
        }
        tokens[nr_token].str[substr_len] = '\0';
        nr_token++;
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

bool check_parentheses(int s, int e) {
  if (tokens[s].type != TK_LPAREN || tokens[e].type != TK_RPAREN) {
    return false;
  }
  int brackets = 0;
  for (int i = s + 1; i <= e - 1; ++i) {
    if (tokens[i].type == TK_LPAREN) {
      brackets++;
    }
    else if (tokens[i].type == TK_RPAREN) {
      brackets--;
    }
    if (brackets < 0) {
      return false;
    }
  }
  return true;
}

uint32_t get_value(int s, int e, bool* success) {
  if (s > e || *success == false) {//相当于没有意义
    *success = false;
    return 0;
  }
  if (s == e) {//
    if (tokens[s].type == TK_NUM || tokens[s].type == TK_HEX) {//这里之前求过值了，tokens[s].str就是值
      uint32_t* num = (uint32_t*)tokens[s].str;
      return *num;
    }
    else if (tokens[s].type == TK_REG) {
      return reg_value(tokens[s].str + 1);//因为寄存器有一个$符号，所以要跳过
    }
  }
  else if (s == e - 1) {
    if (tokens[s].type == TK_NOT) {//相当于!123
      return !get_value(e, e, success);
    }
    else if (tokens[s].type == TK_MINUS) {//相当于-123
      return -get_value(e, e, success);
    }
    else if (tokens[s].type == TK_POINTER) {//相当于*(一个地址)
      return vaddr_read(get_value(e, e, success), 4);
    }
  }
  else if (check_parentheses(s, e) == true) {//相当于(expr)
    return get_value(s + 1, e - 1, success);
  }
  return 0;
}

uint32_t expr(char* e, bool* success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  //测试打印tokens
  for (int i = 0; i < nr_token; ++i) {
    if (tokens[i].type == TK_NUM || tokens[i].type == TK_HEX) {
      uint32_t num;
      memcpy(&num, tokens[i].str, sizeof(num));
      printf("tokens[%d].type = %d, tokens[%d].str = %u\n", i, tokens[i].type, i, num);
    }
    else if (tokens[i].type == TK_REG) {
      printf("tokens[%d].type = %d, tokens[%d].str = %s\n", i, tokens[i].type, i, tokens[i].str);
    }
    else {
      printf("tokens[%d].type = %d, tokens[%d].str = %s\n", i, tokens[i].type, i, tokens[i].str);
    }
  }

  /* TODO: Insert codes to evaluate the expression. */
  *success = true;
  return get_value(0, nr_token - 1, success);
}
