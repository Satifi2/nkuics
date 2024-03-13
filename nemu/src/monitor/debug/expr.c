#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "cpu/reg.h"//因为需要用字符串获取寄存器值，所以需要引入
#include <math.h>

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
  TK_POWER       // 乘方，为了证明我对这个实验比较熟悉，我多实现了一个乘方运算。
};

const char* getTokenTypeName(int type) {//为了方便输出enum对应的字符串，我还写了一个函数，可有可无
  switch (type) {
  case TK_NOTYPE: return "TK_NOTYPE";
  case TK_EQ: return "TK_EQ";
  case TK_ADD: return "TK_ADD";
  case TK_SUB: return "TK_SUB";
  case TK_MINUS: return "TK_MINUS";
  case TK_MUL: return "TK_MUL";
  case TK_DIV: return "TK_DIV";
  case TK_AND: return "TK_AND";
  case TK_OR: return "TK_OR";
  case TK_NOT: return "TK_NOT";
  case TK_NUM: return "TK_NUM";
  case TK_HEX: return "TK_HEX";
  case TK_REG: return "TK_REG";
  case TK_LPAREN: return "TK_LPAREN";
  case TK_RPAREN: return "TK_RPAREN";
  case TK_POINTER: return "TK_POINTER";
  case TK_NOTEQUAL: return "TK_NOTEQUAL";
  case TK_POWER: return "TK_POWER";
  default: return "Unknown Type";
  }
}
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
  {"\\+", TK_ADD},         // plus,同时和正则表达式的+（一个或多个）冲突，所以要加上转义字符
  {"-", TK_SUB},
  {"\\*", TK_MUL}, //因为*在正则表达式中有特殊含义（0个或者一个），所以要加上转义字符
  {"/", TK_DIV},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR}, //  因为|是或的意思，也要转义字符
  {"!", TK_NOT},
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM},
  {"\\$[a-z]+", TK_REG},// 因为$是开头和结尾的特殊含义，所以要加上转义字符
  {"\\(", TK_LPAREN}, //因为()表示优先级， 有特殊含义，所以要加上转义字符
  {"\\)", TK_RPAREN},
  {"\\^", TK_POWER}
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

        int substr_len = pmatch.rm_eo;

        //已经有更好的输出token的方式了，所以暂时不需要日志了。
        // char* substr_start = e + position;
        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //   i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
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
        break;
        default:
        tokens[nr_token].type = rules[i].token_type;
        for (int j = position - substr_len; j < position; ++j) {
          tokens[nr_token].str[j - (position - substr_len)] = e[j];
        }
        tokens[nr_token].str[substr_len] = '\0';
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

int op_priority(int type) {
  switch (type) {
  case TK_OR: return 10;
  case TK_AND: return 11;
  case TK_EQ: return 20;
  case TK_NOTEQUAL: return 20;
  case TK_ADD: return 30;
  case TK_SUB: return 30;
  case TK_MUL: return 40;
  case TK_DIV: return 40;
  case TK_POWER: return 45;
  default: return 99999; // 非运算符或不支持的运算符
  }
}

