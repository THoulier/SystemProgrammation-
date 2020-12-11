#include "common_impl.h"

int main(int argc, char **argv)
{
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */


   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   int sock_fd_dsmexec = handle_connect(argv[1], atoi(argv[2]));

   /* Envoi du nom de machine au lanceur */
   char machine_name[128];
   size_t len_machine_name;
   gethostname(machine_name, 128);
   len_machine_name = strlen(machine_name);
   printf("argc %i\n",argc );

   for (int i=0;i<argc;i++){
     printf("arg %i is %s\n",i,argv[i] );
   }

   /* Envoi du pid au lanceur */
   pid_t pid;
   pid = getpid();

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   int port_num = 0;
   int sock_fd_distant = creer_socket(&port_num);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* Creation structure pour envoyer toutes les informations */
   dsm_proc_t dsm_proc;
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
   char * argv_cleaned[argc-4];

   for (int i = 0; i<argc-4; i++){
      argv_cleaned[i] = argv[i+4];
   }
   argv_cleaned[argc-4] = NULL;

   execvp(argv[3],argv_cleaned);

   close(sock_fd_dsmexec);
   close(STDOUT_FILENO);
   return 0;
}
