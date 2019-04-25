#define  DEVICE_NAME "pci_list"
#define  CLASS_NAME  "pci_list"

#define PCI_LIST_MAJOR 250
#define GET_PCI_DEVICES_NUMBER _IOWR(PCI_LIST_MAJOR, 1,  unsigned long *)
#define GET_PCI_DEVICES _IOW(PCI_LIST_MAJOR, 2, pci_device_info_buff_t *)


