#include "dsm.h"
#include "common_impl.h"

int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */
struct pollfd * DSM_POLL;

/* indique l'adresse de debut de la page de numero numpage */
static char *num2address( int numpage )
{
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

   if( pointer >= (char *)TOP_ADDR ){
      fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
      return NULL;
   }
   else return pointer;
}

/* cette fonction permet de recuperer un numero de page */
/* a partir  d'une adresse  quelconque */
static int address2num( char *addr )
{
  return (((long int)(addr - BASE_ADDR))/(PAGE_SIZE));
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
   if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
	if (state != NO_CHANGE )
	table_page[numpage].status = state;
      if (owner >= 0 )
	table_page[numpage].owner = owner;
      return;
   }
   else {
	fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
      return;
   }
}

static dsm_page_owner_t get_owner( int numpage)
{
   return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

static void dsm_free_page( int numpage )
{
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}

static void *dsm_comm_daemon( void *arg)
{
   printf("comm daemon\n");
   fflush(stdout);
   struct message msg;
   memset(&msg, sizeof(msg),0);
   while(1){
      int enabled = 0;
      enabled = poll(DSM_POLL,DSM_NODE_NUM,-1);
      printf("------------------------------------------------------------------%d\n",DSM_NODE_NUM);
      fflush(stdout);
      if (enabled > 0){
         for (int i = 0; i<DSM_NODE_NUM; i++){

            if (i = DSM_NODE_ID){
               DSM_POLL[i].events = POLLIN;
            }
            if (DSM_POLL[i].revents & POLLHUP){
               //close(DSM_POLL[i].fd);
               //exit(EXIT_FAILURE);
            }
            if (DSM_POLL[i].revents == POLLIN){
               recv_msg(DSM_POLL[i].fd, (void*) &msg, sizeof(msg));
               printf("msg received\n");
               fflush(stdout);
               if (msg.type == REQUEST){
                  dsm_free_page (msg.page_nb);
                  msg.type == FREED;
                  send_msg(DSM_POLL[i].fd, (void*) &msg, sizeof(msg));
                  printf("msg sent\n");
                  fflush(stdout);
               }
               if (msg.type == FREED){
                  dsm_alloc_page(msg.page_nb);
                  dsm_change_info(msg.page_nb, table_page[msg.page_nb].status, table_page[msg.page_nb].owner );
               }
               if (msg.type == UNAVAILABLE){
                  printf("page unavailable\n");
                  fflush(stdout);
               }
            }
         }
      }
   }
   pthread_exit(NULL);
}

static int dsm_send(int dest,void *buf,size_t size)
{
   /* a completer */
}

static int dsm_recv(int from,void *buf,size_t size)
{
   /* a completer */
}

static void dsm_handler( void* addr)
{
   int page = address2num(addr);
   //printf("page demandée: %i\n",page );
   fflush(stdout);


   /*printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   abort(); */
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context)
{
   /* A completer */
   /* adresse qui a provoque une erreur */
   //printf("*******************a segfault has occured*******************\n");
   fflush(stdout);
   void  *addr = info->si_addr;
   struct message msg;
   memset(&msg, sizeof(msg),0);
   int page = address2num(addr);

   msg.type = REQUEST;
   msg.page_nb = page;
   msg.owner = get_owner(page);
   //printf("owner: %i\n",msg.owner);
   //printf("sending to fd: %d\n",DSM_POLL[msg.owner].fd );
   send_msg(DSM_POLL[msg.owner].fd, (void*) &msg, sizeof(msg));
  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
  */
   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
     {
	dsm_handler(addr);
     }
   else
     {
	/* SIGSEGV normal : ne rien faire*/
     }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{
   struct sigaction act;
   int index;
   int sock_dsm;
   int sock_dsmexec;

   sock_dsmexec = atoi(argv[0]);
   sock_dsm = atoi(argv[1]);
   /*Debugprintf("%d et %d\n", sock_dsm, sock_dsmexec);
   fflush(stdout);*/

   /* reception du nombre de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_NUM)*/
   recv_msg(sock_dsmexec,(void*) &DSM_NODE_NUM,sizeof(int));

   DSM_POLL = (struct pollfd*) malloc(sizeof(struct pollfd) *DSM_NODE_NUM);

   /* reception de mon numero de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_ID)*/
   recv_msg(sock_dsmexec,(void*) &DSM_NODE_ID,sizeof(int));

   /* reception des informations de connexion des autres */
   /* processus envoyeestab_sock_fd par le lanceur : */
   /* nom de machine, numero de port, etc. */
   dsm_proc_t dsm_proc[DSM_NODE_NUM];  //tableau pour stocker les structures reçues
   recv_msg(sock_dsmexec,(void*) &dsm_proc,sizeof(dsm_proc));
   sleep(1);
   printf("===================================INTERCONNEXION=====================================");
   fflush(stdout);
   for (int i=0; i<DSM_NODE_NUM; i++){
      printf("i == %d et RANG == %d\n",i, DSM_NODE_ID);
      fflush(stdout);

      if (dsm_proc[i].connect_info.rank < DSM_NODE_ID){ //si le process a un rang inferieur, on accepte la connexion
         struct sockaddr_in client_addr;
         socklen_t size_addr = sizeof(struct sockaddr_in);
         int client_fd = accept(sock_dsm,(struct sockaddr*)&client_addr,&size_addr);
         printf("fd du client qui s'est connecté : %d\n", client_fd);
         fflush(stdout);
         printf("user; %i, fd: %i\n",DSM_NODE_ID,client_fd );
         DSM_POLL[DSM_NODE_ID].fd = client_fd;

      } else if (dsm_proc[i].connect_info.rank > DSM_NODE_ID){ //si le process a un rang superieur, on se connecte
         int fd = handle_connect(dsm_proc[i].connect_info.name, dsm_proc[i].connect_info.port);
         DSM_POLL[dsm_proc[i].connect_info.rank].fd = fd;
         fflush(stdout);
      }

   }

   for (int i=0; i<DSM_NODE_NUM;i++){//Affichage infos de connexions reçues
      printf("Processus %i / rank : %i : machine : %s ; pid : %d ; len : %i ; port : %i\n", i, dsm_proc[i].connect_info.rank, dsm_proc[i].connect_info.name, dsm_proc[i].pid, dsm_proc[i].connect_info.len_name, dsm_proc[i].connect_info.port);
      fflush(stdout);
   }
   printf("===================================INTERCONNEXION DONE=====================================");
   fflush(stdout);
   /*Debug
   printf("______________%d et %d_______________\n", DSM_NODE_ID, DSM_NODE_NUM);
   fflush(stdout);*/

   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */

   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){
     if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
       dsm_alloc_page(index);
     dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
   }

   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO;
   act.sa_sigaction = segv_handler;
   sigaction(SIGSEGV, &act, NULL);

   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);

   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize( void )
{
   /* fermer proprement les connexions avec les autres processus */

   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);

  return;
}
