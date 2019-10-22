#include "apue.h"
#include "server.h"

#define MESSAGE_LEN 4096
#define PORT 6666

int sfd;
pthread_rwlock_t rwlock;

void init_network()
{
	struct sockaddr_in ser_addr;
	int ret;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	{
		perror("socket()");
		exit(1);
	}
	
	const int on = 0;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); 

	bzero(&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port	= htons(PORT);
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
	if (ret < 0)
	{
		perror("bind()");
		exit(1);
	}

	int lis = listen(sfd, 100);
	if (lis != 0)
	{
		perror("listen()");
		exit(1);
	}
}

void destory_network()
{
	close(sfd);
}

void message_classify(Http_client *http_message, char *message)
{
	//i: the number of all lines
	//j: the number of lines from start to the end of header lines
	//k: the number of body lines
	int i, j, k, tmp;
	i = j = k = tmp = 0;
	char *result[50];
	char *buf = message;

	//split by newline
	while ((result[i++] = strtok(buf, "\n")) != NULL)
	{
		printf("%s\n", result[i - 1]);
		buf = NULL;
	}
	i--;

	//Content
	char *part = NULL;
	part = strtok(result[0], " ");

	if (part != NULL && strcmp(part, "Content:"))
	{
		strcpy(http_message->content[tmp++], part);
	}

	while ((part = strtok(NULL, " ")) != NULL)
	{
		strcpy(http_message->content[tmp++], part);
	}

	//Header
	j = 1;
	while (strcmp(result[j], "\r"))
	{
		part = strtok(result[j++], " ");
		part[strlen(part) - 1] = '\0';
		for (tmp = 0; tmp < HEADER_NUM; tmp++)
		{
			if (!strcmp(part, header_name[tmp]))
			{
				part = strtok(NULL, "\0");
				strcpy(http_message->header[tmp], part);
				break;
			}
		}
	}
	j--;

	//Body
	tmp = j;
	while (tmp + 2 < i)
	{
		strcpy(http_message->body[k++], result[tmp++]);
	}

	//Show the message which has been splited
	printf("STRUCT PRINT:\n----\n");
	for (tmp = 0; tmp < CONTENT_NUM; tmp++)
	{
		printf("%s\n", http_message->content[tmp]);
	}
	for (tmp = 0; tmp < j; tmp++)
	{
		printf("%s\n", http_message->header[tmp]);
	}
	for (tmp = 0; tmp < k; tmp++)
	{
		printf("%s\n", http_message->body[tmp]);
	}

}

char *json2html(const char *json)
{
	int i = 0, f_quote = 0, j = 0;

	//Remove space and newline
	char json_str[1024];

	memset(json_str, 0, sizeof(json_str));
	for (i = 0; i < strlen(json); i++)
	{
		//Not space, tab, newline
		if (json[i] != ' ' && json[i] != '\n' && json[i] != '\t')	
		{
			json_str[j++] = json[i];
		}
	}
	printf("Json: %s\n", json_str);

	char *html_str = (char *)malloc(sizeof(char) * 2048);
	memset(html_str, 0, sizeof(html_str));

	printf("HTML_STR: %s\n", html_str);
	//Transform json to html
	/*Ex:  
	  Before transfer: {"a":{"b":{"c":"d","e":"f"},"g":"h"}}
	  After transfer: 
	  <table border="1"><tr><td>a</td><td>
	  <table border="1"><tr><td>b</td><td>
	  <table border="1"><tr><td>c</td><td>d</td>
	  </tr><tr><td>e</td><td>f</td></tr></table>
	  </td></tr><tr><td>g</td><td>h</td></tr></table></td></tr></table>
	 */
	for (i = 0; i < strlen(json_str); i++)
	{
		switch (*(json_str + i))
		{
			case '{':
				strcat(html_str, "<table border=\"1\"><tr>");
				break;
			case '"':
				if (f_quote == 0)
				{
					f_quote = 1;
					strcat(html_str, "<td>");
				}
				else
				{
					f_quote = 0;
					strcat(html_str, "</td>");
				}
				break;
			case ':':
				if (*(json_str + i + 1) != '"')
				{
					strcat(html_str, "<td>");
				}
				break;
			case ',':
				if (*(json_str + i - 1) == '"')
				{
					strcat(html_str, "</tr><tr>");
				}
				else if (*(json_str + i - 1) == '}')
				{
					strcat(html_str, "</td></tr><tr>");
				}
				break;
			case '}':
				if (*(json_str + i - 1) == '}')
				{
					strcat(html_str, "</td></tr></table>");
				}
				else
				{
					strcat(html_str, "</tr></table>");
				}
				break;
			default:
				if (isalnum(*(json_str + i)))
				{
					char tmp[2] = {*(json_str + i)};
					strcat(html_str, tmp);
				}
				break;
		}
	}
	
	return html_str;
}

