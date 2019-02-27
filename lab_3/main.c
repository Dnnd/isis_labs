#include <stdio.h>
#include "http.h"
#include "api.h"

struct query_configuration {
    enum QUERY_METHOD query_method;
    const char *type;
    const char *format;
};


struct query_configuration parse_args(int argc, char **argv) {
    struct query_configuration conf = {
            .query_method = GET,
            .type = "local",
            .format = "unix",
    };
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            conf.type = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            conf.format = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-post") == 0) {
            conf.query_method = POST;
        }
    }
    return conf;
}

int main(int argc, char **argv) {

    struct query_configuration query_params = parse_args(argc, argv);

    struct http_client_config config = {
            .bytes_per_read = 256,
            .bytes_per_write = 256,
            .initial_get_response_headers_size = 256,
            .initial_get_response_body_size = 1024,
            .initial_post_response_body_size = 1024,
            .initial_post_response_headers_size = 1024,
    };

    struct http_client client;
    init_http_client(&client, &config);
    struct api_client api = {
            .portname = "8090",
            .hostname = "www.iu3.bmstu.ru",
            .httpc = &client,
    };
    struct http_response resp;
    fetch_time(&api, query_params.query_method, query_params.type, query_params.format, &resp);
    if (resp.status_code != 200) {
        printf("non-200 status code: %d\n", resp.status_code);
    } else {
        dump_http_response(&resp);
    }
    clear_http_response(&resp);
    clear_http_client(&client);
    return 0;
}
