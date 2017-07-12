/*
author: Jakub Lyko
*/

//simple change for branch

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
#define DATA "Hello from client"

int main(int argc, char *argv[] )
{
	int client_sock, rc, len;
	struct sockaddr_un server_sockaddr;
	struct sockaddr_un client_sockaddr;
	char buf[256];
	memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
	memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));

	if(argc < 2) {
		printf("argc < 2\n");
		exit(1);
	}

	// Create a UNIX domain stream socket
	client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (client_sock == -1) {
		printf("SOCKET ERROR = %d\n", errno);
		exit(1);
	}

	/***************************************/
	/* Set up the UNIX sockaddr structure  */
	/* by using AF_UNIX for the family and */
	/* giving it a filepath to bind to.    */
	/*                                     */
	/* Unlink the file so the bind will    */
	/* succeed, then bind to that file.    */
	/***************************************/
	client_sockaddr.sun_family = AF_UNIX;
	//strncpy(client_sockaddr.sun_path, CLIENT_PATH);
	client_sockaddr.sun_path[0] = '\0';
	strncpy(client_sockaddr.sun_path + 1, CLIENT_PATH, strlen(CLIENT_PATH) );
	strncpy(client_sockaddr.sun_path + 1 + strlen(CLIENT_PATH), argv[1], strlen(argv[1])); //add client id
	len = sizeof(client_sockaddr);

	unlink(CLIENT_PATH);
	rc = bind(client_sock, (struct sockaddr *) &client_sockaddr, len);
	if (rc == -1){
		printf("BIND ERROR: %d\n", errno);
		close(client_sock);
		exit(1);
	}

	/***************************************/
	/* Set up the UNIX sockaddr structure  */
	/* for the server socket and connect   */
	/* to it.                              */
	/***************************************/
	server_sockaddr.sun_family = AF_UNIX;
	//strcpy(server_sockaddr.sun_path, SERVER_PATH);
	server_sockaddr.sun_path[0] = '\0';
	strncpy(server_sockaddr.sun_path + 1, SERVER_PATH, strlen(SERVER_PATH) );
	rc = connect(client_sock, (struct sockaddr *) &server_sockaddr, len);
	if(rc == -1){
		printf("CONNECT ERROR = %d\n", errno);
		printf("Error description is : %s\n",strerror(errno));
		close(client_sock);
		exit(1);
	}

	/************************************/
	/* Copy the data to the buffer and  */
	/* send it to the server socket.    */
	/************************************/
	strcpy(buf, DATA);
	printf("Sending data...\n");
	rc = send(client_sock, buf, strlen(buf), 0);
	if (rc == -1) {
		printf("SEND ERROR = %d\n", errno);
		close(client_sock);
		exit(1);
	}
	else {
		printf("Data sent!\n");
	}

	/**************************************/
	/* Read the data sent from the server */
	/* and print it.                      */
	/**************************************/
	printf("Waiting to receive data...\n");
	memset(buf, 0, sizeof(buf));
	rc = recv(client_sock, buf, sizeof(buf), 0);
	if (rc == -1) {
		printf("RECV ERROR = %d\n", errno);
		close(client_sock);
		exit(1);
	}
	else {
		printf("DATA RECEIVED = %s\n", buf);
	}

	/******************************/
	/*In the end  close the socket. */
	/******************************/
	close(client_sock);
}
