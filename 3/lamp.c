#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>
#include <linux/vt_kern.h>

static struct kobject *example_kobject;
static int test = 0;

static struct timer_list my_timer;
static struct tty_driver *my_driver;
static int led_status = 0;

#define BLINK_DELAY   HZ/5
#define RESTORE_LEDS  0xFF

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
        return sprintf(buf, "%d\n", test);
}

static void my_timer_func(struct timer_list *ptr)
{
        if (test == 0)
                return;

        if (!vc_cons[fg_console].d)
                return;

        if (!vc_cons[fg_console].d->port.tty)
                return;

        if (!my_driver)
                return;

        if (!my_driver->ops)
                return;

        if (!my_driver->ops->ioctl)
                return;

        if (led_status == test)
                led_status = 0;
        else
                led_status = test;

        (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty,
                                KDSETLED,
                                led_status);

        mod_timer(&my_timer, jiffies + BLINK_DELAY);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
                      const char *buf, size_t count)
{
        int value;

        sscanf(buf, "%d", &value);

        if (value < 0 || value > 7)
                return -EINVAL;

        test = value;

        if (test == 0) {
                del_timer_sync(&my_timer);

                if (my_driver && my_driver->ops && my_driver->ops->ioctl &&
                    vc_cons[fg_console].d &&
                    vc_cons[fg_console].d->port.tty) {

                        (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty,
                                                KDSETLED,
                                                RESTORE_LEDS);
                }
        } else {
                led_status = 0;
                mod_timer(&my_timer, jiffies + BLINK_DELAY);
        }

        return count;
}

static struct kobj_attribute foo_attribute = __ATTR(test, 0644, foo_show,
                                                   foo_store);

static int __init sys_init(void)
{
        int error = 0;

        pr_info("Module initialized successfully\n");

        example_kobject = kobject_create_and_add("systest", kernel_kobj);
        if (!example_kobject)
                return -ENOMEM;

        error = sysfs_create_file(example_kobject, &foo_attribute.attr);
        if (error) {
                pr_info("failed to create the foo file in /sys/kernel/systest\n");
                kobject_put(example_kobject);
                return error;
        }

        if (!vc_cons[fg_console].d)
                return -ENODEV;

        if (!vc_cons[fg_console].d->port.tty)
                return -ENODEV;

        my_driver = vc_cons[fg_console].d->port.tty->driver;

        timer_setup(&my_timer, my_timer_func, 0);

        return error;
}

static void __exit sys_exit(void)
{
        pr_info("Module un initialized successfully\n");

        test = 0;

        del_timer_sync(&my_timer);

        if (my_driver && my_driver->ops && my_driver->ops->ioctl &&
            vc_cons[fg_console].d &&
            vc_cons[fg_console].d->port.tty) {

                (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty,
                                        KDSETLED,
                                        RESTORE_LEDS);
        }

        kobject_put(example_kobject);
}

MODULE_LICENSE("GPL");

module_init(sys_init);
module_exit(sys_exit);
