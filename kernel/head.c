#include "type.h"
#include "kernel.h"
#include "global.h"
#include "protect.h"
#include "interrupt.h"
static void reset_gdt();

static void setup_interrupt();

static void setup_tss_selector();

static void setup_ldt_selector();

static void init_8259A();

//static void init_desc(struct descriptor * p_desc, u32 base, u32 limit, u16 attribute);

static void init_idt_desc(unsigned char vector, u8 desc_type, int_handler handler, unsigned char privilege);

static void put_irq_handler(int irq_no, irq_handler handler);

static void get_boot_params();

void head()
{
	//调用之前gdt_ptr中已经加载进gdt寄存器的值了
        _disp_str("Reset GDT now ...", 4, 0,COLOR_GREEN);
	reset_gdt();
	//_disp_str("[OK]",4,40,COLOR_GREEN);
	_disp_str("setup interrupt table now ...",5,0, COLOR_GREEN);
	setup_interrupt();
	//_disp_str("[OK]",5,40,COLOR_GREEN);
	_disp_str("setup tss selector ...", 6, 0, COLOR_GREEN);
	//将tss放入gdt中
	setup_tss_selector();
	//_disp_str("[OK]",6,40,COLOR_GREEN);
	_disp_str("setup ldt selector ... ", 7, 0, COLOR_GREEN);
	//将local descriptor table 放入gdt中
	//循环最大进程数次
	setup_ldt_selector();	//设置gdt中ldt选择子
	//_disp_str("[OK]",7,40,COLOR_GREEN);
	//将boot params放入内核内存中
	get_boot_params();
}

void reset_gdt()
{
	//将loader中的gdt复制到信的gdt中
        _memcpy(&gdt,
                (void*)(*((u32*)(&gdt_ptr[2]))),
                *((u16*)(&gdt_ptr[0]))+1
        );
        u16 * p_gdt_limit = (u16*)(&gdt_ptr[0]);
        u32 * p_gdt_base = (u32*)(&gdt_ptr[2]);
        //给gdt_ptr设置新的值
        *p_gdt_limit = MAX_GDT_ITEMS * sizeof(struct descriptor) - 1;
        *p_gdt_base = (u32)&gdt;
        //*p_gdt_base =(u32)gdt;
}

void setup_interrupt()
{
	//设置idt_ptr
	u16 * p_idt_limit=(u16*)(&idt_ptr[0]);
        u32 * p_idt_base = (u32*)(&idt_ptr[2]);
        *p_idt_limit = MAX_IDT_ITEMS * sizeof(struct gate) - 1;
        *p_idt_base = (u32)&idt;
	//设置中断门
	// 全部初始化成中断门(没有陷阱门)
	init_idt_desc(INT_VECTOR_DIVIDE, DA_386IGate, _divide_error, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_DEBUG, DA_386IGate, _single_step_exception, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_NMI, DA_386IGate, _nmi, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_BREAKPOINT, DA_386IGate, _breakpoint_exception, PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_OVERFLOW, DA_386IGate, _overflow, PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_BOUNDS, DA_386IGate, _bounds_check, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_INVAL_OP, DA_386IGate, _inval_opcode, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_COPROC_NOT, DA_386IGate, _copr_not_available, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_DOUBLE_FAULT, DA_386IGate, _double_fault, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_COPROC_SEG, DA_386IGate, _copr_seg_overrun, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_INVAL_TSS, DA_386IGate, _inval_tss, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_SEG_NOT, DA_386IGate, _segment_not_present, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_STACK_FAULT, DA_386IGate, _stack_exception, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_PROTECTION, DA_386IGate, _general_protection, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_PAGE_FAULT, DA_386IGate, _page_fault, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_COPROC_ERR, DA_386IGate, _copr_error, PRIVILEGE_KERNEL);	
	//设置可编程硬件中断
        init_8259A();
	//
	init_idt_desc(INT_VECTOR_IRQ0 + 0 , DA_386IGate, _hwint00, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ0 + 1 , DA_386IGate, _hwint01, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ0 + 2 , DA_386IGate, _hwint02, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ0 + 3 , DA_386IGate, _hwint03, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ0 + 4 , DA_386IGate, _hwint04, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ0 + 5 , DA_386IGate, _hwint05, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ0 + 6 , DA_386IGate, _hwint06, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ0 + 7 , DA_386IGate, _hwint07, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 0 , DA_386IGate, _hwint08, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 1 , DA_386IGate, _hwint09, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 2 , DA_386IGate, _hwint10, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 3 , DA_386IGate, _hwint11, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 4 , DA_386IGate, _hwint12, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 5 , DA_386IGate, _hwint13, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 6 , DA_386IGate, _hwint14, PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_IRQ8 + 7 , DA_386IGate, _hwint15, PRIVILEGE_KERNEL);	
	
	//这里设置一下我们定义的irq_handler_table, 都设置成默认值
	int i;
	for(i=0;i<MAX_IRQ_HANDLER_COUNT;i++){
		irq_handler_table[i] = default_irq_handler;
	}
	//系统中断
	init_idt_desc(INT_VECTOR_SYS_CALL, DA_386IGate, _sys_call, PRIVILEGE_USER);
}

