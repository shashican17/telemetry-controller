#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/ioctl.h>

int can_socket;

void can_init(){

	struct sockaddr_can addr;
	struct ifreq ifr;
	
	can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	strcpy(ifr.ifr_name, "vcan0");
	ioctl(can_socket, SIOCGIFINDEX, &ifr);
	
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	
	bind(can_socket, (struct sockaddr*)&addr, sizeof(addr));
}

void can_send(int temp){

	struct can_frame frame;
	frame.can_id = 0x100;
	frame.can_dlc = 1;
	frame.data[0] = temp;
	frame.data[1] = alarm;
	
	write(can_socket, &frame, sizeof(frame));
}


