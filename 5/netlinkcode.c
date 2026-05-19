#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <net/sock.h>

#define NETLINK_USER 30
//Указатель на netlink-сокет в ядре
static struct sock *nl_socket = NULL;

static void netlink_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int user_pid;
    int msg_size;
    int res;

    char *user_msg;
    const char *kernel_msg = "Message received by kernel";

    if (!skb) {
        printk(KERN_ERR "netlink_kmod: empty skb\n");
        return;
    }

    nlh = nlmsg_hdr(skb);
    user_msg = (char *)nlmsg_data(nlh);
    user_pid = nlh->nlmsg_pid;

    printk(KERN_INFO "netlink_kmod: message from userspace: %s\n", user_msg);
    //Считаем размер ответа вместе с завершающим символом
    msg_size = strlen(kernel_msg) + 1;

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "netlink_kmod: failed to allocate new skb\n");
        return;
    }
    //Добавляем заголовок в исходящее сообщение
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    strncpy(nlmsg_data(nlh), kernel_msg, msg_size);

    res = nlmsg_unicast(nl_socket, skb_out, user_pid);
    if (res < 0) {
        printk(KERN_ERR "netlink_kmod: failed to send message to userspace\n");
    }
}

static struct netlink_kernel_cfg cfg = {
    .groups=1,
    .input = netlink_recv_msg,
};

static int __init netlink_kmod_init(void)
{
    printk(KERN_INFO "netlink_kmod: module loaded\n");

    nl_socket = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_socket) {
        printk(KERN_ERR "netlink_kmod: failed to create netlink socket\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "netlink_kmod: netlink socket created\n");

    return 0;
}

static void __exit netlink_kmod_exit(void)
{
    printk(KERN_INFO "netlink_kmod: module unloaded\n");

    if (nl_socket) {
        netlink_kernel_release(nl_socket);
    }
}

module_init(netlink_kmod_init);
module_exit(netlink_kmod_exit);

MODULE_LICENSE("GPL");
