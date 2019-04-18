#include "ringbuffer.h"
#include <stdio.h>

void dump_pck_data(struct pck_data* pd) {
	printf("in: %d:%d, out: %d:%d, proto: %d\n", pd->in_addr,pd->in_port, pd->out_addr, pd->out_port, pd->proto);
}

void dump_pck_data_kbuff(struct pck_data_kbuff *kbuff) {
	for_each_in_kbuff(kbuff, dump_pck_data);
}



int main() {
	struct pck_data_kbuff ringbuffer = make_pck_data_kbuff(3);
	dump_pck_data_kbuff(&ringbuffer);
	struct pck_data packet = {
		.in_addr = 1,
		.out_addr = 2,
		.in_port = 11,
		.out_port=22,
		.proto = 0,
	};
	add_pck_data(&ringbuffer, &packet);	
	dump_pck_data_kbuff(&ringbuffer);
	add_pck_data(&ringbuffer, &packet);	
	dump_pck_data_kbuff(&ringbuffer);
	add_pck_data(&ringbuffer, &packet);	
	dump_pck_data_kbuff(&ringbuffer);
	packet.in_addr = 6;
	packet.in_port = 66;
	add_pck_data(&ringbuffer, &packet);	
	dump_pck_data_kbuff(&ringbuffer);
}
