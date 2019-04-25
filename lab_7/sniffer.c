#include <linux/kernel.h>
#include <linux/module.h> 
#include <linux/printk.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include "ringbuffer.h"

#define  DEVICE_NAME "sniffer"
#define  CLASS_NAME  "sniffer"
#define STRBUFF_LEN 52

static struct nf_hook_ops nfho;

struct pck_data_kbuff pck_buff;
EXPORT_SYMBOL(pck_buff);
static char strbuff[STRBUFF_LEN];

static dev_t dev;
static struct cdev c_dev;

static struct class *char_class;

static ssize_t sniffer_read(struct file *filp,
                        char *buffer, 
                        size_t length, 
                        loff_t *offset);

static int sniffer_open(struct inode *inodep, struct file *filep);

static struct file_operations fops =
        {
                .read = sniffer_read,
                .open = sniffer_open,
        };

void dump_pck_data(struct pck_data *pd) {
    printk(KERN_INFO
    "in: %u:%u, out: %u:%u, proto: %u\n", pd->in_addr, pd->in_port, pd->out_addr, pd->out_port, pd->proto);
}

void dump_pck_data_kbuff(struct pck_data_kbuff *kbuff) {
    for_each_in_kbuff(kbuff, dump_pck_data);
}

unsigned int
hook_func(
        void *priv,
        struct sk_buff *skb,
        const struct nf_hook_state *state
) {
    struct iphdr *ip_header = ip_hdr(skb);
    struct tcphdr *tcp_header;
    struct pck_data pkg;
    struct udphdr *udp_header;
    pkg.proto = ip_header->protocol;
    if (pkg.proto == IPPROTO_TCP) {
        tcp_header = tcp_hdr(skb);
        pkg.in_addr = ip_header->saddr;
        pkg.in_port = tcp_header->source;
        pkg.out_addr = ip_header->daddr;
        pkg.out_port = tcp_header->dest;
        add_pck_data(&pck_buff, &pkg);
    } else if (pkg.proto == IPPROTO_UDP) {
        udp_header = udp_hdr(skb);
        pkg.in_addr = ip_header->saddr;
        pkg.in_port = udp_header->source;
        pkg.out_addr = ip_header->daddr;
        pkg.out_port = udp_header->dest;
        add_pck_data(&pck_buff, &pkg);
    }

    return NF_ACCEPT;
}

void format_pkg(struct pck_data *pkg, char *buff, char *format) {
    uint32_t addr;
    uint8_t in_high, in_prehigh, in_prelow, in_low, out_high, out_prehigh, out_prelow, out_low;

    addr = pkg->in_addr;
    in_high = addr & 0xff;
    in_prehigh = 0xff & (addr >> 8);
    in_prelow = 0xff & (addr >> 16);
    in_low = 0xff & (addr >> 24);

    addr = pkg->out_addr;
    out_high = addr & 0xff;
    out_prehigh = 0xff & (addr >> 8);
    out_prelow = 0xff & (addr >> 16);
    out_low = 0xff & (addr >> 24);
    snprintf(buff,
             STRBUFF_LEN,
             format,
             in_high, in_prehigh, in_prelow, in_low, pkg->in_port,
             out_high, out_prehigh, out_prelow, out_low, pkg->out_port);
}

//TCP xxx.xxx.xxx.xxx:xxxxx -> xxx.xxx.xxx.xxx:xxxxx\n
void format_tcp(struct pck_data *pkg, char *buff) {
    format_pkg(pkg, buff, "TCP %03u.%03u.%03u.%03u:%05u -> %03u.%03u.%03u.%03u:%05u\n");
}

//UDP xxx.xxx.xxx.xxx:xxxxx -> xxx.xxx.xxx.xxx:xxxxx\n
void format_udp(struct pck_data *pkg, char *buff) {
    format_pkg(pkg, buff, "UDP %03u.%03u.%03u.%03u:%05u -> %03u.%03u.%03u.%03u:%05u\n");
}

void format_ip_pkg(struct pck_data *pkg, char *buff) {
    if (pkg->proto == IPPROTO_TCP) {
        format_tcp(pkg, buff);
    } else if (pkg->proto == IPPROTO_UDP) {
        format_udp(pkg, buff);
    }
}

static ssize_t sniffer_read(struct file *filp,
                        char *buffer,
                        size_t length,
                        loff_t *offset) {
    if (pck_buff.is_empty) {
        char ch = '\0';
        copy_to_user(buffer, &ch, 1);
        return 1;
    }
    format_ip_pkg(&pck_buff.buff[pck_buff.begin], strbuff);
    pop_pck_data(&pck_buff);
    if (copy_to_user(buffer, &strbuff, strlen(strbuff)))
        return -EFAULT;
    return STRBUFF_LEN;
}


static int __init sniffer_init(void) {
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret) {
        return ret;
    }

    char_class = class_create(THIS_MODULE, CLASS_NAME);
    if (!char_class) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT
        "Failed to register device class\n");
        return -1;
    }
    printk(KERN_INFO "Sniffer: device class registered correctly\n");

    if (device_create(char_class, NULL, dev, NULL, DEVICE_NAME) == NULL) {             
        class_destroy(char_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT
        "Failed to create the device\n");
        return -1;
    }
    printk(KERN_INFO "Sniffer: device class created correctly\n"); 

    cdev_init(&c_dev, &fops);
    cdev_add(&c_dev, dev, 1);

    nfho.hook = hook_func;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho);
    pck_buff = make_pck_data_kbuff(1024);
    return 0;
}

static int sniffer_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static void __exit sniffer_exit(void) {
    nf_unregister_net_hook(&init_net, &nfho);
    device_destroy(char_class, dev);
    class_destroy(char_class);
    unregister_chrdev_region(dev, 1);
    free_pck_data_kbuff(&pck_buff);
}

module_init(sniffer_init);
module_exit(sniffer_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danila Maslennikov");
MODULE_DESCRIPTION("Simple loadable kernel module");
