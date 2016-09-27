#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>

int main(){
  /* создание сокета TCP */
  struct sockaddr_in server, client;
  int sockTCP;
  if((sockTCP = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Ошибка create()");
    return -1;
  }
  // структура для сервера
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY; // локальный адрес
  server.sin_port = htons((u_short)2015); // порт сервера
  // связка
  if(bind(sockTCP, (struct sockaddr*)&server, sizeof(server)) == -1){
    perror("Ошибка bind()");
    return -1;
  }
  // объявляем готовность к соединению
  if(listen(sockTCP,5)==-1){
    perror("Ошибка listen()");
    return -1;
  }
  // создаем опрос дескрипторов 
  int epfd = epoll_create(5);
  if(epfd == -1){
    perror("Ошибка epoll_create()");
    return -1;
  }
  /* создание сокета UDP */
  int sockUDP;
  struct sockaddr_in addr, cliaddr;
  if((sockUDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("Ошибка socket UDP()");
    return -1;
  }
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(2015);
  if(bind(sockUDP, (struct sockaddr*)&addr, sizeof(addr)) < 0){
    perror("Ошибка bind UDP()");
    return -1;
  }
  /* добавление сокетов в события */
  struct epoll_event ev;
  ev.data.fd = sockTCP;
  ev.events = EPOLLIN | EPOLLET | EPOLLHUP;
  if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockTCP, &ev) == -1){ // добавляем дескриптор TCP в структуру
    perror("Ошибка epoll_ctl()");
    return -1;
  }
  ev.data.fd = sockUDP;
  ev.events = EPOLLIN | EPOLLET | EPOLLHUP;
  if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockUDP, &ev) == -1){// добавляем дескриптор UDP в структуру
    perror("Ошибка epoll_ctl UDP()");
    return -1;
  }
  
  /* цикл работы с клиентами */
  struct epoll_event events[1024];
  int newsock, clnlen; // сокеты для клиентов
  while(1){
    int ready = epoll_wait(epfd, events, 1024, -1); // ожидаем событие
    if(ready == -1){
      perror("Ошибка epoll_wait()");
      return -1;
    }
    int j;
    for(j = 0; j < ready; j++){
      if(events[j].data.fd == sockTCP){ // если произошло событие на TCP сокете сервера
        if((newsock = accept(sockTCP, (struct sockaddr*)&client, &clnlen)) == -1){ // подключаем нового пользователя
          perror("Ошибка accept()");
          return -1;
        } 
        int size;
        char buf[255];
        size = recv(newsock, buf, sizeof(buf), 0); // пока получаем от клиента
        send(newsock, buf, sizeof(buf), 0); // отправляем эхо
        printf("TCP>>>%s\n",buf);
  	close(newsock); // отключаем пользователя, т.к. он нам больше не нужен
      } else if(events[j].data.fd == sockUDP){ // если событие на UDP сокете
        int clilen = sizeof(cliaddr);
        char buf[255];
        recvfrom(sockUDP,buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,&clilen);
        printf("UDP>>>%s\n",buf);
        sendto(sockUDP,buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,clilen);
      }
    }
  }
  close(sockTCP);
  close(sockUDP);
  return 0;
}
