#include "type.h"	//平台无关的基本数据类型
#include "protect.h"	//保护模式下gdt ldt等数据结构
#include "proc.h"
#define _DIYOS_GLOABL_C_HERE
#include "global.h"
/**
 * 定义全局描述符
 */
struct descriptor gdt[MAX_GDT_ITEMS];
u8	gdt_ptr[6];	//全局gdt_ptr
//这里不在c语言中重新给gdt赋值了，因为按位操作太复杂了，在汇编中使用sgdt，然后memcpy
/*
={
	//base
	{0,0,0,0,0,0,0,0,0,0,0,0,0},
//	SET_DESC(0,0,0,0,0,0),					//Base
	SET_DESC(0x0000,0xFFFF,DA_CR,DA_32,DA_LIMIT_4K,DA_DPL0), //0-4G  code segment
	SET_DESC(0x0000,0xFFFF,DA_DRW,DA_32,DA_LIMIT_4K,DA_DPL0), //0-4G data segment
	SET_DESC(0xB8000,0xFFFF,DA_DRW,DA_16,DA_LIMIT_1B,DA_DPL3), //0xB8000-0xC7FFF  for gs
};	//全局描述符表
*/
/*
 *  kernel的全局变量
 */
/*
struct descriptor_table gdt_ptr={
//	0, // 对齐
	(MAX_GDT_ITEMS-1)*8 ,	//gdt长度
	gdt,		//gdt指针
} a
;
*/	//全局描述符表
//定义中断描述符表
struct gate idt[MAX_IDT_ITEMS];
u8 idt_ptr[6];
//全局线程表
struct process*  p_proc_ready;	//获得cpu时间的进程

struct process proc_table[MAX_PROCESS_NUM];	//全局线程表