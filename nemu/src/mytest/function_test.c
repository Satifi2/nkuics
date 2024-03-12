#include <stdio.h>
#include <string.h>
union Data {
    int i;
    float f;
    char str[20];
};

void testunion() {
    union Data data;
    data.i = 10;
    printf("data.i: %d\n", data.i);
    data.f = 220.5;
    printf("data.f:%f\n", data.f);
    strcpy(data.str, "Test Union");
    printf("data.s:%s\n", data.str);
}

struct {
    char type;
    union {
        int intNum;
        float floatNum;
        char str[20];
    };
} aunion;

void testaunion() {
    aunion.type = 'i';
    aunion.intNum = 10;
    printf("the value of intNum is %d\n", aunion.intNum);

    aunion.type = 'f';
    aunion.floatNum = 200.5;
    printf("the value of floatNum is %f\n", aunion.floatNum);

    aunion.type = 's';
    strcpy(aunion.str, "Test Aunion");
    printf("the value of str is %s\n", aunion.str);
}

enum { RED, GREEN, BLUE };

void testenum() {
    for (int i = RED;i <= BLUE;i++) {
        printf("%d\n", i);
    }
}

struct Reg1 {
    char type;
};

struct {
    char type;
}reg2;

typedef struct {
    char type;
}Reg3;

void teststruct() {
    struct Reg1 reg1;
    reg1.type = '1';

    reg2.type = '2';

    Reg3 reg3;
    reg3.type = '3';
    printf("%c %c %c\n", reg1.type, reg2.type, reg3.type);
}

void testEndian() {
    int a = 0x12345678;
    char* p = (char*)&a;
    printf("%x %x %x %x\n", p[0], p[1], p[2], p[3]);

    union {
        int a;
        char b;
    }c;

    c.a = 1;
    if (c.b == 1) {
        printf("little endian\n");
    }
    else {
        printf("big endian\n");
    }
}

void  sscanfTest() {
    char* str = "100 200";
    int a, b;
    if (sscanf(str, "%d %d", &a, &b) == 2) {
        printf("Read numbers: %d and %d\n", a, b);
    }
    else {
        printf("Error reading numbers.\n");
    }

}

void strtokTest() {
    char str[] = "Hello, World!";
    char* token;
    char* delim = " ,!";

    token = strtok(str, delim);
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, delim);
    }
}

#include <regex.h>

void test_regex() {
    regex_t regex;
    const char *pattern = "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}";
    const char *test_string = "myemail@example.com";
    char errbuf[100];
    regmatch_t pmatch[1];

    // 编译正则表达式
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret) {
        regerror(ret, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex compilation failed: %s\n", errbuf);
        return;
    }

    // 在字符串中执行正则表达式匹配
    ret = regexec(&regex, test_string, 1, pmatch, 0);
    if (!ret) {
        printf("Match found for '%s' in '%s'\n", pattern, test_string);
    } else if (ret == REG_NOMATCH) {
        printf("No match found for '%s' in '%s'\n", pattern, test_string);
    } else {
        regerror(ret, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex match failed: %s\n", errbuf);
    }

    // 释放正则表达式
    regfree(&regex);
}