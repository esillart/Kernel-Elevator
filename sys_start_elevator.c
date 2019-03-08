#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>

int (*STUB_start_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_start_elevator);
asmlinkage int sys_start_elevator(void) {
	if (STUB_start_elevator)
		return STUB_start_elevator();
	else
		return -8;
}
