#include <linux/kernel.h>
#include <linux/module.h> 
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include "pci_device_info.h"
#include "pci_list_def.h"

static dev_t dev;
static struct cdev c_dev;
static struct class *char_class;
static pci_device_info_buff_t pci_dev_info_buffer;

static long pci_list_ioctl(struct file *f, unsigned int cmd, unsigned long arg);

static int pci_list_open(struct inode *inodep, struct file *filep);

static int pci_list_close(struct inode *inodep, struct file *filep);

static struct file_operations fops = {
                .open = pci_list_open,
                .unlocked_ioctl = pci_list_ioctl,
                .release = pci_list_close,
};

void find_devices(pci_device_info_buff_t *target_buff);

void find_devices(pci_device_info_buff_t *target_buff) {
	struct pci_dev* pdev = NULL;
	pci_device_info_t info;
	for_each_pci_dev(pdev) {
		info.bus_number = pdev->bus->number;
                info.bus_domain = pci_domain_nr(pdev->bus);
                info.vendor_id = pdev->vendor;
                info.function_id = PCI_FUNC(pdev->devfn); //  0b111 2:0
                info.device_id = pdev->device;
                info.bus_slot = PCI_SLOT(pdev->devfn);// 0b 1111 1000 7:3
                push_pci_device_info(target_buff, info);

	}
}
static long pci_list_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    pci_device_info_buff_t user_buff;
    if (cmd == GET_PCI_DEVICES_NUMBER) {
        copy_to_user( (unsigned long *) arg, &pci_dev_info_buffer.len, sizeof(pci_dev_info_buffer.len));
        return 0;
    } else if (cmd == GET_PCI_DEVICES) {
        copy_from_user(&user_buff, (pci_device_info_t *) arg,
                       sizeof(pci_device_info_buff_t));
        copy_to_user(user_buff.data, pci_dev_info_buffer.data, pci_dev_info_buffer.len * sizeof(pci_device_info_t));
        return 0;
    }
    return -EINVAL;
}

static int __init pci_list_init(void) {
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
    printk(KERN_INFO
    "PCI List: device class registered correctly\n");

    if (device_create(char_class, NULL, dev, NULL, DEVICE_NAME) ==
        NULL) { 
        class_destroy(char_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT
        "Failed to create the device\n");
        return -1;
    }
    printk(KERN_INFO
    "PCI List: device class created correctly\n");

    cdev_init(&c_dev, &fops);
    cdev_add(&c_dev, dev, 1);
    return 0;
}

static int pci_list_open(struct inode *inodep, struct file *filep) {
    pci_dev_info_buffer = make_pci_device_info_buff(8);
    find_devices(&pci_dev_info_buffer);
    return 0;
}
static int pci_list_close(struct inode *inode, struct file *file) {
    free_pci_device_info_buff(&pci_dev_info_buffer);
    return 0;
}

static void __exit pci_list_exit(void) {
    device_destroy(char_class, dev);
    free_pci_device_info_buff(&pci_dev_info_buffer);
    class_destroy(char_class);
    unregister_chrdev_region(dev, 1);
}

module_init(pci_list_init);
module_exit(pci_list_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danila Maslennikov");
MODULE_DESCRIPTION("Simple loadable kernel module");

