#define _POSIX_C_SOURCE 199309L

#include "common/defs.h"
#include "k8055.h"

#include <usb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	// Reset the board
	prepare_k8055_command(board, &datap, CMD_SET_AD, 0, 0, 0);
	exec_k8055_command(board, &datap);

	// Reset counters
	prepare_k8055_command(board, &datap, CMD_RST_CNT1, 0, 0, 0);
	exec_k8055_command(board, &datap);
	prepare_k8055_command(board, &datap, CMD_RST_CNT2, 0, 0, 0);
	exec_k8055_command(board, &datap);
	prepare_k8055_command(board, &datap, CMD_SET_CNT1, k8055_get_debounce_value(0), 0, 0);
	exec_k8055_command(board, &datap);
	prepare_k8055_command(board, &datap, CMD_SET_CNT2, k8055_get_debounce_value(300), 0, 0);
	exec_k8055_command(board, &datap);

	Uint16 runs = 0xFFFF/4;
	Uint8 dinp;
	while (runs--) {
		if (read_k8055_status(board, &datap) < 0) break;
		dinp = DIGITAL_IN(&datap); 

		fprintf(stdout, "\rK8055: [%.5u] DIGITAL %u %u %u %u %u - ANALOG %.3u %.3u - COUNTERS %.4X %.4X", runs, DIGITAL_IN_INP1(dinp), DIGITAL_IN_INP2(dinp), DIGITAL_IN_INP3(dinp), DIGITAL_IN_INP4(dinp), DIGITAL_IN_INP5(dinp), ANALOG_IN1(&datap), ANALOG_IN2(&datap), COUNTER1(&datap), COUNTER2(&datap));
		fflush(stdout);
	}

	// Reset the board
	prepare_k8055_command(board, &datap, CMD_SET_AD, 0, 0, 0);
	exec_k8055_command(board, &datap);

	usb_close(board);

	fprintf(stdout, "\nDone.\n");

	return -1;
}
