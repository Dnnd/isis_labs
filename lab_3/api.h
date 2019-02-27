#ifndef LAB_3_API_H
#define LAB_3_API_H


#include "http.h"


struct api_client {
    struct http_client *httpc;
    const char *hostname;
    const char *portname;
};

enum QUERY_METHOD {
    GET,
    POST
};

int fetch_time(struct api_client *client,
               enum QUERY_METHOD method,
               const char *type,
               const char *format,
               struct http_response *resp);


#endif //LAB_3_API_H
