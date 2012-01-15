#ifndef _K8055_HEADER_
#define _K8055_HEADER_

#include <usb.h> // From libusb

#define DIGITAL_IN(dp) ((dp)->data[0])
#define ANALOG_IN1(dp) ((dp)->data[2])
#define ANALOG_IN2(dp) ((dp)->data[3])

#define DIGITAL_IN_INP1(d) ((d & 0x10) >> 4)
#define DIGITAL_IN_INP2(d) ((d & 0x20) >> 5)
#define DIGITAL_IN_INP3(d) ((d & 0x01) >> 0)
#define DIGITAL_IN_INP4(d) ((d & 0x40) >> 6)
#define DIGITAL_IN_INP5(d) ((d & 0x80) >> 7)

#define ENABLE_DO_PORT(d, a) (d | (1 << a)) // Enable digital out port 'a'

#ifndef BIG_ENDIAN_MACHINE
#define COUNTER1(dp) ((unsigned short)((dp)->data[5] << 8 | (dp)->data[4]))
#define COUNTER2(dp) ((unsigned short)((dp)->data[7] << 8 | (dp)->data[6]))
#else
#define COUNTER1(dp) ((unsigned short)((dp)->data[4] << 8 | (dp)->data[5]))
#define COUNTER2(dp) ((unsigned short)((dp)->data[6] << 8 | (dp)->data[7]))
#endif

typedef struct {
	unsigned char data[8];
} k8055_data_packet;

typedef enum {
	BOARD_ADDR_0 = 0x5500,
	BOARD_ADDR_1 = 0x5501,
	BOARD_ADDR_2 = 0x5502,
	BOARD_ADDR_3 = 0x5503
} board_id;

typedef enum {
	CMD_RESET = 0x00,    // Reset board ?
	CMD_SET_CNT1 = 0x01, // Set debounce counter 1
	CMD_SET_CNT2 = 0x02, // Set debounce counter 2
	CMD_RST_CNT1 = 0x03, // Reset counter 1
	CMD_RST_CNT2 = 0x04, // Reset counter 2
	CMD_SET_AD = 0x05    // Set analog/digital outputs
} board_cmd;

// WARNING: USB initialization (usb_init()/usb_set_debug()) must be done
// before calling these functions, and it must be done by the calling code.
usb_dev_handle *get_k8055_board_handle(board_id id); // The returning value must be closed with usb_close(board_handle)

/* Input values depend on the command:
 * CMD_RESET -> No input values
 * CMD_SET_CNT1/2 -> Set debounce counter value to in1
 * CMD_RST_CNT1/2 -> No input values
 * CMD_SET_AD -> Digital out values to in1, Analog out values to in2 and in3
 */
void prepare_k8055_command(usb_dev_handle *dev, k8055_data_packet *dp, board_cmd cmd, unsigned char in1, unsigned char in2, unsigned char in3);
int read_k8055_status(usb_dev_handle *dev, k8055_data_packet *dp);
int exec_k8055_command(usb_dev_handle *dev, k8055_data_packet *dp);

#endif /* _K8055_HEADER_ */
