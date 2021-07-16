#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

extern int xvar;
extern void sayHello(void);

static int __init sample_init(void)
{
	printk("Hello world.. xvar=%d",xvar);
	sayHello();
	return 0;
}

static void __exit sample_exit(void)
{
	printk("Bye, Leaving the world\n");
}

module_init(sample_init);
module_exit(sample_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arrijith");
MODULE_DESCRIPTION("Module dependency - hello world");
