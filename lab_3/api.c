#include "http.h"
#include "api.h"

int fetch_time(struct api_client *client, enum QUERY_METHOD method, const char *type, const char *format,
               struct http_response *resp) {

    struct query_parameter query_parameters[2] = {
            {.name="type", .name_len=sizeof("type") - 1, .value = type, .value_len = strlen(type)},
            {.name="format", .name_len=sizeof("format") - 1, .value=format, .value_len =strlen(format)},
    };

    struct http_client *httpc = client->httpc;

    int fd = 0;
    int err = open_socket(client->hostname, client->portname, &fd);

    if (err != 0) {
        return err;
    }

    const char path_str[] = "/WebApi/time";
    struct byte_vector path;
    init_byte_vector(sizeof(path_str), &path);
    extend(&path, path_str, sizeof(path_str) - 1);

    struct http_header host_header;
    make_http_header("Host", 4, client->hostname, strlen(client->hostname), &host_header);

    struct http_header connection_header;
    make_http_header("Connection", 10, "Close", 5, &connection_header);

    struct http_header accept_header;
    make_http_header("Accept", 6, "application/json", sizeof("application/json") - 1, &accept_header);

    init_http_response(httpc, resp);

    if (method == GET) {
        struct http_header headers[] = {
                host_header,
                connection_header,
                accept_header
        };

        struct http_get_request req = {
                .path = path,
                .headers =headers,
                .headers_num = sizeof(headers) / sizeof(struct http_header),
                .query_params = query_parameters,
                .query_params_num = sizeof(query_parameters) / sizeof(struct query_parameter),
        };

        err = http_get(httpc, &req, resp, fd);

        clear_http_get_request(&req);

    } else if (method == POST) {
        struct http_header content_type_header;
        make_http_header("Content-Type",
                         sizeof("Content-Type") - 1,
                         "application/x-www-form-urlencoded",
                         sizeof("application/x-www-form-urlencoded") - 1,
                         &content_type_header);

        struct http_header headers[] = {
                host_header,
                connection_header,
                accept_header,
                content_type_header
        };

        struct http_post_request req;
        init_http_post_request(httpc, &req);

        req.path = path;
        req.headers = headers;
        req.headers_num = sizeof(headers) / sizeof(struct http_header);
        req.query_params = query_parameters;
        req.query_params_num = sizeof(query_parameters) / sizeof(struct query_parameter);

        parse_post_query_parameters(&req, &req.body);

        req.query_params_num = 0;
        err = http_post(httpc, &req, resp, fd);
        clear_http_post_request(&req);

    } else {
        err = -228;
    }
    close(fd);
    if (err != 0) {
        clear_http_response(resp);
    }
    return err;
}
