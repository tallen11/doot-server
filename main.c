// #include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// #include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
// #include <sys/mman.h>
#include <stdio.h>
// #include <rpi_ws281x/ws2811.h>
#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "version.h"
#include "ws2811.h"

const short PORT = 1153;
const int REC_SIZE = 30;

int main() {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		// std::cerr << "Error creating socket..." << std::endl;
		// exit(0);
		return 0;
	}

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);

	int bindStatus = bind(s, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (bindStatus < 0) {
		// std::cerr << "Error binding socket..." << std::endl;
		// exit(0);
		return 0;
	}

	struct sockaddr_in remaddr;
	unsigned char buf[REC_SIZE];
	socklen_t addrlen = sizeof(remaddr);
	// std::cout << "Waiting..." << std::endl;

	ws2811_t ledString;

	ws2811_channel_t c0;
	c0.gpionum = 18;
	c0.invert = 0;
	c0.count = 60;
	c0.strip_type = WS2811_STRIP_RGB;
	c0.brightness = 255;

	ws2811_channel_t c1;
	c1.gpionum = 0;
	c1.invert = 0;
	c1.count = 0;
	c1.brightness = 0;

	ledString.freq = 800000;
	ledString.dmanum = 5;
	ledString.channel[0] = c0;
	ledString.channel[1] = c1;

	ws2811_return_t ret;
	ret = ws2811_init(&ledString);
	if (ret != WS2811_SUCCESS) {
		// std::cerr << "Failed initializing LED strip" << std::endl;
		// exit(0);
		return 0;
	}

	unsigned char currentBuf[REC_SIZE];
	for (int i = 0; i < REC_SIZE; ++i) {
		currentBuf[i] = 0x0;
	}

	unsigned char targetBuf[REC_SIZE];
	for (int i = 0; i < REC_SIZE; ++i) {
		targetBuf[i] = 0x0;
	}
	
	for (;;) {
		int recvlen = recvfrom(s, buf, REC_SIZE, MSG_DONTWAIT, (struct sockaddr*)&remaddr, &addrlen);
		if (recvlen > 0) {
			for (int i = 0; i < recvlen; ++i) {
				targetBuf[i] = buf[i];
			}
		} // else {
			for (int i = 0; i < REC_SIZE; ++i) {
                        	unsigned char newVal = targetBuf[i] - currentBuf[i];
                		float interp = 0.01f * (float)(newVal);
                        	newVal = (unsigned char)interp;
				// printf("%f ", interp);

                        	uint32_t color = 0;
                        	color = (color | newVal) << 4;
                        	//color = (color | newVal) << 4;
                        	//color = (color | newVal) << 4;
                        	ledString.channel[0].leds[i] = color;

				currentBuf[i] = newVal;
                	}

                	for (int i = 0; i < REC_SIZE; ++i) {
                		unsigned char newVal = targetBuf[REC_SIZE - i] - currentBuf[REC_SIZE - i];
                		float interp = 0.01f * (float)(newVal);
                        	newVal = (unsigned char)interp;

                        	uint32_t color = 0;
                        	color = (color | newVal) << 4;
                        	//color = (color | newVal) << 4;
                        	//color = (color | newVal) << 4;
                        	ledString.channel[0].leds[REC_SIZE + i] = color;

				currentBuf[i] = newVal;
                	}

                	ret = ws2811_render(&ledString);
                	if (ret != WS2811_SUCCESS) {
                        // std::cerr << "Render error" << std::endl;
                	}
	//	}

                //std::cout << std::endl;
	}

	return 0;
}
