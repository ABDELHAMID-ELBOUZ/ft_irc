#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int main(int ac, char **av)
{
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in adr;
	
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = INADDR_ANY;
	adr.sin_port = htons(atoi(av[1]));

	int opt = 1;
	//setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bind(sock_fd, (struct sockaddr *)&adr, sizeof(adr));

	listen(sock_fd, 128);	
	
	while (true)
	{
		struct sockaddr_in client_adr;
		socklen_t	len_client_adr = sizeof(client_adr);
	
		int client_fd = accept(sock_fd, (struct sockaddr *)&client_adr, &len_client_adr);
		
	
		char buff[512];
		memset(&buff, 0, sizeof(buff));
	
		recv(client_fd, buff, sizeof(buff), 0);
	
	
		std::cout << "---> " << buff << std::endl;

	}
		
}