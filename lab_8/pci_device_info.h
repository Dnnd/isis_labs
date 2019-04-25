#ifdef USERSPACE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#endif

typedef struct pci_device_info {
    unsigned short bus_slot;
    unsigned int vendor_id;
    unsigned short bus_domain;
    unsigned short function_id;
    unsigned int device_id;
    unsigned int bus_number;
} pci_device_info_t;

typedef struct pci_device_info_buff {
    pci_device_info_t *data;
    size_t len;
    size_t capacity;
} pci_device_info_buff_t;

typedef void (*pci_device_info_handler)(pci_device_info_t *);

void for_each_pci_dev_in_buff(pci_device_info_buff_t *buff, pci_device_info_handler handler) {
    size_t i;
    for (i = 0; i < buff->len; ++i) {
        handler(&buff->data[i]);
    }
}

void dump_pci_info(pci_device_info_t *info) {
#ifdef USERSPACE
    printf("%hu %d:%hu.%hu device_id: %#04xd vendor_id: %#04xd\n", info->bus_domain, info->bus_number, info->bus_slot, info->function_id,
           info->device_id, info->vendor_id);
#else

    printk(KERN_INFO "%hu %d:%hu.%hu %hu %hu\n", info->bus_domain, info->bus_number, info->bus_slot, info->function_id, info->device_id, info->vendor_id);
#endif
}

void dump_pci_info_buff(pci_device_info_buff_t *buff) {
    for_each_pci_dev_in_buff(buff, &dump_pci_info);
}

pci_device_info_buff_t make_pci_device_info_buff(size_t capacity) {
    pci_device_info_buff_t buffer = {
#ifndef USERSPACE
            .data = kcalloc(capacity, sizeof(pci_device_info_t), GFP_KERNEL),
#else
            .data = calloc(capacity, sizeof(pci_device_info_t)),
#endif
            .len = 0,
            .capacity = capacity,
    };
    return buffer;
}

void swap_pci_device_info_buff(pci_device_info_buff_t *lhs, pci_device_info_buff_t *rhs) {
    pci_device_info_buff_t tmp = *rhs;
    *rhs = *lhs;
    *lhs = tmp;
}

void free_pci_device_info_buff(pci_device_info_buff_t *buff) {
#ifndef USERSPACE
    kfree(buff->data);
#else
    free(buff->data);
#endif
    buff->len = 0;
    buff->capacity = 0;
    buff->data = NULL;
}

void reserve_pci_device_info(pci_device_info_buff_t *buff, size_t capacity) {
    pci_device_info_buff_t tmp_buff;
    if (buff->capacity >= capacity) {
        return;
    }
    tmp_buff = make_pci_device_info_buff(capacity);
    memcpy(tmp_buff.data, buff->data, buff->len * sizeof(*(buff->data)));
    tmp_buff.len = buff->len;
    tmp_buff.capacity = capacity;

    swap_pci_device_info_buff(&tmp_buff, buff);
    free_pci_device_info_buff(&tmp_buff);
}

void push_pci_device_info(pci_device_info_buff_t *buff, pci_device_info_t data) {
    if (buff->capacity == buff->len) {
        reserve_pci_device_info(buff, buff->len * 2);
    }
    buff->data[buff->len] = data;
    ++buff->len;
}


