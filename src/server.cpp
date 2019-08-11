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

int main()
{
	const char* ip = "127.0.0.1"; 	
	int port = 8181;

	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds, epollfd;
	listen_sock = socket(AF_INET, SOCK_STREAM,0);

	if(listen_sock <= 0 ){
		fmt::print("create host socket fail {} \n",strerror(errno));
		exit(1);
	}
	sockaddr_in oAddr;
	memset(&oAddr, 0, sizeof(oAddr));
	oAddr.sin_family = AF_INET;
	oAddr.sin_addr.s_addr = inet_addr(ip );
	oAddr.sin_port = htons(port);

	if(bind(listen_sock,(sockaddr *)&oAddr, sizeof(oAddr)) < 0 ){
		fmt::print("bind error {}",strerror(errno));
		exit(1);
	}
	if(listen(listen_sock,100)){
		fmt::print("listen error");
		exit(1);
	}else{
		fmt::print("server listen at {}:{}\n",ip,port);	
	}
	epollfd = epoll_create(1024);
	if(epollfd < 0 ){
		fmt::print("listen error");
		exit(1);
	}
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD,listen_sock,&ev) < 0 ){
		fmt::print("fail to add listen fd to epoll, err:{} \n", strerror(errno)); 
	}
	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		int n;
		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listen_sock) {
				sockaddr_in  local;
				socklen_t addrlen = sizeof(local);
				conn_sock = accept(listen_sock,
						(struct sockaddr *) &local, &addrlen);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				fmt::print("new clonnection\n");
				setnonblocking(conn_sock);
				ev.events = EPOLLIN ;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
							&ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				char buff[100] = {0};
				size_t ret = recv(events[n].data.fd, buff,sizeof(buff),0);
				while(ret > 0){
					ret = recv(events[n].data.fd, buff,sizeof(buff),0);
					fmt::print("recv: {}\n", buff);
				}
				if(ret < 0){
					fmt::print("recv error {}\n",strerror(errno));

					if(epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd,NULL) < 0){
						fmt::print("delete event error {}\n", strerror(errno));
						exit(1);
					}

				}
			}
		}
	}



	return 0;
}
