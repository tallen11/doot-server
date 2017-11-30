#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// #include <netdb.h>
#include <arpa/inet.h>

const short PORT = 1153;
const int REC_SIZE = 26;

int main() {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		std::cerr << "Error creating socket..." << std::endl;
		exit(0);
	}

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);

	int bindStatus = bind(s, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (bindStatus < 0) {
		std::cerr << "Error binding socket..." << std::endl;
		exit(0);
	}

	struct sockaddr_in remaddr;
	unsigned char buf[REC_SIZE];
	socklen_t addrlen = sizeof(remaddr);
	std::cout << "Waiting..." << std::endl;

	for (;;) {
		int recvlen = recvfrom(s, buf, REC_SIZE, 0, (struct sockaddr*)&remaddr, &addrlen);
		if (recvlen > 0) {
			// TODO: Received data, update LEDs


			for (int i = 0; i < recvlen; ++i) {
				std::cout << (int)buf[i] << " ";
			}
			std::cout << std::endl;

			
		}
	}

	return 0;
}
