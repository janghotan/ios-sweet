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

/* Log into the ramoops region (0x4d010000, 0xe0000): a valid persistent_ram "DBGC" buffer at every 4K
 * boundary so whichever zone starts there shows our text in /sys/fs/pstore after the next boot. */
#define RAMOOPS 0x4d010000UL
static char lg[256]; static int ln;
static void lc(char c) { if (ln < 200) lg[ln++] = c; }
static void ls(const char *t) { while (*t) lc(*t++); }
static void lh(unsigned long v) { for (int i = 60; i >= 0; i -= 4) lc("0123456789abcdef"[(v >> i) & 15]); lc(' '); }
static void lflush(void)
{
	int n = (ln + 3) & ~3;
	for (int i = ln; i < n; i++) lg[i] = ' ';
	for (unsigned long z = RAMOOPS; z < RAMOOPS + 0xe0000; z += 0x1000) {
		volatile unsigned *p = (volatile unsigned *)z;
		p[0] = 0x43474244; p[1] = 0; p[2] = n;
		for (int i = 0; i < n / 4; i++)
			p[3 + i] = ((unsigned char)lg[4*i]) | ((unsigned char)lg[4*i+1] << 8) | ((unsigned char)lg[4*i+2] << 16) | ((unsigned)(unsigned char)lg[4*i+3] << 24);
	}
}
void secondary_main(unsigned long i) { (void)i; for (;;) __asm__ volatile("wfe"); }
void exc_handler(unsigned long slot, unsigned long *regs) { (void)slot; (void)regs; for (;;) __asm__ volatile("wfe"); }
void shim_main(unsigned long dtb)
{
	unsigned long m1, m2;
	__asm__ volatile("mrs %0, midr_el1" : "=r"(m1));
	__asm__ volatile("mrs %0, mpidr_el1" : "=r"(m2));
	ls("IOSSWEET shim alive EL="); lh(rsr_el()); ls("DTB="); lh(dtb); ls("MIDR="); lh(m1); ls("MPIDR="); lh(m2);
	lflush();
	*(volatile unsigned *)0x10007000UL = 0x22000000;	/* MTK TOPRGU: stop watchdog */
	ls("wdt-off "); lflush();
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
	ls("display-done hang"); lflush();
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
