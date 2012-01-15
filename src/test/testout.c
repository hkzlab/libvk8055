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

	struct timespec wait;
	wait.tv_sec = 0;
	wait.tv_nsec = 50000000;

	// Reset the board
	prepare_k8055_command(board, &datap, CMD_SET_AD, 0, 0, 0);
	exec_k8055_command(board, &datap);

	// Reset counters
	prepare_k8055_command(board, &datap, CMD_RST_CNT1, 0, 0, 0);
	exec_k8055_command(board, &datap);
	prepare_k8055_command(board, &datap, CMD_RST_CNT2, 0, 0, 0);
	exec_k8055_command(board, &datap);
	prepare_k8055_command(board, &datap, CMD_SET_CNT1, 3, 0, 0);
	exec_k8055_command(board, &datap);
	prepare_k8055_command(board, &datap, CMD_SET_CNT2, 5, 0, 0);
	exec_k8055_command(board, &datap);

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
			anout+=2;
		else
			anout-=2;

		if (anout == 254 || anout == 0) // 254 instead of 255 so we can avoid other checks
			direction_an = !direction_an;

		if (digit_out == 7 || digit_out == 0) {
			direction_dig = !direction_dig;
		}

		nanosleep(&wait, NULL);
	}

	// Reset the board
	prepare_k8055_command(board, &datap, CMD_SET_AD, 0, 0, 0);
	exec_k8055_command(board, &datap);

	usb_close(board);

	fprintf(stdout, "\nDone.\n");

	return -1;
}
