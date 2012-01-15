#include "k8055.h"
#include "common/defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BOARD_VENDOR 0x10CF // K8055 board vendor id

#define DEFAULT_USB_TIMEOUT 50

#define READ_ENDPOINT 0x81 // USB Output endpoint
#define WRITE_ENDPOINT 0x01 // USB Input endpoint

#define MAX_DEBOUNCE_TIME 7450

int claim_usb_device(usb_dev_handle *dev, int interface);

/*** *** *** *** *** *** *** *** ***/

int claim_usb_device(usb_dev_handle *dev, int interface) {
	assert(dev);

	if (usb_detach_kernel_driver_np(dev, interface) < 0)
		fprintf(stderr, "K8055: %s\n", usb_strerror());

	usb_set_altinterface(dev, interface);
	if (usb_claim_interface(dev, interface) < 0) {
		fprintf(stderr, "K8055: %s\n", usb_strerror());
		return -1;
	}

	return 0;
}

usb_dev_handle *get_k8055_board_handle(board_id id) {
	usb_dev_handle *board_handle = NULL;
	struct usb_bus *cbus = NULL;
	struct usb_device *cdev = NULL;

	// Search for all busses, and all devices on them
	usb_find_busses();
	usb_find_devices();

	// Iterate over all device busses (usb_busses definited in usb.h)
	for (cbus = usb_busses; cbus != NULL; cbus = cbus->next)
		for (cdev = cbus->devices; cdev != NULL; cdev = cdev->next) // Iterate over all devices in the bus
			if (cdev->descriptor.idVendor == BOARD_VENDOR && cdev->descriptor.idProduct == id) {
				board_handle = usb_open(cdev); // Found the board, opening it!
	
				if (claim_usb_device(board_handle, 0) < 0) {
					fprintf(stderr, "K8055: Unable to claim board id 0x%.4X.\n", id);
					usb_close(board_handle);
					return NULL;
				} else return board_handle;
			}

	if (!board_handle)
		fprintf(stderr, "K8055: Couldn't find board with id 0x%.4X.\n", id);

	return board_handle;
}

int read_k8055_status(usb_dev_handle *dev, k8055_data_packet *dp) {
	assert(dev);
	assert(dp);

	memset(dp, 0, sizeof(k8055_data_packet));

	return usb_interrupt_read(dev, READ_ENDPOINT, (char*)(dp->data), 8, DEFAULT_USB_TIMEOUT);
}

int exec_k8055_command(usb_dev_handle *dev, k8055_data_packet *dp) {
	assert(dev);
	assert(dp);

	return usb_interrupt_write(dev, WRITE_ENDPOINT, (char*)dp->data, 8, DEFAULT_USB_TIMEOUT);
}

void prepare_k8055_command(usb_dev_handle *dev, k8055_data_packet *dp, board_cmd cmd, unsigned char in1, unsigned char in2, unsigned char in3) {
	assert(dev);
	assert(dp);

	memset(dp, 0, sizeof(k8055_data_packet));

	dp->data[0] = cmd;
	
	switch (cmd) {
		case CMD_RESET:
			break;
		case CMD_SET_CNT1:
			dp->data[6] = in1;
			break;
		case CMD_SET_CNT2:
			dp->data[7] = in1;
			break;
		case CMD_RST_CNT1:
			dp->data[4] = 0x00;
			break;
		case CMD_RST_CNT2:
			dp->data[5] = 0x00;
			break;
		case CMD_SET_AD:
			dp->data[1] = in1;
			dp->data[2] = in2;
			dp->data[3] = in3;
			break;
		default:
			fprintf(stderr, "K8055: Unknown command 0x%.2X issued.\n", cmd);
			break;
	}

}

// Debounce conversion formula took from libk8055 by Sven Lindberg! Thanks!
unsigned char k8055_get_debounce_value(unsigned int debounce_time) {
		if (debounce_time > MAX_DEBOUNCE_TIME) debounce_time = MAX_DEBOUNCE_TIME;
		return (unsigned char)roundf(sqrtf(debounce_time/0.115f));
}

