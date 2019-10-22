#ifndef _SERVER_H_
#define _SERVER_H_

//Request-line
#define METHOD_NUM 8
#define METHOD_LEN_MAX 8
const static char method_name[METHOD_NUM][METHOD_LEN_MAX] = {
	"GET",
	"POST",
	"HEAD",
	"PUT",
	"DELETE",
	"OPTIONS",
	"TRACE",
	"CONNECT"
};

enum content_t
{
	METHOD, URL, VERSION
};

//header
#define HEADER_NUM 6
#define HEADER_KEY_LEN_MAX 20
const static char header_name[HEADER_NUM][HEADER_KEY_LEN_MAX] = {
	"Host",
	"User-Agent",
	"Accept",
	"Accept-Language",
	"Accept-Encoding",
	"Connection"
};

enum header_t
{
	HOST, USER_AGENT, ACCEPTCONTENT,
	ACCEPT_LANGUAGE, ACCEPT_ENCODING, CONNECTION
};

#define BODY_NUM 20
#define CONTENT_NUM 3
#define CONTENT_LEN_MAX 2048
#define BODY_LEN_MAX 2048
#define HEADER_LEN_MAX 2048
typedef struct Http_client
{
	char content[CONTENT_NUM][CONTENT_LEN_MAX];		//request-line
	char header[HEADER_NUM][HEADER_LEN_MAX];		//headers
	char body[BODY_NUM][BODY_LEN_MAX];			//request-body
}Http_client;

char *handle(const Http_client *message);

#endif

