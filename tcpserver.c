
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define PORT "8080"
#define BACKLOG 1 //max number of connection can queue up

int check(int exp,const char *msg);
void *get_in_addr(struct sockaddr *sa);
void *handle_connection(void *sockfd);


int main(){
  int sockfd, newfd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage client_addr;


  memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;


  int rv;
  check( (rv = getaddrinfo(NULL, PORT, &hints, &servinfo)), "server: getaddrinfo");

  for(p = servinfo; p!=NULL; p = p->ai_next){
    //create socket
    check( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)), "server: socket" );
    //set socket
    int yes = 1;
    check(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)), "server: setsockopt");
    //bind socket
    check(bind(sockfd, p->ai_addr, p->ai_addrlen),"server: bind");

    break;
  }

  freeaddrinfo(servinfo);

  if(p == NULL){
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  check(listen(sockfd, BACKLOG),"server: failed to listen\n");

  printf("server is waiting for connections");


  socklen_t sin_size;
  char buffer[INET6_ADDRSTRLEN];
  while(1){
    printf("waiting for new connection\n");
    sin_size = sizeof(client_addr);
    check( newfd = accept( sockfd, (struct sockaddr*)&client_addr, &sin_size ), "server:accept\n");
    
    inet_ntop(client_addr.ss_family, get_in_addr( (struct sockaddr*)&client_addr ),buffer ,sizeof(buffer));
    printf("Got connection from %s!!!\n",buffer);

    //create new thread for each connection
    pthread_t t;
    int * sockptr = malloc(sizeof(int));
    *sockptr = newfd;
    pthread_create(&t, NULL, handle_connection, sockptr);
  }

  return 0;
}


void *handle_connection(void *p_sockfd ){
  int socket = *(int*)p_sockfd;
  check(send(socket,"HELLO WORLD", 13, 0),"server:send");
  close(socket);
  free(p_sockfd);
  return NULL;
}

int check(int exp,const char *msg){
  if(exp == -1){
    perror(msg);
    exit(1);
  }
  return exp;
}

void *get_in_addr(struct sockaddr *sa){
  if(sa->sa_family== AF_INET){
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


