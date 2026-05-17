#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 

MODULE_LICENSE("Matveiproduct");
MODULE_AUTHOR("Matvei");
MODULE_DESCRIPTION("Hello World kernel");

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello matvei!\n");
    return 0; 
}

static void __exit hello_cleanup(void)
{
    printk(KERN_INFO "Goodbye matvei\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
