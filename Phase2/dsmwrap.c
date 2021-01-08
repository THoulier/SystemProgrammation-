#include "common_impl.h"

int main(int argc, char **argv)
{
   int sock_fd_dsmexec;
   char machine_name[NAMELEN];
   size_t len_machine_name;
   pid_t pid;
   int port_num = 0;
   int sock_fd_distant;
   dsm_proc_t dsm_proc;
   char fd_lanceur[MSGLEN];
   char fd_dsm[MSGLEN];
   char ** new_argv =(char**) malloc((argc-4+2) * sizeof(char*));
   memset(fd_lanceur, 0, MSGLEN);
   memset(fd_dsm, 0, MSGLEN);
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */


   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   sock_fd_dsmexec = handle_connect(argv[1], atoi(argv[2]));
   sprintf(fd_lanceur, "%i", sock_fd_dsmexec);
   /* Envoi du nom de machine au lanceur */

   gethostname(machine_name, NAMELEN);
   len_machine_name = strlen(machine_name)+1;


   /* Envoi du pid au lanceur */
   pid = getpid();

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   sock_fd_distant = creer_socket(&port_num);
   sprintf(fd_dsm, "%i", sock_fd_distant);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* Creation structure pour envoyer toutes les informations */
   strcpy(dsm_proc.connect_info.name,machine_name);
   dsm_proc.connect_info.port = port_num;
   dsm_proc.connect_info.rank = atoi(argv[3]);
   dsm_proc.pid = pid;
   dsm_proc.connect_info.len_name = len_machine_name;

   /* Envoie de la structure */
   send_msg(sock_fd_dsmexec, (void *) &dsm_proc, sizeof(dsm_proc));
   fflush(stdout);
   fflush(stderr);

   /* on execute la bonne commande */
   for (int i =0; i< argc; i++){
      printf("========ANCIEN ARGV=======%s\n", argv[i]);
   }
   fflush(stdout);


   for (int i = 0;i<argc-4+2;i++){
     new_argv[i] = malloc(sizeof(**new_argv) * NAMELEN);
   }
   new_argv[0] = fd_lanceur;
   new_argv[1] = fd_dsm;

   for (int i = 0;i<argc-4;i++){
     new_argv[i+2] = argv[i+5];
   }
   for (int i =0; i< argc-5+2; i++){
      printf("=======ARGV NETTOYE========%s\n", new_argv[i]);
   }
   fflush(stdout);

   if ((execvp(argv[4],new_argv)) == -1){
     printf("did not lauch program\n");
   }

   close(sock_fd_dsmexec);
   free(new_argv);
   return 0;
}
