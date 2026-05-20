#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <net/net_namespace.h>

#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Simple netfilter IP blacklist");

#define PROC_NAME "fitl"
#define MAX_IPS 16

static struct nf_hook_ops nfho;
static __be32 blacklist[MAX_IPS];
static int ip_count = 0;

/* Проверка: есть ли IP в списке */
static int ip_is_blocked(__be32 ip)
{
    int i;

    for (i = 0; i < ip_count; i++) {
        if (blacklist[i] == ip)
            return 1;
    }

    return 0;
}

/* Netfilter hook: проверяем исходящие пакеты */
static unsigned int hook_func(void *priv,
                              struct sk_buff *skb,
                              const struct nf_hook_state *state)
{
    struct iphdr *ip_header;

    if (!skb)
        return NF_ACCEPT;

    ip_header = ip_hdr(skb);

    if (!ip_header)
        return NF_ACCEPT;

    if (ip_is_blocked(ip_header->daddr)) {
        printk(KERN_INFO "nf_blacklist: blocked packet to %pI4\n",
               &ip_header->daddr);
        return NF_DROP;
    }

    return NF_ACCEPT;
}

/* cat /proc/ip_blacklist */
static ssize_t proc_read(struct file *file,
                         char __user *buf,
                         size_t count,
                         loff_t *offp)
{
    char kbuf[512];
    int len = 0;
    int i;

    if (*offp > 0)
        return 0;

    len += scnprintf(kbuf + len, sizeof(kbuf) - len,
                     "Blocked IP addresses:\n");

    for (i = 0; i < ip_count; i++) {
        len += scnprintf(kbuf + len, sizeof(kbuf) - len,
                         "%pI4\n", &blacklist[i]);
    }

    if (copy_to_user(buf, kbuf, len))
        return -EFAULT;

    *offp = len;

    return len;
}

/*
 * echo "add 8.8.8.8" > /proc/ip_blacklist
 * echo "del 8.8.8.8" > /proc/ip_blacklist
 */
static ssize_t proc_write(struct file *file,
                          const char __user *buf,
                          size_t count,
                          loff_t *offp)
{
    char kbuf[64];
    char command[8];
    char ip_str[16];
    __be32 ip;
    int i;

    if (count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';

    if (sscanf(kbuf, "%7s %15s", command, ip_str) != 2)
        return -EINVAL;

    if (!in4_pton(ip_str, -1, (u8 *)&ip, '\0', NULL))
        return -EINVAL;

    if (strcmp(command, "add") == 0) {
        if (ip_count >= MAX_IPS)
            return -ENOMEM;

        if (!ip_is_blocked(ip)) {
            blacklist[ip_count] = ip;
            ip_count++;
            printk(KERN_INFO "nf_blacklist: added %pI4\n", &ip);
        }

        return count;
    }

    if (strcmp(command, "del") == 0) {
        for (i = 0; i < ip_count; i++) {
            if (blacklist[i] == ip) {
                blacklist[i] = blacklist[ip_count - 1];
                ip_count--;
                printk(KERN_INFO "nf_blacklist: deleted %pI4\n", &ip);
                return count;
            }
        }

        return count;
    }

    return -EINVAL;
}

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init nf_blacklist_init(void)
{
    int ret;

    proc_create(PROC_NAME, 0666, NULL, &proc_fops);

    nfho.hook = hook_func;
    nfho.hooknum = NF_INET_LOCAL_OUT;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;

    ret = nf_register_net_hook(&init_net, &nfho);

    if (ret) {
        remove_proc_entry(PROC_NAME, NULL);
        return ret;
    }

    printk(KERN_INFO "nf_blacklist: module loaded\n");

    return 0;
}

static void __exit nf_blacklist_exit(void)
{
    nf_unregister_net_hook(&init_net, &nfho);
    remove_proc_entry(PROC_NAME, NULL);

    printk(KERN_INFO "nf_blacklist: module unloaded\n");
}

module_init(nf_blacklist_init);
module_exit(nf_blacklist_exit);