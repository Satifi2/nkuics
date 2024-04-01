#include "cpu/exec.h"

make_EHelper(add) {
  //将两个要加的数进行符号扩展
  rtl_sext(&t3, &id_src->val, id_src->width);
  rtl_sext(&t2, &id_dest->val, id_dest->width);
  //相加
  rtl_add(&t0, &id_dest->val,  &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, 4);
  //是否有进位
  t1 = (t0 < t2);
  rtl_set_CF(&t1);
  //是否有溢出
  t1 = ((((int32_t)(t2) < 0) == ((int32_t)(t3) < 0)) && (((int32_t)(t0) < 0) != ((int32_t)(t2) < 0)));
  rtl_set_OF(&t1);
  print_asm_template2(add);
}


make_EHelper(sub) {
  //// 将源操作数和目标操作数的值符号扩展到32位,存放在t1和t2
  rtl_sext(&t1, &id_dest->val, id_dest->width);
  rtl_sext(&t2, &id_src->val, id_src->width);
  // 执行减法操作并将结果存储在t0中
  rtl_sub(&t0, &t1, &t2);
  t3 = (t0 > t1);
  // 如果发生无符号下溢，设置进位标志（CF）
  rtl_set_CF(&t3);
  // 如果结果的符号不正确，则设置溢出标志（OF）
  t3 = ((((int32_t)(t1) < 0) == (((int32_t)(t2) >> 31) == 0)) && (((int32_t)(t0) < 0) != ((int32_t)(t1) < 0)));
  // 根据结果更新零标志（ZF）和符号标志（SF）
  rtl_set_OF(&t3);
  rtl_update_ZFSF(&t0, 4);
   // 将结果写回目标操作数
  operand_write(id_dest, &t0);
  print_asm_template2(sub);
}

make_EHelper(cmp) {
  //进行符号扩展
  rtl_sext(&t2, &id_src->val, id_src->width);
  rtl_sext(&t1, &id_dest->val, id_dest->width);
  //执行减法操作
  rtl_sub(&t0, &t1, &t2);
  //更新ZF和SF
  rtl_update_ZFSF(&t0, 4);
  //判断是否有进位
  t3 = (t0 > t1);
  rtl_set_CF(&t3);
  //判断是否有溢出
  t3 = ((((int32_t)(t1) < 0) == (((int32_t)(t2) >> 31) == 0)) && (((int32_t)(t0) < 0) != ((int32_t)(t1) < 0)));
  rtl_set_OF(&t3);
  print_asm_template2(cmp);
}

make_EHelper(inc) {
  // 临时寄存器t2用于保存操作结果
  rtl_addi(&t2, &id_dest->val, 1); // t2 = id_dest->val + 1
  operand_write(id_dest, &t2); // 将结果写回目标寄存器
  
  rtl_update_ZFSF(&t2, id_dest->width); // 根据结果更新ZF和SF标志位

  // 检查溢出条件：如果id_dest是最大正数，则执行inc后会变为最小负数，产生溢出
  // OF = (id_dest == 0x7FFFFFFF) ? 1 : 0; 简化为
  // OF = (id_dest == 0x7F...F && t2 == 0x80...0)
  rtl_li(&t0, 0x7FFFFFFF >> ((4 - id_dest->width) * 8));
  rtl_eqi(&t1, &id_dest->val, t0); // t1 = (id_dest->val == 最大正数)
  rtl_msb(&t0, &t2, id_dest->width); // t0 = t2的最高位，检查是否为负数
  rtl_and(&t0, &t0, &t1); // t0 = t1 AND t0，即检查是否从最大正数变为最小负数
  rtl_set_OF(&t0); // 根据t0的值设置OF标志位

  print_asm_template1(inc);
}


make_EHelper(dec) {
  TODO();

  print_asm_template1(dec);
}

make_EHelper(neg) {
  TODO();

  print_asm_template1(neg);
}

make_EHelper(adc) {
  // 首先将源操作数（id_src->val）加到目标操作数（id_dest->val）
  rtl_add(&t2, &id_dest->val, &id_src->val);
  // 检查是否发生无符号数加法溢出，用于后续设置 CF（进位标志）
  rtl_sltu(&t3, &t2, &id_dest->val);
  // 获取当前 CF 的值，并存储到 t1 中
  rtl_get_CF(&t1);
  // 将 CF 的值加到 t2（之前加法的结果）上
  rtl_add(&t2, &t2, &t1);
  // 将最终结果写回目标操作数
  operand_write(id_dest, &t2);

  // 更新零标志（ZF）和符号标志（SF），基于 t2 的值和操作数的宽度
  rtl_update_ZFSF(&t2, id_dest->width);

  // 重新检查是否发生无符号数加法溢出，用于设置 CF
  rtl_sltu(&t0, &t2, &id_dest->val);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  // 计算溢出标志 OF，如果源操作数和目标操作数符号相同，但结果符号不同，则设置 OF
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(adc);
}

make_EHelper(sbb) {
  rtl_sub(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &id_dest->val, &t2);
  rtl_get_CF(&t1);
  rtl_sub(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sbb);
}

make_EHelper(mul) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(mul);
}

// imul with one operand
make_EHelper(imul1) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_imul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_src2->val, &id_src2->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_src2->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_li(&t1, 0);
      rtl_lr_w(&t0, R_AX);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_li(&t1, 0);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_div(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  switch (id_dest->width) {
    case 1:
      rtl_lr_w(&t0, R_AX);
      rtl_sext(&t0, &t0, 2);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_idiv(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(idiv);
}
