/*structure et fonction concernant l'aspect HTTP */

char *rewrite_url(char *url);



enum http_method {
    HTTP_GET ,
    HTTP_UNSUPPORTED ,
};
typedef struct {
    enum http_method method;
    int major_version;
    int minor_version;
    char url[1024];
} http_request;

void skip_headers(FILE *client);
void send_status(FILE *client , int code , const char *reason_phrase, http_request request);
void send_response(FILE *client , int code ,http_request request, const char *reason_phrase ,const char *message_body);
int parse_http_request(FILE *open, http_request *request);
