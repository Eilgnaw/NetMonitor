/* 
 * NetMonitor.c 
 * 
 *  Created on: 2018年1月19日 
 *  Author: wxl
 */  
#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <signal.h>  
#include <errno.h>  
#include <sys/types.h>  
#include <asm/types.h>  
#include <arpa/inet.h>  
#include <sys/socket.h>  
#include <linux/netlink.h>  
#include <linux/rtnetlink.h>  
#include <linux/route.h>  
#define BUFLEN 20480  
  
#define t_assert(x) { \  
    if(!(x))  {err = -__LINE__;goto error;} \  
}  
  
/*Ctrl + C 退出*/  
static volatile int keepRunning = 1;  
  
void intHandler(int dummy)  
{  
    keepRunning = 0;  
}  
  
/** 
 * 解析RTA,并存入tb 
 */  
void parse_rtattr(struct rtattr **tb, int max, struct rtattr *attr, int len)  
{  
    for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {  
        if (attr->rta_type <= max) {  
            tb[attr->rta_type] = attr;  
        }  
    }  
}  
  
/** 
 * 显示连接信息 
 * 当网卡变动的时候触发这个信息,例如插/拔网线,增/减网卡设备,启用/禁用接口等. 
 */  
void print_ifinfomsg(struct nlmsghdr *nh)  
{  
    int len;  
    struct rtattr *tb[IFLA_MAX + 1];  
    struct ifinfomsg *ifinfo;  
    bzero(tb, sizeof(tb));  
    ifinfo = NLMSG_DATA(nh);  
    len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));  
    parse_rtattr(tb, IFLA_MAX, IFLA_RTA (ifinfo), len);  
    printf("%s: %s ", (nh->nlmsg_type==RTM_NEWLINK)?"NEWLINK":"DELLINK", (ifinfo->ifi_flags & IFF_UP) ? "up" : "down");  
    if(tb[IFLA_IFNAME]) {  
        printf("%s", RTA_DATA(tb[IFLA_IFNAME]));  
    }  
    printf("\n");  
}  
  
/** 
 * 显示地址信息 
 * 当地址变动的时候触发这个信息,例如通过DHCP获取到地址后 
 */  
void print_ifaddrmsg(struct nlmsghdr *nh)  
{  
    int len;  
    struct rtattr *tb[IFA_MAX + 1];  
    struct ifaddrmsg *ifaddr;  
    char tmp[256];  
    bzero(tb, sizeof(tb));  
    ifaddr = NLMSG_DATA(nh);  
    len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifaddr));  
    parse_rtattr(tb, IFA_MAX, IFA_RTA (ifaddr), len);  
    char eth[100];
    char addre[100];
    char cmdline[100];
    printf("%s ", (nh->nlmsg_type==RTM_NEWADDR)?"NEWADDR":"DELADDR"); 
    if (nh->nlmsg_type==RTM_NEWADDR) {//只要新的 ip
        if (tb[IFA_LABEL] != NULL) { 
        sprintf(eth, "%s", RTA_DATA(tb[IFA_LABEL]));
            if (tb[IFA_ADDRESS] != NULL) {  
                inet_ntop(ifaddr->ifa_family, RTA_DATA(tb[IFA_ADDRESS]), tmp, sizeof(tmp));  
                if(strcmp(eth, "eth0") == 0 ){//我只需要监听 eth0 网卡的就行了
                    sprintf(addre, "%s",tmp);
                    sprintf(cmdline, "/opt/sendmsg.sh %s",addre); 
                    system(cmdline);  
                    printf("%s ", tmp);
                }
            }  
        }  
     } 
    
    printf("\n");  
}  
  
/** 
 * 显示路由信息 
 * 当路由变动的时候触发这个信息 
 */  
void print_rtmsg(struct nlmsghdr *nh)  
{  
    int len;  
    struct rtattr *tb[RTA_MAX + 1];  
    struct rtmsg *rt;  
    char tmp[256];  
    bzero(tb, sizeof(tb));  
    rt = NLMSG_DATA(nh);  
    len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*rt));  
    parse_rtattr(tb, RTA_MAX, RTM_RTA(rt), len);  
    printf("%s: ", (nh->nlmsg_type==RTM_NEWROUTE)?"NEWROUT":"DELROUT");  
    if (tb[RTA_DST] != NULL) {  
        inet_ntop(rt->rtm_family, RTA_DATA(tb[RTA_DST]), tmp, sizeof(tmp));  
        printf("RTA_DST %s ", tmp);  
    }  
    if (tb[RTA_SRC] != NULL) {  
        inet_ntop(rt->rtm_family, RTA_DATA(tb[RTA_SRC]), tmp, sizeof(tmp));  
        printf("RTA_SRC %s ", tmp);  
    }  
    if (tb[RTA_GATEWAY] != NULL) {  
        inet_ntop(rt->rtm_family, RTA_DATA(tb[RTA_GATEWAY]), tmp, sizeof(tmp));  
        printf("RTA_GATEWAY %s ", tmp);  
    }  
  
    printf("\n");  
}  
int main(int argc, char *argv[])  
{  
    int socket_fd;  
    int err = 0;  
    fd_set rd_set;  
    struct timeval timeout;  
    int select_r;  
    int read_r;  
    struct sockaddr_nl sa;  
    struct nlmsghdr *nh;  
  
  
    int len = BUFLEN;  
    char buff[2048];  
    signal(SIGINT, intHandler);  
  
    /*打开NetLink Socket*/  
    socket_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);  
    t_assert(socket_fd > 0);  
    t_assert(!setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len)));  
  
    /*设定接收类型并绑定Socket*/  
    bzero(&sa, sizeof(sa));  
    sa.nl_family = AF_NETLINK;  
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_IFADDR | RTMGRP_IPV6_ROUTE;  
    t_assert(!bind(socket_fd, (struct sockaddr *) &sa, sizeof(sa)));  
  
    while (keepRunning) {  
        FD_ZERO(&rd_set);  
        FD_SET(socket_fd, &rd_set);  
        timeout.tv_sec = 5;  
        timeout.tv_usec = 0;  
        select_r = select(socket_fd + 1, &rd_set, NULL, NULL, &timeout);  
        if (select_r < 0) {  
            perror("select");  
        } else if (select_r > 0) {  
            if (FD_ISSET(socket_fd, &rd_set)) {  
                read_r = read(socket_fd, buff, sizeof(buff));  
                for (nh = (struct nlmsghdr *) buff; NLMSG_OK(nh, read_r); nh = NLMSG_NEXT(nh, read_r)) {  
                    switch (nh->nlmsg_type) {  
                    default:  
                        /*收到些奇怪的信息*/  
                        printf("nh->nlmsg_type = %d\n", nh->nlmsg_type);  
                        break;  
                    case NLMSG_DONE:  
                    case NLMSG_ERROR:  
                        break;  
                    case RTM_NEWLINK:  
                    case RTM_DELLINK:  
                        print_ifinfomsg(nh);  
                        break;  
                    case RTM_NEWADDR:  
                    case RTM_DELADDR:  
                        print_ifaddrmsg(nh);  
                        break;  
                    case RTM_NEWROUTE:  
                    case RTM_DELROUTE:  
                        print_rtmsg(nh);  
                        break;  
                    }  
  
                }  
            }  
        }  
    }  
  
    close(socket_fd);  
  
error:  
    if (err < 0) {  
        printf("Error at line %d\nErrno=%d\n", -err, errno);  
    }  
    return err;  
} 