#ifndef LAB_3_HTTP_H
#define LAB_3_HTTP_H

#include <stddef.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "vector.h"

#define CRLF "\r\n"
#define HTTP_VERSION " HTTP/1.1"
#define HTTP_METHOD_POST "POST "
#define HTTP_METHOD_GET "GET "


struct http_header {
    struct byte_vector data;
};

struct query_parameter {
    const char *name;
    size_t name_len;
    const char *value;
    size_t value_len;
};

struct http_client_config {
    size_t bytes_per_read;
    size_t bytes_per_write;
    size_t initial_read_buffer_size;
    size_t initial_write_buffer_size;
    size_t initial_get_response_headers_size;
    size_t initial_post_response_headers_size;
    size_t initial_get_response_body_size;
    size_t initial_post_response_body_size;
    size_t initial_request_path_size;
};

struct http_get_request {
    struct byte_vector path;
    struct query_parameter *query_params;
    struct http_header *headers;
    size_t query_params_num;
    size_t headers_num;
};

struct http_post_request {
    struct byte_vector path;
    struct query_parameter *query_params;
    struct http_header *headers;
    size_t query_params_num;
    size_t headers_num;
    struct byte_vector body;
};

struct http_client {
    struct byte_vector read_buffer;
    struct byte_vector write_buffer;
    struct http_client_config *config;
};

struct http_response {
    struct byte_vector body;
    int status_code;
};


void init_http_client(struct http_client *client, struct http_client_config *config);

void init_http_response(struct http_client *client, struct http_response *response);

void init_http_post_request(struct http_client *client, struct http_post_request *request);

int dump_query_parameters(struct query_parameter *params, size_t params_num, struct byte_vector *dest);

void parse_post_query_parameters(const struct http_post_request *req, struct byte_vector *dest);

void parse_get_query_parameters(const struct http_get_request *req, struct byte_vector *dest);

void clear_http_get_request(struct http_get_request *request);

void clear_http_post_request(struct http_post_request *request);

void clear_http_response(struct http_response *response);

void clear_http_client(struct http_client *client);

int make_http_header(const char *name,
                     size_t name_len,
                     const char *value,
                     size_t value_len,
                     struct http_header *header);

int parse_http_response(struct http_client *client, struct http_response *response);

int read_http_response(struct http_client *http_client,
                       struct http_response *response,
                       int fd);

void dump_http_response(struct http_response *response);

int dump_header_to_buffer(struct byte_vector *dest_buffer, struct http_header *header);

int dump_headers_to_buffer(struct byte_vector *dest_buffer, struct http_header *headers, size_t headers_num);

int dump_http_get_request_to_buffer(const struct http_get_request *request, struct byte_vector *dest);


int dump_http_post_request_to_buffer(const struct http_post_request *request, struct byte_vector *dest);

int request_http(struct http_client *http_client, struct http_response *response, int fd);

int http_get(struct http_client *http_client,
             const struct http_get_request *request,
             struct http_response *response,
             int fd);

int http_post(struct http_client *http_client,
              const struct http_post_request *request,
              struct http_response *response,
              int fd);


int open_socket(const char *hostname, const char *portname, int *target_desc);

#endif //LAB_3_HTTP_H