int power(int a, int b) {
  int result = 1;
  for (int i = 0; i < b; i++) {
    result *= a;
  }
  return result;
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
    if (tokens[s].type == TK_ADD) {//相当于!123
      return get_value(e, e, success);
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
  else {
    // 找到关键运算符
    int min_priority = 50; // 初始化最低优先级
    int key_op_pos = -1; // 关键运算符位置初始化
    int lparen_count = 0; // 左括号计数
    for (int i = s; i <= e; ++i) {
      if (tokens[i].type == TK_LPAREN) {
        ++lparen_count;
      }
      else if (tokens[i].type == TK_RPAREN) {
        --lparen_count;
      }
      if (lparen_count < 0) { // 如果左括号数<右括号数，报错
        *success = false;
        return 0;
      }
      if (lparen_count == 0 && op_priority(tokens[i].type) <= min_priority) {
        min_priority = op_priority(tokens[i].type);
        key_op_pos = i;
      }
    }

    if (key_op_pos == -1 ) { // 如果没有找到关键运算符，报错
      //在没有关键运算符的情况下，如果第一个token是TK_MINUS,有可能是-(1+2)这种情况，需要求出(1+2),如果可以求出，返回-(1+2)
      //类似的还有TK_POINTER和TK_ADD和TK_NOT这一种单目运算符，如果都不是，或者求不出来，报错
      if (tokens[s].type == TK_MINUS) {
        return -get_value(s + 1, e, success);
      }
      else if (tokens[s].type == TK_POINTER) {
        return vaddr_read(get_value(s + 1, e, success), 4);
      }
      else if (tokens[s].type == TK_ADD) {
        return get_value(s + 1, e, success);
      }
      else if (tokens[s].type == TK_NOT) {
        return !get_value(s + 1, e, success);
      }
      *success = false;
      return 0;
    }
    // 根据找到的关键运算符分割表达式并递归求值
    uint32_t val1 = get_value(s, key_op_pos - 1, success);
    if (!*success) return 0; 

    uint32_t val2 = get_value(key_op_pos + 1, e, success);
    if (!*success) return 0; // 检查递归调用是否成功

    switch (tokens[key_op_pos].type) {
    case TK_ADD: return val1 + val2;
    case TK_SUB: return val1 - val2;
    case TK_MUL: return val1 * val2;
    case TK_DIV:
    if (val2 == 0) { // 除数为0，报错
      *success = false;
      return 0;
    }
    return (int)val1 / (int)val2;
    case TK_AND: return val1 && val2;
    case TK_OR: return val1 || val2;
    case TK_EQ: return val1 == val2;
    case TK_NOTEQUAL: return val1 != val2;
    case TK_POWER: return power(val1, val2);
    default: // 不支持的运算符，报错
    *success = false;
    return 0;
    }
  }
  return 0;
}

uint32_t expr(char* e, bool* success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  // 去除空白token
  int j = 0;
  for (int i = 0; i < nr_token; ++i) {
    if (tokens[i].type != TK_NOTYPE) {
      tokens[j] = tokens[i];
      j++;
    }
  }
  nr_token = j; // 更新token的数量

  for (int i = 0; i < nr_token; ++i) {
    if (tokens[i].type == TK_SUB && 
    (i == 0 ||  tokens[i-1].type == TK_LPAREN  || op_priority(tokens[i - 1].type) <50 || tokens[i-1].type == TK_MINUS || tokens[i-1].type == TK_POINTER)) {
      tokens[i].type = TK_MINUS; // 标记为负号
    }
    else if (tokens[i].type == TK_MUL && 
    (i == 0 ||  tokens[i-1].type == TK_LPAREN  || op_priority(tokens[i - 1].type) <50 || tokens[i-1].type == TK_MINUS || tokens[i-1].type == TK_POINTER)) {
      tokens[i].type = TK_POINTER; // 标记为解引用
    }
  }

  //测试打印tokens
  for (int i = 0; i < nr_token; ++i) {
    const char* typeName = getTokenTypeName(tokens[i].type);
    if (tokens[i].type == TK_NUM || tokens[i].type == TK_HEX) {
      uint32_t num;
      memcpy(&num, tokens[i].str, sizeof(num));
      Log("tokens[%d].type = %s (%d), tokens[%d].str : %u\n", i, typeName, tokens[i].type, i, num);
    }
    else if (tokens[i].type == TK_REG) {
      Log("tokens[%d].type = %s (%d), tokens[%d].str : %s\n", i, typeName, tokens[i].type, i, tokens[i].str);
    }
    else {
      Log("tokens[%d].type = %s (%d), tokens[%d].str : %s\n", i, typeName, tokens[i].type, i, tokens[i].str);
    }
  }

  /* TODO: Insert codes to evaluate the expression. */
  *success = true;
  return get_value(0, nr_token - 1, success);
}
