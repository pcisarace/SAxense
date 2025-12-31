// SixAxis Sense

/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>

#define REPORT_SIZE	141 // this is the size not accounting for the report id
#define REPORT_ID	0x32
	// 0x31 is input/output report (don't use)
	// 0x32 is hd haptics
	// 0x37 ????
#define SAMPLE_SIZE	64
#define SAMPLE_RATE	3000

uint32_t crc32(const uint8_t* data, size_t size) {
	uint32_t crc = ~0xEADA2D49;  // 0xA2 seed
	                             //https://github.com/torvalds/linux/blob/b236920731dd90c3fba8c227aa0c4dee5351a639/drivers/hid/hid-playstation.c#L76

	while (size--) {
		crc ^= *data++;
		for (unsigned i = 0; i < 8; i++)
			crc = ((crc >> 1) ^ (0xEDB88320 & -(crc & 1)));
	}

	return ~crc;
}

typedef struct __attribute__((packed)) packet {
	uint8_t pid :6;
	bool unk :1,
	     sized :1;
	uint8_t length;
	uint8_t data[];
} packet_t;

struct __attribute__((packed)) report {
	uint8_t report_id;
	union {
		struct __attribute__((packed)) {
			uint8_t tag :4,
			        seq :4;
			uint8_t data[];
		};
		struct __attribute__((packed)) {
			uint8_t payload[REPORT_SIZE - sizeof(uint32_t)];
			uint32_t crc;
		};
	};
} *report;

uint8_t *sample, *ii;
char *print_buff;

static void proc(int) {// write data
	fwrite_unlocked(report, sizeof(*report), 1, stdout);


	if (!fread_unlocked(sample, sizeof(*sample), SAMPLE_SIZE, stdin)) exit(0);

	(*ii)++;
	report->crc = crc32((void*)report, 1+sizeof(report->payload));
}

int main() {
	/*
	struct packet tmp = {
		.pid = 0x11,
		.unk = 1,
		.sized = 1,
	};

	fprintf(stderr,"hi %x \n",tmp);
	return 0;
	*/
	print_buff = (char*)malloc(1024 * sizeof(char));
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	mlockall(MCL_CURRENT | MCL_FUTURE);// buffer initialization

	static const packet_t packet_0x11 = {
		.pid = 0x11, //these make up the third byte
		.sized = true,
		.length = 7,  // this is the fourth byte
		.data = {0b11111110, 0, 0, 0, 0, 0xFF, 00}, // this is the 5th byte to the 11th, the 11th gets incremented every packet
	}, packet_0x12 = {
		.pid = 0x12, //these form 0x92
		.sized = true,
		.length = SAMPLE_SIZE,
		.data = {[0 ... SAMPLE_SIZE-1] = 0},
	};

	report = malloc(sizeof(*report));
	report->report_id = REPORT_ID;
	report->tag = 0;

	packet_t *packets[] = {
		(void*)(report->data + 0),
		(void*)(report->data + sizeof(packet_0x11)+packet_0x11.length), // sizeof has the data be the size of its pointer
	};
	memcpy(packets[0], &packet_0x11, sizeof(packet_0x11)+packet_0x11.length);
	memcpy(packets[1], &packet_0x12, sizeof(packet_0x12));

	ii = &packets[0]->data[6]; // this is where the 11th byte is incremented
	sample = packets[1]->data;

	struct itimerspec ts = {0};
	ts.it_interval.tv_nsec = 1000000000UL*SAMPLE_SIZE/(SAMPLE_RATE*2);
	ts.it_value.tv_nsec = 1;

	timer_t timerid;
	struct sigevent se = {0};
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = SIGRTMIN;
	se.sigev_value.sival_ptr = &timerid;

	//proc(1);
	signal(SIGRTMIN, proc);
	timer_create(CLOCK_MONOTONIC, &se, &timerid);
	timer_settime(timerid, 0, &ts, NULL);
	//return 0;
	for (;;) sleep(3600);
	
}

// by Sdore, 2025
//  www.sdore.me
