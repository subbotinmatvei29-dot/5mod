#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_USER 30
#define MAX_PAYLOAD 1024

int main(int argc, char *argv[])
{
    int sock_fd;
    struct sockaddr_nl src_addr;
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct msghdr msg;

    const char *text;


    if (argc > 1) {
        text = argv[1];
    } else {
        text = "Hello from userspace";
    }
    
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }
    //Настраиваем локальный адрес userspace-программы
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind");
        close(sock_fd);
        return 1;
    }
    //nl_pid = 0 означает что сообщение отправляется в ядро
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    nlh = malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        perror("malloc");
        close(sock_fd);
        return 1;
    }

    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strncpy((char *)NLMSG_DATA(nlh), text, MAX_PAYLOAD - 1);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    //msghdr содержит адрес получателя и данные для отправки
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Sending to kernel: %s\n", text);

    if (sendmsg(sock_fd, &msg, 0) < 0) {
        perror("sendmsg");
        free(nlh);
        close(sock_fd);
        return 1;
    }

    if (recvmsg(sock_fd, &msg, 0) < 0) {
        perror("recvmsg");
        free(nlh);
        close(sock_fd);
        return 1;
    }

    printf("Received from kernel: %s\n", (char *)NLMSG_DATA(nlh));

    free(nlh);
    close(sock_fd);

    return 0;
}