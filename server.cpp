// Server side C program to demonstrate Socket programming
#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

using namespace std;


#define PORT 8080

long findSize(FILE *fp)
{
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	rewind(fp);
	return size;
}

int sendall(int sock, unsigned char *buffer, long remaining)
{
	int n, offset = 0;
	while (remaining)
	{
		n = send(sock, buffer + offset, remaining, 0);
		if (n == -1)
			break;
		remaining -= n;
		offset += n;
	}
	return n != -1;
}


void processGetRequest(int sock) {
    FILE *fp = fopen("index.html", "rb");
    if (fp)
    {
        long filesize = findSize(fp);
        send(sock, &filesize, sizeof(filesize), 0);
        unsigned char *buffer = (unsigned char*) malloc(filesize * sizeof(unsigned char));
        if (buffer)
        {
            fread(buffer, sizeof(unsigned char), filesize, fp);
            //envia o arquivo para o cliente
            if (sendall(sock, buffer, filesize))
                printf("\nFile %s successfully sended\n", "index.html");
            else
                printf("\nError sending file %s\n", "index.html");
            free(buffer);
        }
        fclose(fp);
    }
}

int main()
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        FILE *fp = fopen("index.html", "rb");
        if (fp)
        {
            long filesize = findSize(fp);
            unsigned char *fileBuffer = (unsigned char*) malloc(filesize * sizeof(unsigned char));
            if (fileBuffer)
            {
                fread(fileBuffer, sizeof(unsigned char), filesize, fp);

                string html = "";
                for (int i = 0; i < filesize; i++) {
                    html += fileBuffer[i];
                }

                string str = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " +  to_string(filesize) + "\n\n" + html;
        
                char buffer[30000] = {0};
                valread = read( new_socket , buffer, 30000);
                printf("%s\n",buffer );
                write(new_socket , str.c_str() , (int)str.size());
                printf("------------------Hello message sent-------------------\n");
                close(new_socket);


                free(fileBuffer);
            }
            fclose(fp);
        }
    }
    return 0;
}