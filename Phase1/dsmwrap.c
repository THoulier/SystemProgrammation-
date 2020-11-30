#include "common_impl.h"

int main(int argc, char **argv)
{
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   printf("hello world\n");
   char buffer[128];
   memset(buffer, 0, 128);
   for (int i = 0; i<argc;i++){
     printf("argv %i: %s\n",i,argv[i]);
   }
   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   int sock_fd = handle_connect("127.0.0.1", 8081);
   /* Envoi du nom de machine au lanceur */
   char machine_name[128];
   size_t len_machine_name;
   gethostname(machine_name, 128);
   printf("%s\n", machine_name);
   len_machine_name = strlen(machine_name);
   send_msg(sock_fd, (void *) &len_machine_name, sizeof(size_t));
   send_msg(sock_fd, machine_name, 128);
   /* Envoi du pid au lanceur */
   pid_t pid;
   pid = getpid();
   send_msg(sock_fd, (void *) &pid, sizeof(pid));
   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
   return 0;
}
