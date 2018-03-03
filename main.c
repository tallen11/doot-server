// #include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// #include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
// #include <sys/mman.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
// #include <rpi_ws281x/ws2811.h>
#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "version.h"
#include "ws2811.h"

static short PORT = 1153;
static int COLOR_COUNT = 3;
static int LED_COUNT = 60;
#define REC_SIZE (30 * COLOR_COUNT)

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
	c0.count = LED_COUNT;
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

	clock_t lastTime = clock();

	const float P_CONST = 0.25f;

	for (;;) {
		int recvlen = recvfrom(s, buf, REC_SIZE, MSG_DONTWAIT, (struct sockaddr*)&remaddr, &addrlen);
		if (recvlen > 0) {
			for (int i = 0; i < recvlen; ++i) {
				targetBuf[i] = buf[i];
			}
		}

		clock_t currentTime = clock();
		float diff = ((float)(currentTime - lastTime) / (float)CLOCKS_PER_SEC) * 1000.0;
		if (diff < (1.0f / 60.0f) * 1000.0f) {
			continue;
		}

		lastTime = currentTime;

		for (int i = 0; i < REC_SIZE; i += COLOR_COUNT) {
			float red_t = (float)targetBuf[i];
			float green_t = (float)targetBuf[i+1];
			float blue_t = (float)targetBuf[i+2];

			float red_c = (float)currentBuf[i];
			float green_c = (float)currentBuf[i+1];
			float blue_c = (float)currentBuf[i+2];

			float red_pid = (red_t - red_c) * P_CONST;
			float green_pid = (green_t - green_c) * P_CONST;
			float blue_pid = (blue_t - blue_c) * P_CONST;

			uint8_t red = (uint8_t)(red_c + red_pid);
			uint8_t green = (uint8_t)(green_c + green_pid);
			uint8_t blue = (uint8_t)(blue_c + blue_pid);

			uint32_t color = (0 << 24) | (red << 16) | (green << 8) | blue;
			size_t index = i / 3;
			
			ledString.channel[0].leds[index] = color;
			ledString.channel[0].leds[LED_COUNT - 1 - index] = color;

			currentBuf[i] = red;
			currentBuf[i+1] = green;
			currentBuf[i+2] = blue;
		}
		
                ret = ws2811_render(&ledString);
                if (ret != WS2811_SUCCESS) {

                }
	}

	return 0;
}

