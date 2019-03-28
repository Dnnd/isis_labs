#include "http.h"
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

void init_http_client(struct http_client *client, struct http_client_config *config) {
//    bzero(client, sizeof(*client));
    init_byte_vector(config->initial_read_buffer_size, &client->read_buffer);
    init_byte_vector(config->initial_write_buffer_size, &client->write_buffer);
    client->config = config;

}

void init_http_response(struct http_client *client, struct http_response *response) {
    bzero(response, sizeof(*response));
    init_byte_vector(client->config->initial_get_response_body_size, &response->body);

}

void init_http_post_request(struct http_client *client, struct http_post_request *request) {
    bzero(request, sizeof(*request));
    init_byte_vector(client->config->initial_post_response_body_size, &request->body);
}

int dump_query_parameters(struct query_parameter *params, size_t params_num, struct byte_vector *dest) {
    for (size_t i = 0; i < params_num; ++i) {
        int err = extend(dest, params[i].name, params[i].name_len);
        if (err != 0) {
            return err;
        }

        err = push_back(dest, '=');
        if (err != 0) {
            return err;
        }

        err = extend(dest, params[i].value, params[i].value_len);
        if (err != 0) {
            return err;
        }

        if (i + 1 < params_num) {
            err = push_back(dest, '&');
            if (err != 0) {
                return err;
            }
        }
    }
    return 0;
}

void parse_post_query_parameters(const struct http_post_request *req, struct byte_vector *dest) {
    dump_query_parameters(req->query_params, req->query_params_num, dest);
}

void parse_get_query_parameters(const struct http_get_request *req, struct byte_vector *dest) {
    dump_query_parameters(req->query_params, req->query_params_num, dest);
}

void clear_http_get_request(struct http_get_request *request) {
    clear_byte_vector(&request->path);
    struct http_header *headers = request->headers;
    for (int i = 0; i < request->headers_num; ++i) {
        clear_byte_vector(&headers[i].data);
    }
}

void clear_http_post_request(struct http_post_request *request) {
    clear_byte_vector(&request->path);
    struct http_header *headers = request->headers;
    for (int i = 0; i < request->headers_num; ++i) {
        clear_byte_vector(&headers[i].data);
    }
    clear_byte_vector(&request->body);
}

void clear_http_response(struct http_response *response) {
    clear_byte_vector(&response->body);
    bzero(response, sizeof(*response));
}

void clear_http_client(struct http_client *client) {
    clear_byte_vector(&client->read_buffer);
    clear_byte_vector(&client->write_buffer);
    bzero(client, sizeof(*client));
}

int
make_http_header(const char *name, size_t name_len, const char *value, size_t value_len, struct http_header *header) {

    bzero(header, sizeof(*header));
    init_byte_vector(name_len + value_len + 2, &header->data);

    int err = extend(&header->data, name, name_len);
    if (err != 0) {
        clear_byte_vector(&header->data);
        return err;
    }

    err = extend(&header->data, ": ", 2);
    if (err != 0) {
        clear_byte_vector(&header->data);
        return err;
    }

    err = extend(&header->data, value, value_len);
    if (err != 0) {
        clear_byte_vector(&header->data);
        return err;
    }

    return 0;
}

int parse_http_response(struct http_client *client, struct http_response *response) {
    swap(&client->read_buffer, &response->body);
    const char *body_it = response->body.data;
    char *resp = "HTTP/1.0 200 OK";
    char *template_it = resp;
    int matched = 0;
    while ((*template_it != '\0' && *template_it == *body_it)
           || (matched == 7 && *template_it != '\0')) {
        ++template_it;
        ++body_it;
        ++matched;
    }
    if ((*template_it != '\0' && matched == 9) || matched == 15) {
        char *end = NULL;
        int status_code = (int) strtol(response->body.data + 9, &end, 10);
        response->status_code = status_code;
        return 0;
    }
    return 0;
}

int read_http_response(struct http_client *http_client, struct http_response *response, int fd) {
    struct http_client_config *config = http_client->config;
    char read_chunk[config->bytes_per_read];
    for (;;) {
        ssize_t count = read(fd, read_chunk, config->bytes_per_read);
        if (count < 0) {
            if (errno != EINTR) {
                fprintf(stderr, "%s", strerror(errno));
                return errno;
            }
        }
        if (count == 0) {
            return parse_http_response(http_client, response);
        }
        int err = extend(&http_client->read_buffer, read_chunk, count);
        if (err != 0) {
            return err;
        }
    }
}

