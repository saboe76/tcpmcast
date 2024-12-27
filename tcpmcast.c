#include <stdio.h>          /* printf(), snprintf() */
#include <stdlib.h>         /* strtol(), exit() */
#include <sys/types.h>
#include <sys/socket.h>     /* socket(), setsockopt(), bind(), recvfrom(), sendto() */
#include <errno.h>          /* perror() */
#include <netinet/in.h>     /* IPPROTO_IP, sockaddr_in, htons(), htonl() */
#include <arpa/inet.h>      /* inet_addr() */
#include <unistd.h>         /* fork(), sleep() */
#include <sys/utsname.h>    /* uname() */
#include <string.h>         /* memset() */
#include <time.h>
#include <sys/time.h>

#define BUF_IN_LEN  1023
#define BUF_OUT_LEN 2048

// get the local time in format 'format'
size_t get_localtime(struct timeval *tv, char *buf, size_t buf_len, const char *format) {
	char tmp[32];
	struct tm *tm_info;
	tm_info = localtime(&tv->tv_sec);
	strftime(tmp, sizeof(tmp), format, tm_info);
	return snprintf(buf, buf_len, "%s.%03d ", tmp, (int)tv->tv_usec/1000);
}

int main(int argc, char *argv[]) {
	struct timeval tv;

	int sock_src, sock_dst;
	struct sockaddr_in sa_src, sa_dst, sa_loc;

	int n, m, ret;
	unsigned short port_src, port_dst;
	char buf_in[BUF_IN_LEN+1];
	char buf_out[BUF_OUT_LEN];
	char buf_time[64];


	// cmdline checks, we need exactly 5 parameters
	//
	// 5 parameters?
	if (argc != 6) {
		fprintf(stderr, "Usage: %s src_ip src_port   mc_ifaddr mc_group mc_port\n", argv[0]);
		exit(1);
	}
	// src_ip is ipv4 address?
	if (inet_addr(argv[1]) == (in_addr_t)-1) {
		fprintf(stderr, "src_ip (%s) is not a valid ipv4 address\n", argv[1]);
		exit(1);
	}
	// src_port is in range?
	port_src = strtol(argv[2], NULL, 0);
	if (port_src < 1 || port_src > 65535) {
		fprintf(stderr, "src_port (%s) is not a valid port number\n", argv[2]);
		exit(1);
	}
	// multicast interface address is valid ipv4 address?
	if (inet_addr(argv[3]) == (in_addr_t)-1) {
		fprintf(stderr, "mc_ifaddr (%s) is not a valid ipv4 address\n", argv[3]);
		exit(1);
	}
	// multicast address is valid ipv4 address?
	if (inet_addr(argv[4]) == (in_addr_t)-1) {
		fprintf(stderr, "mc_group (%s) is not a valid ipv4 address\n", argv[4]);
		exit(1);
	}
	// multicast port is in range?
	port_dst = strtol(argv[5], NULL, 0);
	if (port_dst < 1 || port_dst > 65535) {
		fprintf(stderr, "mc_port (%s) is not a valid port number\n", argv[5]);
		exit(1);
	}


	// get udp socket and bind to interace
	//
	sock_dst = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_dst < 0) {
		perror("mcast socket");
		exit(-1);
	}
	// interface address to send from
	memset(&sa_loc, 0, sizeof(sa_loc));
	sa_loc.sin_family = AF_INET;
	sa_loc.sin_addr.s_addr = inet_addr(argv[3]);
	sa_loc.sin_port = 0;
	// mcast group to send to, from that interface address
	memset(&sa_dst, 0, sizeof(sa_dst));
	sa_dst.sin_family = AF_INET;
	sa_dst.sin_addr.s_addr = inet_addr(argv[4]);
	sa_dst.sin_port = htons(port_dst);
	// "bind" to interface
	ret = setsockopt(sock_dst, IPPROTO_IP, IP_MULTICAST_IF,
			(struct sockaddr_in *)&sa_loc, sizeof(sa_loc));
	if (ret < 0) {
		perror("IP_MULTICAST_IF");
		exit(-1);
	}


	// tcp socket, connect
	//
	sock_src = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_src < 0) {
		perror("tcp socket");
		exit(-1);
	}
	// server to receive from
	memset(&sa_src, 0, sizeof(sa_src));
	sa_src.sin_family = AF_INET;
	sa_src.sin_addr.s_addr = inet_addr(argv[1]);
	sa_src.sin_port = htons(port_src);
	// connect to source
	ret = connect(sock_src, (const struct sockaddr *)&sa_src,
			sizeof(sa_src));
	if (ret < 0) {
		perror("tcp connect");
		exit(-1);
	}


	while (1) {
		// read from src
		n = read(sock_src, buf_in, sizeof(buf_in));

		// get time stamp
		gettimeofday(&tv, NULL);

		// exit on failed read
		if (n <= 0) {
			perror("tcp read");
			exit(-1);
		}

		// terminate by newline if not
		if (buf_in[n-1] != '\n') {
			buf_in[n++] = '\n';
		}

		// get textual time "epoch.milliseconds "
		m = get_localtime(&tv, buf_time,
				sizeof(buf_time), "%s");

		// concat time and received data to out buffer
		memcpy(buf_out+0, buf_time, m );
		memcpy(buf_out+m, buf_in,   n);

		// send to everyone who's listening
		n = sendto(sock_dst, buf_out, n+m, 0,
				(struct sockaddr *)&sa_dst,
				sizeof(sa_dst));
		if (n <= 0) {
			perror("udp sendto");
			exit(-1);
		}
	}

	return 0;
}
