#include<fmt/format.h>
#include <sys/epoll.h>
#include <error.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


void setnonblocking(int fd){
	int flag = fcntl(fd,F_GETFL,0);
	fcntl(fd,F_SETFL,flag|O_NONBLOCK); 	
}

#define MAX_EVENTS 10
#define SERVER "127.0.0.1"
#define PORT 8181

int main()
{
	const char* ip = "127.0.0.1"; 	
	int port = 8181;

	struct epoll_event ev, events[MAX_EVENTS];
	int client_sock, nfds, epollfd;
	client_sock = socket(AF_INET, SOCK_STREAM,0);

	if(client_sock <= 0 ){
		fmt::print("create host socket fail {} ",strerror(errno));
		exit(1);
	}

	epollfd = epoll_create(1);
	if(epollfd < 0 ){
		fmt::print("listen error");
		exit(1);
	}
	ev.events = EPOLLIN|EPOLLOUT;
	ev.data.fd = client_sock;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD,client_sock,&ev) < 0 ){
		fmt::print("fail to add listen fd to epoll, err:{} ", strerror(errno)); 
	}
	struct sockaddr_in dest;
	/*---Initialize server address/port struct---*/
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(PORT);
	if ( inet_pton(AF_INET, SERVER, &dest.sin_addr.s_addr) == 0 ) {
		perror(SERVER);
		exit(errno);
	}
	/*---Connect to server---*/
	if ( connect(client_sock, (struct sockaddr*)&dest, sizeof(dest)) != 0 ) {
		if(errno != EINPROGRESS) {
			perror("Connect ");
			exit(errno);
		}
	}
	// first time, EPOLLIN Means connection ok

	nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
	if (nfds == -1) {
		perror("epoll_wait");
		exit(EXIT_FAILURE);
	}
	for(int i = 0 ;i < nfds; i++){
		if(events[i].events & EPOLLIN) {
			fmt::print("Socket {}  connected\n", events[i].data.fd);
		}
	}

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for(int i =0;i<nfds; i++){
			if(events[i].events & EPOLLOUT) {
				char buff[100] = "hello epoll";
				size_t ret = send(events[i].data.fd, buff,sizeof(buff),0);
				if(ret < 0){
					fmt::print("recv error {}\n",strerror(errno));
				}else{
					fmt::print("send return {}\n",ret);
				}
			}else{
				fmt::print("recv other envent \n");			
			}
		}
	}



	return 0;
}
