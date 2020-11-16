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
      /*int fd_out;
      if (-1 == (fd_out = open("machine_file",O_RDONLY))){
         perror("Error on opening machine file");
      } */


      /* lecture du fichier de machines */
      FILE * fichier = NULL;
      char buff[128];
      memset(buff,0,128);
      char tab_machine_name[3][128];

      fichier = fopen("machine_file","r");
      if (fichier != NULL){
         while (fgets(buff, 128, fichier) != NULL){
            num_procs ++;
            strcpy(tab_machine_name[num_procs],buff);
            printf("%s\n", buff);
         }
      }
      fclose(fichier);
      printf("nb de processus à creer : %d\n", num_procs);
      for (int i=0; i<num_procs; i++){
         printf("machines : %s\n", tab_machine_name[num_procs]);
      }

      /* 1- on recupere le nombre de processus a lancer */

      /* 2- on recupere les noms des machines : le nom de */
      

      /* la machine est un des elements d'identification */
      
      /* creation de la socket d'ecoute */
      /* + ecoute effective */ 
      
      /* creation des fils */
      for(i = 0; i < 1 ; i++) {
      
         /* creation du tube pour rediriger stdout */
         
         /* creation du tube pour rediriger stderr */
         char buffsend[128];
         memset(buffsend,0,128);
         int fd_stdout[2];

         pipe(fd_stdout);

         pid = fork();
         if(pid == -1) ERROR_EXIT("fork");
         
         if (pid == 0) { /* fils */	

            close(fd_stdout[0]);
            char * argv_test[] = {"test",NULL}; //tableau argv du programme qu'on va executer avec execv
            execv("./bin/truc",argv_test);
            dup2(fd_stdout[1], STDOUT_FILENO);
            wait(NULL);
            
            /* redirection stdout */	      
            
            /* redirection stderr */	      	      
            
            /* Creation du tableau d'arguments pour le ssh */ 
            
            /* jump to new prog : */
            /* execvp("ssh",newargv); */

         } else  if(pid > 0) { /* pere */		      
            /* fermeture des extremites des tubes non utiles */
            close(fd_stdout[1]);
            read(fd_stdout[0],buffsend,128);
            write(STDOUT_FILENO,buffsend,strlen(buffsend));

            num_procs_creat++;	      
         }
      }
      
      
      for(i = 0; i < num_procs ; i++){
      
      /* on accepte les connexions des processus dsm */
      
      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      /* 2- puis la chaine elle-meme */
      
      /* On recupere le pid du processus distant  */
      
      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
      }
      
      /* envoi du nombre de processus aux processus dsm*/
      
      /* envoi des rangs aux processus dsm */
      
      /* envoi des infos de connexion aux processus */
      
      /* gestion des E/S : on recupere les caracteres */
      /* sur les tubes de redirection de stdout/stderr */     
      /* while(1)
            {
               je recupere les infos sur les tubes de redirection
               jusqu'à ce qu'ils soient inactifs (ie fermes par les
               processus dsm ecrivains de l'autre cote ...)
         
            };
         */
      
      /* on attend les processus fils */
      
      /* on ferme les descripteurs proprement */
      
      /* on ferme la socket d'ecoute */
   }   
   exit(EXIT_SUCCESS);  
}

