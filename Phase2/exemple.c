#include "dsm.h"

int main(int argc, char **argv)
{
   char *pointer;
   char *current;
   int value;
    printf("----------------------DEBUT EXEMPLE-------------------------\n");
    fflush(stdout);
    pointer = dsm_init(argc,argv);
   current = pointer;

   printf("[%i] Coucou, mon adresse de base est : %p\n", DSM_NODE_ID, pointer);

   if (DSM_NODE_ID == 0)
     {
       current += 4*sizeof(int);
       value = *((int *)current);
       printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
     }
   else if (DSM_NODE_ID == 1)
     {
       current += 2*PAGE_SIZE;
       current += 16*sizeof(int);

       value = *((int *)current);
       printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
     }
   dsm_finalize();
    sleep(1);
    printf("----------------------FIN EXEMPLE-------------------------\n");
    fflush(stdout);
    while(1){
      sleep(1);
    }
   return 1;
}