void dump_http_response(struct http_response *response) {
    write(STDOUT_FILENO, response->body.data, response->body.length);
}

int dump_header_to_buffer(struct byte_vector *dest_buffer, struct http_header *header) {
    int err = extend_v(dest_buffer, &header->data);
    if (err != 0) {
        return err;
    }
    return extend(dest_buffer, CRLF, sizeof(CRLF) - 1);
}

int dump_headers_to_buffer(struct byte_vector *dest_buffer, struct http_header *headers, size_t headers_num) {
    for (size_t i = 0; i < headers_num; ++i) {
        dump_header_to_buffer(dest_buffer, &headers[i]);

    }
    return extend(dest_buffer, CRLF, sizeof(CRLF) - 1);
}

int dump_http_get_request_to_buffer(const struct http_get_request *request, struct byte_vector *dest) {
    int err = extend(dest, HTTP_METHOD_GET, sizeof(HTTP_METHOD_GET) - 1);
    if (err != 0) {
        return err;
    }
    err = extend_v(dest, &request->path);
    if (err != 0) {
        return err;
    }

    if (request->query_params_num > 0) {
        push_back(dest, '?');
        parse_get_query_parameters(request, dest);
    }

    err = extend(dest, HTTP_VERSION, sizeof(HTTP_VERSION) - 1);
    if (err != 0) {
        return err;
    }

    err = extend(dest, CRLF, sizeof(CRLF) - 1);
    if (err != 0) {
        return err;
    }

    return dump_headers_to_buffer(dest, request->headers, request->headers_num);
}

int dump_http_post_request_to_buffer(const struct http_post_request *request, struct byte_vector *dest) {
    int err = extend(dest, HTTP_METHOD_POST, sizeof(HTTP_METHOD_POST) - 1);
    if (err != 0) {
        return err;
    }
    err = extend_v(dest, &request->path);
    if (err != 0) {
        return err;
    }

    if (request->query_params_num > 0) {
        push_back(dest, '?');
        parse_post_query_parameters(request, dest);
    }

    err = extend(dest, HTTP_VERSION, sizeof(HTTP_VERSION) - 1);
    if (err != 0) {
        return err;
    }

    err = extend(dest, CRLF, sizeof(CRLF) - 1);
    if (err != 0) {
        return err;
    }

    struct http_header content_length_header;
    const char content_length_str[] = "Content-Length";
    char content_len_value[21] = {0};
    int content_len_value_len = snprintf(content_len_value, 20, "%zu", request->body.length);
    if (content_len_value_len < 0) {
        return -1;
    }
    make_http_header(content_length_str,
                     sizeof(content_length_str) - 1,
                     content_len_value,
                     (size_t) content_len_value_len,
                     &content_length_header);
    err = dump_header_to_buffer(dest, &content_length_header);
    if (err != 0) {
        clear_byte_vector(&content_length_header.data);
        return err;
    }

    err = dump_headers_to_buffer(dest, request->headers, request->headers_num);
    if (err != 0) {
        clear_byte_vector(&content_length_header.data);
        return err;
    }
    clear_byte_vector(&content_length_header.data);
    return extend_v(dest, &request->body);
}

int request_http(struct http_client *http_client, struct http_response *response, int fd) {
    size_t written = 0;
    for (;;) {
        ssize_t count = write(fd, &http_client->write_buffer.data[written], http_client->config->bytes_per_write);
        if (count < 0) {
            if (errno != EINTR) {
                fprintf(stderr, "%s", strerror(errno));
                return errno;
            }
        }
        written += count;
        if (written >= http_client->write_buffer.length) {
            return read_http_response(http_client, response, fd);
        }
    }
}

int http_get(struct http_client *http_client, const struct http_get_request *request, struct http_response *response,
             int fd) {
    dump_http_get_request_to_buffer(request, &http_client->write_buffer);
    return request_http(http_client, response, fd);
}

int http_post(struct http_client *http_client, const struct http_post_request *request, struct http_response *response,
              int fd) {
    dump_http_post_request_to_buffer(request, &http_client->write_buffer);
    return request_http(http_client, response, fd);
}

int open_socket(const char *hostname, const char *portname, int *target_desc) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;
    struct addrinfo *res = 0;
    int err = getaddrinfo(hostname, portname, &hints, &res);
    if (err != 0) {
        return err;
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (fd == -1) {
        freeaddrinfo(res);
        return err;
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        freeaddrinfo(res);
        return err;
    }

    freeaddrinfo(res);
    *target_desc = fd;
    return 0;
}
