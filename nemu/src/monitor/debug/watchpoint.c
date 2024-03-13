#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


WP* new_wp(char *expr, int val) {
  if (free_ == NULL) {
    printf("Exception: No free watchpoints available.\n");
    return NULL;
  }
  
  WP* wp = free_; // 从空闲列表中获取一个监视点
  free_ = free_->next; // 更新空闲列表的头指针
  // 初始化监视点
  strncpy(wp->expr, expr, 512); // 复制表达式
  wp->value = val; // 设置值
  wp->next = NULL; // 新监视点将被加到链表尾部，所以next为NULL
  // 将监视点加入到活动链表中
  if (head == NULL) {
    // 如果活动链表为空，新监视点即为头节点
    head = wp;
  } else {
    // 否则，遍历到链表尾部，并将新监视点加入
    WP* last = head;
    while (last->next != NULL) {
      last = last->next;
    }
    last->next = wp;
  }
  return wp; // 返回新创建的监视点
}

void print_wp() {
  if (head == NULL) {
    printf("No watchpoints.\n");
    return;
  }
  
  printf("NO\texp\torigin_val\n");
  WP* wp = head;
  while (wp != NULL) {
    printf("%d\t%s\t%d\n", wp->NO, wp->expr,wp->value);
    wp = wp->next;
  }
}
