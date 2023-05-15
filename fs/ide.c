/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (1/2) */
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_ID, 4));
		temp = begin + off;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_OFFSET, 4));
		temp = 0;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION, 4));
		panic_on(syscall_read_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_STATUS, 4));
		panic_on(syscall_read_dev((void *)(dst + off), DEV_DISK_ADDRESS | DEV_DISK_BUFFER,
					  DEV_DISK_BUFFER_LEN));
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
		panic_on(syscall_write_dev((void *)(src + off), DEV_DISK_ADDRESS | DEV_DISK_BUFFER,
					   DEV_DISK_BUFFER_LEN));
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_ID, 4));
		temp = begin + off;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_OFFSET, 4));
		temp = 1;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION, 4));
		panic_on(syscall_read_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_STATUS, 4));
	}
}

struct {
	int logic;
	int num;
	int flag;
} ssd[32];
void ssd_init() {
	int i;
	for (i = 0; i < 32; i++) {
		ssd[i].logic = -1;
		ssd[i].num = 0;
		ssd[i].flag = 0;
	}
}

int ssd_read(u_int logic_no, void *dst) {
	int i;
	for (i = 0; i < 32; i++) {
		if (ssd[i].logic == logic_no) {
			ide_read(0, i, dst, 1);
			return 0;
		}
	}
	return -1;
}

void ssd_write(u_int logic_no, void *src) {
	int i;
	for (i = 0; i < 32; i++) {
		if (ssd[i].logic == logic_no) {
			break;
		}
	}
	if (i == 32) {
		int a = w(src);
		ssd[a].logic = logic_no;
	} else {
		ssd_erase(logic_no);
		int a = w(src);
		ssd[a].logic = logic_no;
	}
}

int w(void *src) {
	int j;
	int min = 1000000;
	int m;
	for (j = 0; j < 32; j++) {
		if (ssd[j].flag == 0 && ssd[j].num < min) {
			min = ssd[j].num;
			m = j;
		}
	}
	if (min < 5) {
		ide_write(0, m, src, 1);
		ssd[m].flag = 1;
		return m;
	} else {
		min = 1000000;
		int n;
		for (j = 0; j < 32; j++) {
			if (ssd[j].flag == 1 && ssd[j].num < min) {
				min = ssd[j].num;
				n = j;
			}
		}
		u_int temp[520];
		ide_read(0, n, temp, 1);
		ide_write(0, m, temp, 1);
		ssd[m].flag = 1;
		int a = ssd[n].logic;
		ssd_erase(ssd[n].logic);
		ssd[m].logic = a;
		ide_write(0, n, src, 1);
		ssd[n].flag = 1;
		return n;
	}
}

void ssd_erase(u_int logic_no) {
	int i;
	for (i = 0; i < 32; i++) {
		if (ssd[i].logic == logic_no) {
			break;
		}
	}
	if (i == 32)
		return;
	ssd[i].logic = -1;
	ssd[i].flag = 0;
	ssd[i].num++;
	u_int zero[520] = {0};
	ide_write(0, i, (void *)zero, 1);
}
