#include "common_impl.h"

int creer_socket(int *port_num) 
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
	
	// listen
	printf("Listening...\n");
	if (listen(server_sock, 10) == -1){
		perror("Error while listening");
		return 0;
	}
	*port_num = htons(server_addr.sin_port);
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


int send_msg(int fd,void * buffer, int len){
	int ret = -1;
	if ((ret = send(fd, buffer, len, 0)) < 0){
		printf("Error while sending a message");
		return 0;
	}
	return ret;
}

int recv_msg(int fd,void * buffer, int len){
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


