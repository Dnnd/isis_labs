#include <stdio.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <zconf.h>
#include <string.h>
#include <stdlib.h>
#include "pci_device_info.h"
#include "pci_list_def.h"

#define READ_BUFFER_LENGTH 1024
static char read_buff[READ_BUFFER_LENGTH];

typedef struct pci_dev_info_ext {
    pci_device_info_t *info;
    char *vendor;
    char *device;
} pci_dev_info_ext_t;

int main() {
    int fd = open("/dev/pci_list", O_RDWR);
    size_t len = 0;
    ioctl(fd, GET_PCI_DEVICES_NUMBER, &len);
    printf("len %lu\n", len);
    pci_device_info_buff_t buff = make_pci_device_info_buff(len);
    buff.len = len;
    ioctl(fd, GET_PCI_DEVICES, &buff);
    dump_pci_info_buff(&buff);
    pci_dev_info_ext_t *ext_info = calloc(len, sizeof(pci_dev_info_ext_t));


    for (size_t i = 0; i < buff.len; ++i) {
        ext_info[i].info = &buff.data[i];
        ext_info[i].vendor = NULL;
        ext_info[i].device = NULL;
    }


    FILE *pci_ids = fopen("pci.ids", "r");
    size_t vendors_matched = 0;
    size_t devices_matched = 0;

    while (feof(pci_ids) != EOF && (vendors_matched < len || devices_matched < len)) {
        fgets(read_buff, READ_BUFFER_LENGTH, pci_ids);
        if (read_buff[0] != '\t' && vendors_matched < len) {
            long vendor = strtol(read_buff, NULL, 16);
            for (size_t i = 0; i < buff.len; ++i) {
                if (ext_info[i].info->vendor_id == vendor) {
                    ext_info[i].vendor = strdup(read_buff);
                    vendors_matched++;
                }
            }
            continue;
        }

        if (read_buff[0] == '\t' && read_buff[1] != '\t' && devices_matched < len) {
            long device = strtol(read_buff, NULL, 16);
            for (size_t i = 0; i < buff.len; ++i) {
                if (ext_info[i].info->device_id == device) {
                    ext_info[i].device = strdup(read_buff + 1);
                    devices_matched++;
                }
            }
            continue;
        }
    }

    for (size_t i = 0; i < len; ++i) {
        printf("vendor: %sdevice: %sdev_info: ", ext_info[i].vendor, ext_info[i].device);
        dump_pci_info(ext_info[i].info);
    }

    fclose(pci_ids);
    free_pci_device_info_buff(&buff);
    for (size_t i = 0; i < len; ++i) {
        free(ext_info[i].vendor);
        free(ext_info[i].device);
    }
    free(ext_info);
    close(fd);
}
