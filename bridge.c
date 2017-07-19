#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFFLEN	512

// Bridge discovery packet from Hue Documentation
char *msearch = "M-SEARCH * HTTP/1.1\r\n"
		"HOST:239.255.255.250:1900\r\n"
		"MAN:\"ssdp:discover\"\r\n"
		"ST:ssdp:all\r\n"
		"MX:1";


static void
fatal(char *e)
{
	perror(e);
	exit(1);
}
int
main()
{
	int sockfd;
	int len;
	char buf[BUFFLEN];
	struct sockaddr_in sockaddr_bind, sockaddr_send, sockaddr_recv;
	socklen_t recvlen = sizeof(sockaddr_recv);
	struct timeval tv;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		fatal("Failed to create socket");

	// Setup sockaddr's
	bzero(&sockaddr_bind, sizeof(sockaddr_bind));
	sockaddr_bind.sin_family = AF_INET;
	sockaddr_bind.sin_port = htons(0);
	sockaddr_bind.sin_addr.s_addr = htonl(INADDR_ANY);

	bzero(&sockaddr_send, sizeof(sockaddr_send));
	sockaddr_send.sin_family = AF_INET;
	sockaddr_send.sin_port = htons(1900);
	sockaddr_send.sin_addr.s_addr = inet_addr("239.255.255.250");

	if (bind(sockfd, (struct sockaddr *)&sockaddr_bind,
				sizeof(sockaddr_bind)) == -1)
		fatal("Failed to bind socket");

	if (sendto(sockfd, msearch, strlen(msearch), 0,
		(struct sockaddr *)&sockaddr_send, sizeof(sockaddr_send)) == -1)
		fatal("Failed to send discovery packet");

	// TODO verify use of SO_RCVTIMEO
	bzero(&tv, sizeof(tv));
	tv.tv_sec = 5;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
	while(1) {
		if ((len = recvfrom(sockfd, buf, BUFFLEN, 0,
			(struct sockaddr *)&sockaddr_recv, &recvlen)) == -1) {
			switch (errno) {
				case EWOULDBLOCK:
					goto out;
				default:
					fatal("Failed to read data from socket");
			}
		}
		if (strstr(buf, "IpBridge") != NULL)
			printf("Received packet from %s:%d\n",
				inet_ntoa(sockaddr_recv.sin_addr),
					ntohs(sockaddr_recv.sin_port));
	}
out:
	return(0);
}
