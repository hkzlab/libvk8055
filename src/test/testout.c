#define _POSIX_C_SOURCE 199309L

#include "common/defs.h"
#include "k8055.h"

#include <usb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct usb_dev_handle *board_search(void) {
	board_id bid = BOARD_ADDR_0;
	struct usb_dev_handle *board = NULL;

	while (!board && bid <= BOARD_ADDR_3) {
		board = get_k8055_board_handle(bid);
		bid++;
	}

	return board;
}

int main(void) {

	usb_init();

	struct usb_dev_handle* board = board_search();
	if(!board) return -1;

	k8055_data_packet datap;
	memset(&datap, 0, sizeof(k8055_data_packet));

	struct timespec wait;

	wait.tv_sec = 0;
	wait.tv_nsec = 50000000;

	Uint16 runs = 1024;
	Sint8 digit_out = 0, direction_dig = 0, direction_an = 0;
	Uint8 anout = 0;

	while (runs--) {
		prepare_k8055_command(board, &datap, CMD_SET_AD, ENABLE_DO_PORT(0, digit_out), anout, 255 - anout);
		if (exec_k8055_command(board, &datap) < 0) break;

		fprintf(stdout, "\rK8055: [%.4u] DO 0x%.2X - AO1 0x%.2X - AO2 0x%.2X", runs, datap.data[1], datap.data[2], datap.data[3]);
		fflush(stdout);

		if (!direction_dig) 
			digit_out++;
		else 
			digit_out--;

		if (!direction_an)
			anout++;
		else
			anout--;

		if (anout == 255 || anout == 0) 
			direction_an = !direction_an;

		if (digit_out == 7 || digit_out == 0) {
			direction_dig = !direction_dig;
		}

		nanosleep(&wait, NULL);
	}

	prepare_k8055_command(board, &datap, CMD_RESET, 0, 0, 0);
	exec_k8055_command(board, &datap);

	usb_close(board);

	fprintf(stdout, "\nDone.\n");

	return -1;
}
