#include <stdio.h>
#include <string.h>
union Data{
  int i;
  float f;
  char str[20];
};

void testunion(){
  union Data data;
  data.i=10;
  printf("data.i: %d\n",data.i);
  data.f=220.5;
  printf("data.f:%f\n",data.f);
  strcpy(data.str,"Test Union");
  printf("data.s:%s\n",data.str);
}

struct {
    char type;
    union {
        int intNum;
        float floatNum;
        char str[20];
    };
} aunion;

void testaunion(){
    aunion.type='i';
    aunion.intNum=10;
    printf("the value of intNum is %d\n",aunion.intNum);

    aunion.type='f';
    aunion.floatNum=200.5;
    printf("the value of floatNum is %f\n",aunion.floatNum);

    aunion.type='s';
    strcpy(aunion.str,"Test Aunion");
    printf("the value of str is %s\n",aunion.str);
}

enum {RED,GREEN,BLUE};

void testenum(){
    for(int i=RED;i<=BLUE;i++){
        printf("%d\n",i);
    }
}

struct Reg1{
    char type;
    int index;
};

struct {
    char type;
    int index;
}reg2;

typedef struct{
    char type;
    int index;
}Reg3;

void teststruct(){
    struct Reg1 reg1;
    reg1.type='1';
    
    reg2.type='2';
    
    Reg3 reg3;
    reg3.type='3';
    printf("%c %c %c",reg1.type,reg2.type,reg3.type);
}