#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

#define MAX 6000
void error(char* msg){
  perror(msg);
  exit(0);
}

int findAd(char *url){
  int i;
  char chaine[MAX]="";
  FILE* fichier=fopen("easylist.txt","r+");
  if(fichier==NULL){
	error("Lecture du fichier impossible\n");
  }   
  while(fgets(chaine,MAX+1,fichier)!=NULL){
		for(i=0;chaine[i]!='\0';i++){
			if(chaine[i]=='\n' && chaine[i+1]=='\0'){
				chaine[i]='\0';
				break;
			}
		}
		if(strstr(url,chaine)!=NULL){
			printf("\n Pub trouvée dans l'url %s \n",url);
			fclose(fichier);
			return 1;
			
		}
  }
  fclose(fichier);
  return 0;
}

int main(int argc,char* argv[]){
  pid_t pid;
  //struct sockaddr_in cli_addr,serv_addr;
  struct hostent* host;
  struct sockaddr_in6 from;
  struct addrinfo req, *ans;
  int sockfd,newsockfd;
  int code;

  if(argc<2)
    error("./proxy <port_no>");

  printf("\n*****WELCOME TO PROXY SERVER*****\n");
  if(fopen("easylist.txt","r")==NULL){
	  system("wget 'www.easylist.to/easylist/easylist.txt' ");
  }
  
 /* bzero((char*)&serv_addr,sizeof(serv_addr));
  bzero((char*)&cli_addr, sizeof(cli_addr));

  serv_addr.sin_family=AF_INET;
  serv_addr.sin_port=htons(atoi(argv[1]));
  serv_addr.sin_addr.s_addr=INADDR_ANY;*/
  req.ai_flags= AI_PASSIVE;
  req.ai_family= PF_INET6;
  req.ai_socktype= SOCK_STREAM;
  req.ai_protocol=0;
  
  code = getaddrinfo(NULL,argv[1],&req, &ans);
  if(code !=0){
	error("failed getaddrinfo");
  }



  //sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  sockfd=socket(ans->ai_family,ans->ai_socktype,ans->ai_protocol);
  if(sockfd<0)
    error("Problem in initializing socket");

  //if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    if(bind(sockfd,ans->ai_addr, ans->ai_addrlen)<0)
      error("Error on binding");


  listen(sockfd,50);
  //int clilen=sizeof(cli_addr);
    int clilen=sizeof(from);
  accepting:

  //newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
  newsockfd=accept(sockfd,(struct sockaddr*)&from,&clilen);
  if(newsockfd<0)
    error("Problem in accepting connection");


  pid=fork();
  if(pid==0){
    struct sockaddr_in host_addr;
    int flag=0,newsockfd1,n,port=0,i,sockfd1;
    char buffer[510],t1[300],t2[300],t3[10];
    char* temp=NULL;
    bzero((char*)buffer,300);
    recv(newsockfd,buffer,300,0);
    sscanf(buffer,"%s %s %s",t1,t2,t3);

    if(((strncmp(t1,"GET",3)==0))&&((strncmp(t3,"HTTP/1.1",8)==0)||(strncmp(t3,"HTTP/1.0",8)==0))&&(strncmp(t2,"http://",7)==0)){
      strcpy(t1,t2);				
      flag=0;
      for(i=7;i<strlen(t2);i++){
        if(t2[i]==':'){
          flag=1;
          break;
        }
      }

      temp=strtok(t2,"//");
      if(flag==0){
        port=80;
        temp=strtok(NULL,"/");
      }
      else{
        temp=strtok(NULL,":");
      }

      sprintf(t2,"%s",temp);
      printf("host = %s",t2);
      host=gethostbyname(t2);

      if(flag==1){
        temp=strtok(NULL,"/");
        port=atoi(temp);
      }


      strcat(t1,"^]");
      temp=strtok(t1,"//");
      temp=strtok(NULL,"/");
      if(temp!=NULL) 
        temp=strtok(NULL,"^]");
      printf("\npath = %s\nPort = %d\n",temp,port);
      bzero((char*)&host_addr,sizeof(host_addr));
      host_addr.sin_port=htons(port);
      host_addr.sin_family=AF_INET;
      bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length);

      sockfd1=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      newsockfd1=connect(sockfd1,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));
      sprintf(buffer,"\nConnected to %s  IP - %s\n",t2,inet_ntoa(host_addr.sin_addr));
      if(newsockfd1<0)
        error("Error in connecting to remote server");
      //printf("\n%s\n",buffer);

     // send(newsockfd,buffer,strlen(buffer),0);
      bzero((char*)buffer,sizeof(buffer));
      if(temp!=NULL)
        sprintf(buffer,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",temp,t3,t2);
      else
        sprintf(buffer,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",t3,t2);

      n=send(sockfd1,buffer,strlen(buffer),0);
      printf("TEST\n");
     // printf("\n%s\n",buffer);
      if(n<0)
        error("Error writing to socket");
      else{
        do{
          bzero((char*)buffer,300);
          n=recv(sockfd1,buffer,300,0);
	  if(!(n<=0)){
	    if(findAd(buffer)==0){  			
                 send(newsockfd,buffer,n,0);
	    }
          }
	
        }while(n>0);
      }
    }
    else{
      send(newsockfd,"400 : BAD REQUEST\nONLY HTTP REQUESTS ALLOWED",18,0);
    }
    close(sockfd1);
    close(newsockfd);
    close(sockfd);
    _exit(0);
  }
  else{
    close(newsockfd);
    goto accepting;
  }
  freeaddrinfo(ans);
  return 0;
}