//初始化tss
void setup_tss_selector()
{
	_memset(&g_tss,0,sizeof(struct tss));
	g_tss.ss0 = SELECTOR_KERNEL_DS;
	init_desc(&gdt[INDEX_TSS], (u32)&g_tss, sizeof(g_tss)-1, DA_386TSS);
	g_tss.iobase = sizeof(g_tss);	//No IO permission bitmap
}

//设置ldt的gdt选择子
void setup_ldt_selector()
{
	int i;
	for(i=0;i<TASKS_COUNT + PROCS_COUNT;i++)
	{
		_memset(&proc_table[i], 0, sizeof(struct process));
		
		proc_table[i].ldt_sel = SELECTOR_LDT_FIRST + (i<<3);
	
		init_desc(&gdt[INDEX_LDT_FIRST + i], (u32)proc_table[i].ldts, MAX_LDT_ITEMS * sizeof(struct descriptor) -1 , DA_LDT);
	}
}
void init_desc(struct descriptor * p_desc, u32 base, u32 limit, u16 attribute)
{
	//设置描述符
	p_desc->limit_low	=	limit & 0x0FFFF;
	p_desc->base_low	=	base & 0x0FFFF;
	p_desc->base_mid	=	(base>>16)&0x0FF;
	p_desc->attr1		=	attribute & 0xFF;
	p_desc->limit_high_attr2=	((limit>>16) & 0x0F)|((attribute>>8)&0xF0);
	p_desc->base_high	=	(base>>24)&0x0FF;
}

void init_idt_desc(unsigned char vector, u8 desc_type, int_handler handler, unsigned char privilege)
{
	struct gate * p_gate = &idt[vector];
	u32 base =(u32)handler;
	p_gate->offset_low	= base & 0xFFFF;
	p_gate->selector	= SELECTOR_KERNEL_CS;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type|(privilege<<5);
	p_gate->offset_high	= (base>>16) & 0xFFFF;
}

/**
 * 设置8259A可编程中断
 */
void init_8259A()
{
	/* Master 8259, ICW1. */
	_out_byte(INT_M_CTL, 0x11);
	/* Slave 8259, ICW1. */
	_out_byte(INT_S_CTL, 0x11);
	/*Master 8259, ICW2 设置主8259中断入口地址为0x20*/
	_out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0);
	/*Slave 8259, ICW2 设置从8259 中断入口地址0x28*/
	_out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8);
	/* Master 8259, ICW3. IR2对应 从8259 */
	_out_byte(INT_M_CTLMASK, 0x4);
	/*Slave 8259， ICW3. 对应主8259的IR2*/
	_out_byte(INT_S_CTLMASK, 0x2);
	/* Master 8259, ICW4. */
	_out_byte(INT_M_CTLMASK,	0x1);
	/* Slave  8259, ICW4. */
	_out_byte(INT_S_CTLMASK,	0x1);
	/* Master 8259, OCW1.  */
//	_out_byte(INT_M_CTLMASK,	0xFF);	//0xFF表示主8259的中断全部被屏蔽
	_out_byte(INT_M_CTLMASK,	0xFF);	//0xFD 二进制11111101	表示1号硬件中断被打开（keyboard)
	/* Slave  8259, OCW1.  */
	_out_byte(INT_S_CTLMASK,	0xFF);
}

void get_boot_params()
{
	//int *p =(int*)0x200000; //boot params 放在内存2M处
	_memcpy((void*)&boot_params,(void*)0x200000,sizeof(boot_params));
}
