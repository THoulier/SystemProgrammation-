#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;


/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */
   //printf("entree dans le handler\n");
   int status;
   pid_t   pid;
   while ( (pid = waitpid(-1, &status, WNOHANG)) > 0) {
      printf("child %d terminated\n", pid);
   }
}


int main(int argc, char *argv[])
{
   if (argc < 3){
      usage();
   } else {
      pid_t pid;
      int num_procs = 0;
      int i;
      int port_num = 0;
      int sock_fd;

      /* Mise en place d'un traitant pour recuperer les fils zombies*/
      /* XXX.sa_handler = sigchld_handler; */

      struct sigaction sigchld_action;
      memset (&sigchld_action, 0, sizeof (sigchld_action));
      sigchld_action.sa_flags = SA_RESTART;
      sigchld_action.sa_handler = sigchld_handler;
      sigaction(SIGCHLD, &sigchld_action, NULL);


      /* lecture du fichier de machines */
      FILE * fichier = NULL;
      char buff[NAMELEN];
      memset(buff,0,NAMELEN);
      char caractere;
      char *positionEntree = NULL;
      fichier = fopen("machine_file","r");
      /* 1- on recupere le nombre de processus a lancer */
      do
      {
          caractere=fgetc(fichier);// lire cractere par caractere
          if(isspace(caractere))// a chaque mise à la ligne
          {
              num_procs++;// on ajoute + 1 à nombre de mots
          }
      }while(caractere != EOF);//lire jusqu'à la fin du fichier
      // printf("num proc %i\n",num_procs );
      rewind(fichier);

      /* 2- on recupere les noms des machines : le nom de */
      char ** tab_machine_name =(char**) malloc(num_procs * sizeof(char*));

      for (int i = 0;i<num_procs;i++){
        tab_machine_name[i] = malloc(sizeof(**tab_machine_name) * NAMELEN);
      }

      if (fichier != NULL){
        for (int i=0; i<num_procs; i++){
          if (fgets(buff, NAMELEN, fichier) != NULL){
            positionEntree = strchr(buff, '\n'); // On recherche l'"Entrée"
            if (positionEntree != NULL) // Si on a trouvé le retour à la ligne
            {
            *positionEntree = '\0'; // On remplace ce caractère par \0
            }
            strcpy(tab_machine_name[i],buff);
          }
        }
       }

      fclose(fichier);


      /* la machine est un des elements d'identification */

      /* creation de la socket d'ecoute */
      /* + ecoute effective */

      sock_fd = creer_socket(&port_num);


      /* Initialisation de struct poll */
      struct pollfd fds[2*num_procs]; //stdout + stderr pour chaque pipe
	   memset(&fds, 0, 2*num_procs*sizeof(struct pollfd));

      /* Initialisation des parametres du tableau argv */
      int size_argv_ssh = 7;
      char * argv_ssh[argc-3+size_argv_ssh]; //tableau argv du programme qu'on va executer avec execv
      char path[MSGLEN];
      memset(path, 0, MSGLEN);
      char path2[MSGLEN];
      memset(path2, 0, MSGLEN);
      char rank[MSGLEN];
      memset(rank, 0, MSGLEN);
      char prog[MSGLEN];
      memset(prog, 0, MSGLEN);
      char dsmexec_machine_name[NAMELEN];
      memset(dsmexec_machine_name,0,NAMELEN);
      char * dsmexec_port = malloc(sizeof(*dsmexec_port));

      gethostname(dsmexec_machine_name, NAMELEN);
      sprintf(dsmexec_port, "%i", port_num);
      /* creation des fils */
      for(i = 0; i < num_procs ; i++) {
         int fd_stdout[2];
         int fd_stderr[2];

         /* creation du tube pour rediriger stdout */
         pipe(fd_stdout);
         /* creation du tube pour rediriger stderr */
         pipe(fd_stderr);

         pid = fork();

         if(pid == -1) ERROR_EXIT("fork");

         if (pid == 0) { /* fils */

           /* redirection stdout */
            close(fd_stdout[0]);
            dup2(fd_stdout[1], STDOUT_FILENO);

            /* redirection stderr */
            close(fd_stderr[0]);
            dup2(fd_stderr[1], STDERR_FILENO);

            /* Creation du tableau d'arguments pour le ssh */

            getcwd(path,MSGLEN);
            strcpy(path2,path);
            sprintf(path,"%s/bin/dsmwrap", path); //Contient le chemin de dsmwrap
            strcpy(prog,argv[2]);
            sprintf(path2,"%s/bin/%s", path2,prog); //Contient le chemin vers le programme à exectuer sur la machine distante
            sprintf(rank, "%d", i);

            argv_ssh[0] = "ssh";
            argv_ssh[1] = tab_machine_name[i];
            argv_ssh[2] = path;
            argv_ssh[3] = dsmexec_machine_name;
            argv_ssh[4] = dsmexec_port;
            argv_ssh[5] = rank;
            argv_ssh[6] = path2;
            for (int i = size_argv_ssh; i<argc+4+1; i++){
              argv_ssh[i] = argv[i-4];
            }
            /* jump to new prog : */
            if (execvp("ssh", argv_ssh) == -1){
              printf("can't lauch dsmwrap\n");
            }

         } else  if(pid > 0) { /* pere */
            /* fermeture des extremites des tubes non utiles */
            close(fd_stdout[1]);
            close(fd_stderr[1]);

            fds[2*i].fd = fd_stdout[0]; //indice pair : on recupere les fds des stdout
	         fds[2*i].events = POLLIN; //on initialise events à POLLIN


            fds[2*i+1].fd = fd_stderr[0]; //indice impair : on recupere les fds des stderr
            fds[2*i+1].events = POLLIN; //on initialise events à POLLIN

            num_procs_creat++;
         }
      }

      /* Initialisation du tableau de structures qui va contenir les informations sur les processus */
      dsm_proc_t dsm_proc[num_procs];  //tableau pour stocker les structures reçues
      int tab_sock_fd[num_procs]; //tableau pour stocker les sockets des processus distants

      for(i = 0; i < num_procs ; i++){
      /* on accepte les connexions des processus dsm */
      struct sockaddr_in client_addr;
      socklen_t size_addr = sizeof(struct sockaddr_in);
      int client_fd = accept(sock_fd,(struct sockaddr*)&client_addr,&size_addr);
      tab_sock_fd[i] = client_fd;
      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      //size_t len_machine_name;
      //recv_msg(client_fd, (void *) &len_machine_name, sizeof(len_machine_name));
      /* 2- puis la chaine elle-meme */
      //recv_msg(client_fd, (void *) &dsm_proc[i].connect_info.name, len_machine_name);
      /* On recupere le pid du processus distant  */
      //recv_msg(client_fd, (void *) &dsm_proc[i].pid, sizeof(pid_t));
      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
      //recv_msg(client_fd, (void *) &dsm_proc[i].connect_info.port, sizeof(int));
      recv_msg(client_fd, (void *) &dsm_proc[i], sizeof(dsm_proc[i]));

      //envoi du nombre de processus aux processus dsm
      send_msg(client_fd, (void *) &num_procs, sizeof(int));
      //envoi des rangs aux processus dsm
      send_msg(client_fd, (void *) &dsm_proc[i].connect_info.rank, sizeof(int));

      printf("Processus %i / rank : %i : machine : %s ; pid : %d ; len : %i ; port : %i\n", i, dsm_proc[i].connect_info.rank, dsm_proc[i].connect_info.name, dsm_proc[i].pid, dsm_proc[i].connect_info.len_name, dsm_proc[i].connect_info.port);
      }

      /*Envoie des infos de connexions aux autres processus*/
      for (i = 0; i < num_procs ; i++){
         send_msg(tab_sock_fd[i], (void *) &dsm_proc, sizeof(dsm_proc));
      }


      /*Debug
      for (int i = 0; i<2*num_procs; i++){
            printf("fds %i : %i\n",i,fds[i].fd);
      }

      printf("nb process : %d\n", num_procs);*/

      /* gestion des E/S : on recupere les caracteres */
      /* sur les tubes de redirection de stdout/stderr */
      handle_poll(fds, num_procs);

      /* on attend les processus fils */
      int status;
      pid_t   pid2;
      while ( (pid2 = waitpid(-1, &status, WNOHANG)) > 0) {
         printf("child %d terminated\n", pid2);
      }
      /* on ferme les descripteurs proprement */
      for (i=0 ; i<2*num_procs; i++){
         close(fds[i].fd);
      }
      /* Liberation de la memoire */
 
      free(dsmexec_port);
      free(tab_machine_name);
      /* on ferme la socket d'ecoute */

      close(sock_fd);
   }
   exit(EXIT_SUCCESS);
}
