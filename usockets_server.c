#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>

#define SOCK_PATH  "tpf_unix_sock.server"
#define DATA "Hello from server"

void doprocessing (int sock, struct sockaddr_un addr);

int main(void)
{
	int server_sock, client_sock, len, rc;
	struct sockaddr_un server_sockaddr;
	struct sockaddr_un client_sockaddr;
	memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
	memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
	char buf[256];
	memset(buf, 0, 256);
	int backlog = 5;
	int pid;
	
	if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

	if (server_sock < 0) {
	      perror("ERROR opening socket");
	      exit(1);
	   }
	 
	server_sockaddr.sun_family = AF_UNIX;
	server_sockaddr.sun_path[0] = '\0';
	strncpy(server_sockaddr.sun_path + 1, SOCK_PATH, strlen(SOCK_PATH) );
	
	unlink(server_sockaddr.sun_path);  //unlink() before bind() to remove the socket if it already exists.
	
	len = sizeof(server_sockaddr);
	
	/* Now bind the host address using bind() call.*/
	rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1){
        printf("BIND ERROR: %d\n", errno);
        close(server_sock);
        exit(1);
    }
    
    /* Listen for any client sockets */
    rc = listen(server_sock, backlog);
    if (rc == -1){ 
        printf("LISTEN ERROR: %d\n", errno);
        close(server_sock);
        exit(1);
    }
    printf("socket listening...\n");
    
    while (1) {
		/* Accept an incoming connection */
		client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &len);
		if (client_sock == -1){
			printf("ACCEPT ERROR: %d\n", errno);
			close(server_sock);
			close(client_sock);
			exit(1);
		}

		/* Create child process */
		pid = fork();
		if (pid < 0) {
			 perror("ERROR on fork");
			 //close(server_sock);
			 //close(client_sock);
			 exit(1);
		}

		 if (pid == 0) {
			 // This is the client process
			 close(server_sock);
			 printf("PID of child:%d \n", getpid());
			 doprocessing(client_sock, client_sockaddr);
			 exit(0);
		  }
		  else {
			 printf("PID of parent:%d \n", getpid());
			 close(client_sock);
			 //close(server_sock);
		  }
    }

	printf("return 0\n");
	return 0;
}

void doprocessing (int sock, struct sockaddr_un addr) {
   int n, len, rc;
   char buffer[256];
   bzero(buffer,256);

   /* Get the name of the connected socket */
	len = sizeof(addr);
	rc = getpeername(sock, (struct sockaddr *) &addr, &len);
	if (rc == -1){
		printf("GETPEERNAME ERROR: %d\n", errno);
		//close(sock);
		//exit(1);
	}
	else {
		printf("Client socket name length: %d\n", strlen(addr.sun_path));
		printf("Client socket filepath: %s\n", addr.sun_path);
	}

	/* Read and print the data incoming on the connected socket */
	n = recv(sock, buffer, sizeof(buffer), 0);
	// can also be: n = read(sock,buffer,255);
	if (n == -1){
			printf("RECV ERROR: %d\n", errno);
			close(sock);  //if needed?
			exit(1);
		}
		else {
			printf("DATA RECEIVED = %s\n", buffer);
		}

	/* Send data back to the connected socket */
	memset(buffer, 0, 256);
	strcpy(buffer, DATA);
	printf("Sending data...\n");
	rc = send(sock, buffer, strlen(buffer), 0);
	if (rc == -1) {
		printf("SEND ERROR: %d", errno);
		exit(1);
	}
	else {
		printf("Data sent!\n");
	}
}
