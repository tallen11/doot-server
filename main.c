#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <assert.h>
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

const float REFRESH_RATE = 60.0f;
const float P_CONST = 0.25f;

int setupLEDStrip(ws2811_t *ledStrip);
uint32_t channelsToColor(uint8_t red, uint8_t green, uint8_t blue);

int main() {
	// Setup socket...
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		return 0;
	}

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);

	int bindStatus = bind(s, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (bindStatus < 0) {
		return 0;
	}

	struct sockaddr_in remaddr;
	uint8_t buf[REC_SIZE];
	socklen_t addrlen = sizeof(remaddr);

	// Setup LED strip
	ws2811_t ledStrip;
	int lss = setupLEDStrip(&ledStrip);
	assert(lss == 0);

	// Init current color value buffer...
	uint8_t currentBuf[REC_SIZE];
	for (int i = 0; i < REC_SIZE; ++i) {
		currentBuf[i] = 0;
	}

	// Init target color value buffer...
	uint8_t targetBuf[REC_SIZE];
	for (int i = 0; i < REC_SIZE; ++i) {
		targetBuf[i] = 0;
	}

	clock_t lastTime = clock();
	for (;;) {
		// Get any new bytes from socket. Non-blocking so we can still render at refresh rate...
		int recvlen = recvfrom(s, buf, REC_SIZE, MSG_DONTWAIT, (struct sockaddr*)&remaddr, &addrlen);
		if (recvlen > 0) {
			for (int i = 0; i < recvlen; ++i) {
				targetBuf[i] = buf[i];
			}
		}

		// Limit refresh rate of LED strip...
		clock_t currentTime = clock();
		float diff = ((float)(currentTime - lastTime) / (float)CLOCKS_PER_SEC) * 1000.0;
		if (diff < (1.0f / REFRESH_RATE) * 1000.0f) {
			continue;
		}

		lastTime = currentTime;

		// Update LED strip colors from received data...
		for (int i = 0; i < REC_SIZE; i += COLOR_COUNT) {
			// Target color values...
			float red_t = (float)targetBuf[i];
			float green_t = (float)targetBuf[i+1];
			float blue_t = (float)targetBuf[i+2];

			// Current color values...
			float red_c = (float)currentBuf[i];
			float green_c = (float)currentBuf[i+1];
			float blue_c = (float)currentBuf[i+2];

			// Simple P controller to smoothly move current color values to target values...
			float red_pid = (red_t - red_c) * P_CONST;
			float green_pid = (green_t - green_c) * P_CONST;
			float blue_pid = (blue_t - blue_c) * P_CONST;

			// Updated colors values...
			uint8_t red = (uint8_t)(red_c + red_pid);
			uint8_t green = (uint8_t)(green_c + green_pid);
			uint8_t blue = (uint8_t)(blue_c + blue_pid);

			// Create 32 bit color from individual channels...
			uint32_t color = channelsToColor(red, green, blue);
			size_t index = i / 3;
			
			// Update LED strip...
			ledStrip.channel[0].leds[index] = color;
			ledStrip.channel[0].leds[LED_COUNT - 1 - index] = color;

			// Save current colors for next time...
			currentBuf[i] = red;
			currentBuf[i+1] = green;
			currentBuf[i+2] = blue;
		}
		
		// Render the new colors...
        ws2811_return_t ret = ws2811_render(&ledStrip);
        if (ret != WS2811_SUCCESS) { }
	}

	return 0;
}

int setupLEDStrip(ws2811_t *ledStrip) {
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

	ledStrip->freq = 800000;
	ledStrip->dmanum = 5;
	ledStrip->channel[0] = c0;
	ledStrip->channel[1] = c1;

	ws2811_return_t ret = ws2811_init(ledStrip);
	if (ret != WS2811_SUCCESS) {
		return -1;
	}

	return 0;
}

uint32_t channelsToColor(uint8_t red, uint8_t green, uint8_t blue) {
	return (0 << 24) | (red << 16) | (green << 8) | blue;
}
