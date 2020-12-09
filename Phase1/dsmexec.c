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
}


int main(int argc, char *argv[])
{
   if (argc < 3){
      usage();
   } else {
      pid_t pid;
      int num_procs = 0;
      int i;

      /* Mise en place d'un traitant pour recuperer les fils zombies*/
      /* XXX.sa_handler = sigchld_handler; */

      char ** tab_machine_name =(char**) malloc(3 * sizeof(char*));
      for (int i = 0;i<3;i++){
        tab_machine_name[i] = malloc(sizeof(**tab_machine_name) * 128);
      }

      /* lecture du fichier de machines */
      FILE * fichier = NULL;
      char buff[128];
      memset(buff,0,128);
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
      printf("num proc %i\n",num_procs );
      rewind(fichier);
      /* 2- on recupere les noms des machines : le nom de */
      

      if (fichier != NULL){
        for (int i=0; i<num_procs; i++){
          if (fgets(buff, 128, fichier) != NULL){
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
      int port_num = 0;
      int sock_fd = creer_socket(&port_num);
      printf("le numero de port est : %d\n", port_num); //port modifie apres creation de socket

      char * dsmexec_port = malloc(sizeof(*dsmexec_port));
      sprintf(dsmexec_port, "%i", port_num);

      char dsmexec_machine_name[128];
      memset(dsmexec_machine_name,0,128);
	   gethostname(dsmexec_machine_name, 128);

      /* Initialisation de struct poll */
      struct pollfd fds[2*num_procs-1]; //stdout + stderr pour chaque pipe
	   memset(&fds, 0, 2*num_procs*sizeof(struct pollfd));


      /* creation des fils */
      for(i = 0; i < num_procs ; i++) {

         /* creation du tube pour rediriger stdout */
         int fd_stdout[2];
         pipe(fd_stdout);
         /* creation du tube pour rediriger stderr */
         int fd_stderr[2];
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

            char * argv_ssh[argc-2+6]; //tableau argv du programme qu'on va executer avec execv
            //memset(argv_ssh, 0, 7 * sizeof(*argv_ssh));

            char path[1024];
            memset(path, 0, 1024);
            getcwd(path,1024);
            sprintf(path,"%s/bin/dsmwrap", path); //Contient le chemin de dsmwrap

            argv_ssh[0] = "ssh";
            argv_ssh[1] = tab_machine_name[i];
            argv_ssh[2] = path;
            argv_ssh[3] = dsmexec_machine_name;
            argv_ssh[4] = dsmexec_port;
            for (int i = 5; i<argc+3; i++){
              argv_ssh[i] = argv[i-3];
            }
            argv_ssh[argc-2+5] = NULL;
            /* jump to new prog : */
            /* execvp("ssh",newargv); */
            execvp("ssh", argv_ssh);

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
      dsm_proc_t dsm_proc[num_procs]; //tableau pour stocker les structures reçues
      int tab_sock_fd[num_procs-1]; //tableau pour stocker les sockets des processus distants
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

      dsm_proc[i].connect_info.rank = i;
      close(client_fd);
      printf("Processus %i / rank : %i : machine : %s ; pid : %d ; len : %i ; port : %i\n", i, dsm_proc[i].connect_info.rank, dsm_proc[i].connect_info.name, dsm_proc[i].pid, dsm_proc[i].connect_info.len_name, dsm_proc[i].connect_info.port);
      }
      
      /*
      for (int i = 0; i < num_procs ; i++){
         for (int j =0; j < num_procs ; j++){
            if (i != j){ 
               //envoi du nombre de processus aux processus dsm
               send_msg(tab_sock_fd[j], (void *) &num_procs, sizeof(int));
               //envoi des rangs aux processus dsm 
               send_msg(tab_sock_fd[j], (void *) &dsm_proc[j].connect_info.rank, sizeof(int));
               //envoi des infos de connexion aux processus 
               send_msg(tab_sock_fd[j], (void *) &dsm_proc[j].connect_info.port, sizeof(int));
               send_msg(tab_sock_fd[j], (void *) &dsm_proc[j].connect_info.name, dsm_proc[j].connect_info.len_name);
            }
         }
      }
   */


     

      /* gestion des E/S : on recupere les caracteres */
      /* sur les tubes de redirection de stdout/stderr */
      /* while(1)
            {
               je recupere les infos sur les tubes de redirection
               jusqu'à ce qu'ils soient inactifs (ie fermes par les
               processus dsm ecrivains de l'autre cote ...)

            };
         */
        
      for (int i = 0; i<2*num_procs; i++){
            printf("fds %i : %i\n",i,fds[i].fd);
      }

      printf("nb process : %d\n", num_procs);
      handle_poll(fds, num_procs);

      /* on attend les processus fils */

      /* on ferme les descripteurs proprement */
      for (i=0 ; i<2*num_procs;i++){
         close(fds[i].fd);
      }
      /* on ferme la socket d'ecoute */
      close(sock_fd);
   }
   exit(EXIT_SUCCESS);
}
