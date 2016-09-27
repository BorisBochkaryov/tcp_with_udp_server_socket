#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>

int main(){
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(2015);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  // создаем сокет
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Ошибка create()");
    return -1;
  }
  // соединяемся с сервером
  if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1){
    perror("Ошибка connect()");
    return -1;
  }
  // отправка сообщения
  send(sock, "Hello", 6, 0);
  char buf[255];
  if(recv(sock, buf, sizeof(buf), 0) < 0){
    perror("Ошибка recv()");
    return -1;
  }
  printf("Сервер: %s\n",buf);
  close(sock);
  return 0;
}
