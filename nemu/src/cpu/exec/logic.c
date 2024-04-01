#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t0, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t0, id_dest->width);
  rtl_set_CF(&tzero);//and不可能出现进位和溢出，因此设置标志位为0。
  rtl_set_OF(&tzero);
  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t0, &id_dest->val, &id_src->val);// id_dest->val是目的操作数的值，id_src->val是源操作数的值。
  operand_write(id_dest, &t0);// 将结果写回目的操作数。
  rtl_update_ZFSF(&t0, id_dest->width);// 更新零标志(ZF)和符号标志(SF)根据t0的结果
  rtl_set_OF(&tzero); // 清除溢出标志(OF)。位与操作不会产生溢出，所以这个标志被清除。
  rtl_set_CF(&tzero); //CF同理
  print_asm_template2(and);
}

make_EHelper(xor) {//有变动
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(xor);
}

make_EHelper(or ) {
  rtl_or(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(or );
}


make_EHelper(sar) {
  int32_t temp = id_dest->val;
  // rtl_sext(&temp, &temp, id_dest->width);//有变动
  temp = ((temp << (8 * (4 - id_dest->width))) >> (8 * (4 - id_dest->width)));
  rtl_sar(&t0, (uint32_t*)(&temp), &id_src->val);
  // unnecessary to update CF and OF in NEMU
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&t0, &id_dest->val, &id_src->val);
  // unnecessary to update CF and OF in NEMU
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t0, &id_dest->val, &id_src->val);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t0, id_dest->width);
  operand_write(id_dest, &t0);
  print_asm_template2(shr);
}

make_EHelper(rol) {
  for (int i = 0;i < id_src->val;i++)
  {
    rtl_msb(&t0, &id_dest->val, id_dest->width);
    rtl_shli(&id_dest->val, &id_dest->val, 1);
    rtl_add(&id_dest->val, &id_dest->val, &t0);
    rtl_set_CF(&t0);
  }

  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(ror) {//有变动
  uint32_t mask = (1u << id_dest->width * 8) - 1; // 创建一个掩码以确保旋转不会超出操作数的大小
  for (int i = 0; i < id_src->val; i++) {
    rtl_andi(&t0, &id_dest->val, 1); // 获取最低位
    rtl_shri(&id_dest->val, &id_dest->val, 1); // 将目标值右移一位
    rtl_shli(&t0, &t0, id_dest->width * 8 - 1); // 将之前的最低位移动到最高位的位置
    rtl_or(&id_dest->val, &id_dest->val, &t0); // 将最低位添加到结果的最高位
    id_dest->val &= mask; // 应用掩码以确保操作符大小
  }

  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  print_asm_template2(ror);
}


make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  t0 = id_dest->val;
  rtl_not(&t0);
  operand_write(id_dest, &t0);
  print_asm_template1(not);
}
