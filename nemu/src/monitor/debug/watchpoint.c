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

void free_wp(int NO) {
    WP *wp = head;
    WP *prev = NULL;

    // 遍历活动监视点链表，查找指定NO的监视点
    while (wp != NULL && wp->NO != NO) {
        prev = wp;
        wp = wp->next;
    }

    // 如果没有找到，则打印错误信息并返回
    if (wp == NULL) {
        printf("Exception: Watchpoint number %d not found.\n", NO);
        return;
    }

    // 如果找到了监视点，从活动链表中移除
    if (prev != NULL) {
        prev->next = wp->next; // 将前一个监视点的next指向当前监视点的next
    } else {
        head = wp->next; // 如果要删除的是头节点，则更新头节点
    }

    // 将删除的监视点放回空闲链表
    wp->next = free_; // 将当前监视点的next指向空闲链表的头部
    free_ = wp; // 更新空闲链表的头部为当前监视点

    printf("Watchpoint number %d has been deleted.\n", NO);
}

bool check_wp() {
    bool triggered = false; // 标记是否有监视点被触发
    WP *wp = head; // 从头节点开始遍历
    while (wp != NULL) {
        bool success;
        uint32_t new_val = expr(wp->expr, &success); // 计算表达式的当前值
        if (!success) {
            printf("Error evaluating expression: %s\n", wp->expr);
        } else {
            if (wp->value != new_val) { // 比较新旧值
                printf("Watchpoint %d triggered: %s changed from %u to %u\n",
                       wp->NO, wp->expr, wp->value, new_val);
                wp->value = new_val; // 更新存储的值
                triggered = true; // 标记监视点被触发
            }
        }
        wp = wp->next; // 移动到下一个监视点
    }
    return triggered; // 返回是否有监视点被触发
}
