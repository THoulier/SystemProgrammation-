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


ssize_t send_msg (int fd, void * buffer, int len){
        ssize_t sent = 0;
        do{
            ssize_t ret = send(fd, buffer + sent, len - sent, 0);

			if (ret != -1){
				sent += ret;
			}
            else if (ret < 0){
                if(errno == EINTR){//si appel system durant l'execution du send, on recommence
                    continue;
                } else {
                    perror("Error while sending a messsage\n");
				    break;
                }
            }
        } while(sent != len);
    return sent;
}

ssize_t recv_msg (int fd, void * buffer, int len){
        ssize_t rec = 0;
        do{
            ssize_t ret = recv(fd, buffer + rec, len - rec, 0);

			if (ret != -1){
				rec += ret;
			}
            else if (ret < 0){
                if(errno == EINTR){//si appel system durant l'execution du recv, on recommence
                    continue;
                } else {
                    perror("Error while receiving a messsage\n");
				    break;
                }
            }

        } while(rec != len);
    return rec;
}
/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */

void handle_poll(struct pollfd fds[], int num_procs){
	char buff_stdout[MSGLEN], buff_stderr[MSGLEN];
    memset(buff_stdout,'\0',MSGLEN), memset(buff_stderr,'\0',MSGLEN);

	int tabi2rank[2*num_procs]; //tableau conversion i vers le rang
	for (int j = 1; j<=2*num_procs; j=j+2){
		tabi2rank[j-1] = j/2;
		tabi2rank[j] = j/2;
	}

	int pipe_working = 2*num_procs;
	int ret = -1;
	while(pipe_working > 0)
	{
		int enabled = 0;
		enabled = poll(fds,2*num_procs,-1);

		if (enabled > 0){

			for (int i = 0; i < 2*num_procs; i++){
				ret = -1;
				if (i%2 == 0){
					if (fds[i].revents ==  POLLIN){ //indices pairs = stdout
						memset(buff_stdout,'\0',MSGLEN);

						printf("===================%i\n",i);

						printf("----------------------------[Processus %i : STDOUT]----------------------------\n", tabi2rank[i]);
						while (ret != 0){ //recuperer des msgs de toutes tailles

							ret = read(fds[i].fd, (void *)buff_stdout, MSGLEN);

							if (ret == -1){
								perror("Error while reading");
							}

							printf("%s", buff_stdout);
							memset(buff_stdout,'\0',MSGLEN);
						}
						printf("----------------------------[END STDOUT %i]----------------------------\n", tabi2rank[i]);

					} else if ((fds[i].revents == POLLHUP)){ //si fin de connexion
						printf("Connection ends stdout %i\n",i);
						pipe_working--;
						fds[i].fd = -1;
						close(fds[i].fd);
					}

				} else if (i%2 != 0){
					memset(buff_stderr,'\0',MSGLEN);
					if (fds[i].revents == POLLIN){ //indices impairs = stderr
						printf("===================%i\n",i);

						printf("----------------------------[Processus %i : STDERR]----------------------------\n", tabi2rank[i]);
						while (ret != 0 && errno != EINTR){
							if ((ret = read(fds[i].fd, (void *)buff_stderr, MSGLEN)) > 0){
								printf("%s", buff_stderr);
							}

							memset(buff_stderr,'\0',MSGLEN);
						}
						printf("----------------------------[END STDERR %i]----------------------------\n", tabi2rank[i]);

					} else if ((fds[i].revents == POLLHUP)){
						printf("Connection ends stderr %i\n",i);
						pipe_working--;
						fds[i].fd = -1;
						close(fds[i].fd);
					}
				}

			}
		}
	}
}