char *message_handle(const Http_client *message)
{
	//Check URL
	char file[1024];
	char *response = NULL;
	char buf[20000];
	char url[1024];

	//transfer Hex code to chararcter
	bzero(buf, 0);
	int i = 0, j = 0;
	for (i = 0; i < strlen(message->content[URL]); i++, j++)
	{
		//transfer %20 to space
		const char *c = &(message->content[URL][i]);
		if ( *c == '%' && *(c + 1) == '2' && *(c + 2) == '0')
		{
			url[j] = ' ';
			i += 2;
		}
		else
		{
			url[j] = *c;
		}
	}
	printf("URL: %s\n", url);

	//Add directory name before the file
	if (strcmp(url, "/") == 0)
	{
		strcpy(file, "./www/index.html");
	}
	else
	{
		sprintf(file, "./www%s", url);
	}

	//Check json file
	int is_json_f = 0;
	char is_json[5] = {
		file[strlen(file) - 4],
		file[strlen(file) - 3],
		file[strlen(file) - 2],
		file[strlen(file) - 1],
	};

	if (strcmp(is_json, "json") == 0)	//Is json file
	{
		is_json_f = 1;
	}

	int fd = open(file, O_RDONLY);

	if (fd == -1)
	{
		fprintf(stderr, "Failed\n");
		//Status code: 404
		response = (char *)malloc(sizeof(char) * 200);
		strcpy(response, "HTTP/1.1 404 NOT FOUND\r\n Content-Type: text/html\r\n Content-Length: 35\r\n\r\n<body><h1>404 Not Found</h1></body>");
	}
	else
	{
		int rc = read(fd, buf, sizeof(buf));
		if (rc < 0)
		{
			perror("read()");
		}
		char *buf2 = buf;
		if (is_json_f == 1)
		{
			printf("BUF2: %s\n", buf2);
			buf2 = json2html(buf);
		}
		response = (char *)malloc(sizeof(char) * (strlen(buf2) + 100));
		sprintf (response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %lu\r\n\r\n%s", strlen(buf2), buf2); 
		printf ("HTTP/1.1 200 OK\r\n Content-Type: text/html\r\n Content-Length: %lu\r\n\r\n%s\n", strlen(buf2), buf2);
		if (is_json_f == 1)
		{
			free(buf2);
		}
	}
	return response;
}


void *thr_func(void *arg)
{
	Http_client http_message;
	char message[MESSAGE_LEN];
	char *response = NULL;
	int afd = *((int *)arg);

	bzero(message, sizeof(message));
	//Get request from browser
	int rc = recv(afd, message, sizeof(message) - 1, 0);

	if (rc < 0)
	{
		perror("recv()");
		exit(1);
	}
	//Split information from the message
	message_classify(&http_message, message);


	//Handle the information which has been splited
	//response: the correspond to browser
	response = message_handle(&http_message);
	printf("response: %s\n", response);

	send (afd, response, strlen(response), 0);

	free(response);

	close(afd);

	return NULL;
}

int main(void)
{
	int afd;
	struct sockaddr_in peer_addr;
	socklen_t addr_len;
	pthread_t tid;

	init_network();
	addr_len = sizeof(peer_addr);
	bzero(&peer_addr, addr_len);

	while(1)
	{
		//Connect browser
		afd = accept(sfd, (struct sockaddr*)&peer_addr, &addr_len);
		if (afd < 0)
		{
			perror("accept()");
			exit(1);
		}
		printf("connect with %s %d\n", inet_ntoa(peer_addr.sin_addr),
				ntohs(peer_addr.sin_port));

		pthread_create(&tid, NULL, thr_func, (void*)&afd);
	}

	destory_network();

	return 0;
}

