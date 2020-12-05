#include "common_impl.h"

int main(int argc, char **argv)
{   
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   printf("argc is : %d\n",argc);

   for (int i = 0;i<argc;i++){
      printf("argv %i is : %s\n", i, argv[i]);
   }
   
   char buffer[128];
   memset(buffer, 0, 128);
   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */   
   int sock_fd_dsmexec = handle_connect(argv[1], atoi(argv[2]));

   /* Envoi du nom de machine au lanceur */
   char machine_name[128];
   size_t len_machine_name;
   gethostname(machine_name, 128);
   len_machine_name = strlen(machine_name);

   send_msg(sock_fd_dsmexec, (void *) &len_machine_name, sizeof(size_t));
   send_msg(sock_fd_dsmexec, machine_name, 128);

   /* Envoi du pid au lanceur */
   pid_t pid;
   pid = getpid();
   send_msg(sock_fd_dsmexec, (void *) &pid, sizeof(pid));
   printf("le pid est : %d\n", pid);
   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   int port_num = 0;
   int sock_fd_distant = creer_socket(&port_num);
   printf("le numero de port est (wrap) : %d\n", port_num);
   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   send_msg(sock_fd_dsmexec, (void *) &port_num, sizeof(port_num));

   
   /* on execute la bonne commande */
   char * argv_cleaned[argc-4];

   for (int i = 0; i<argc-4; i++){
      argv_cleaned[i] = argv[i+4];
      printf("argv cleaned %i : %s\n", i, argv_cleaned[i]);
   }
   argv_cleaned[argc-4] = NULL;
   
   execvp(argv[3],argv_cleaned);

   close(sock_fd_dsmexec);
   return 0;
}
