/* Milestone 0 for realme Pad (MT6768): prove fastboot-boot runs us and the LK framebuffer works. */
#include "console.h"
#define PSCI_SYSTEM_RESET 0x84000009UL
static void smc(unsigned long fn)
{
	register unsigned long x0 __asm__("x0") = fn;
	__asm__ volatile("smc #0" : "+r"(x0) :: "x1", "x2", "x3", "memory");
}
static unsigned long rsr_el(void)
{
	unsigned long v;
	__asm__ volatile("mrs %0, CurrentEL" : "=r"(v));
	return v >> 2;
}
void secondary_main(unsigned long i) { (void)i; for (;;) __asm__ volatile("wfe"); }
void exc_handler(unsigned long slot, unsigned long *regs) { (void)slot; (void)regs; for (;;) __asm__ volatile("wfe"); }
void shim_main(unsigned long dtb)
{
	*(volatile unsigned *)0x10007000UL = 0x22000000;	/* MTK TOPRGU: stop watchdog */
	con_clear(BLUE);
	con_str("PAD", "mt6768 milestone 0");
	con_hex("EL", rsr_el());
	con_hex("DTB", dtb);
	unsigned long m;
	__asm__ volatile("mrs %0, midr_el1" : "=r"(m));
	con_hex("MIDR", m);
	__asm__ volatile("mrs %0, mpidr_el1" : "=r"(m));
	con_hex("MPIDR", m);
	con_str("OK", "HANG: hold power 10s");
	/* LK may scan out buffer 1 or 2: replicate buffer 0 */
	for (int b = 1; b < 3; b++)
		for (unsigned long i = 0; i < 2016UL * 1216; i++)
			((volatile unsigned *)(FB_BASE + b * 2016UL * 4864))[i] = ((volatile unsigned *)FB_BASE)[i];
	for (;;)
		__asm__ volatile("wfe");
	smc(PSCI_SYSTEM_RESET);
	for (;;)
		__asm__ volatile("wfe");
}
