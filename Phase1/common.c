#include "common_impl.h"

int creer_socket(int * port_num)
{
   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */
   /* Create socket */
	printf("Creating socket...\n");
	int server_sock = socket(AF_INET, SOCK_STREAM, 0);
	// create server addr
	char machine_name[128];
	memset(machine_name, 0, 128);
	gethostname(machine_name, 128);

	struct sockaddr_in  server_addr;
	socklen_t  len = sizeof(struct sockaddr_in);
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family= AF_INET;
	server_addr.sin_port = htons(*port_num);
	inet_aton(machine_name,&(server_addr.sin_addr));


	// bind to server addr
	printf("Binding...\n");
	if( bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("Error while binding");
		return 0;
	}

	if (getsockname(server_sock,(struct sockaddr *)&server_addr,&len) == -1 ){ //recupere le numero de port assigne a la socket
      	perror("Error getsockname function\n");
   	}
	*port_num = htons(server_addr.sin_port);

	// listen
	printf("Listening...\n");
	if (listen(server_sock, 10) == -1){
		perror("Error while listening");
		return 0;
	}

	return server_sock;

}

int handle_connect(char address_ip[], int portnb) {

	/* Create socket */
	printf("Creating socket...\n");
   int sock_fd =  socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	/* create server addr */
	char  * addr_ip = address_ip;
	short port = portnb;
	struct sockaddr_in  server_addr;
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family= AF_INET;
	server_addr.sin_port = htons(port);
	inet_aton(addr_ip,&(server_addr.sin_addr));

    /* connection direct au client */
	printf("Connecting to client ...");
    if (connect(sock_fd, (struct sockaddr *)&server_addr,sizeof(server_addr)) == -1 ){
        perror("Error connect");
        exit(1);
    }
	printf("done!\n");
	return sock_fd;

}


int send_msg(int fd, void * buffer, int len){
	int ret = -1;
	if ((ret = send(fd, buffer, len, 0)) < 0){
		printf("Error while sending a message");
		return 0;
	}
	return ret;
}

int recv_msg(int fd, void * buffer, int len){
	int ret = -1;
	if ((ret = recv(fd, buffer, len, 0)) < 0){
		printf("Error while receiving a message");
		return 0;
	}
	return ret;
}
/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */

void handle_poll(struct pollfd fds[], int num_procs){
	char buff_stdout[1024], buff_stderr[1024];
    memset(buff_stdout,0,1024), memset(buff_stderr,0,1024);

	int pipe_working = 2*num_procs;
	while(pipe_working >= 0)
	{
		int enabled = 0;
		enabled = poll(fds,2*num_procs,-1);

		if (enabled > 0){

			for (int i = 0; i < 2*num_procs; i++){
				int ret = -1;
				
				if (fds[i].revents ==  POLLIN && i%2 == 0){ //indices pairs = stdout
					printf("Je suis un stdout avec i = %i\n",i);

					if ((ret = read(fds[i].fd, buff_stdout, 1024)) > 0){
						printf("buffer non vide\n");
						printf("[Processus %i : STDOUT] : \n", i);
						//write(STDOUT_FILENO,buff_stdout,strlen(buff_stdout));
						printf("%s", buff_stdout);
						printf("[END stdout %i]\n", i);
					} else{
						pipe_working--;
					}
					
					memset(buff_stdout,0,1024);
				}
				
				else if ((fds[i].revents == POLLHUP)){
					printf("Connection ends %i\n",i);
					pipe_working--;
					//close(fds[i].fd);
				}

				else if (fds[i].revents == POLLIN && i%2 != 0){ //indices impairs = stderr
					printf("je suis un stderr avec i = %i\n",i);

					if ((ret = read(fds[i].fd, buff_stderr, 1024)) > 0){
						printf("[Processus %i : STDERR] : \n", i);
						//write(STDOUT_FILENO,buff_stderr,strlen(buff_stderr));
						printf("%s\n", buff_stderr);
						
					} else {
						pipe_working--;
					}
					printf("[END stderr %i]\n", i);
					memset(buff_stderr,0,1024);
				}
				//printf("i : %i\n",i);
			}
		}
	}
}
