#include "esp_log.h"

#include "httpd.h"
#include "flipdot.h"
#include "main.h"
#include "util.h"


static const char* TAG = "httpd.c";

static esp_err_t http_api_get_framebuffer(httpd_req_t *req) {
    size_t buf_len = flipdot.width + 1;
    char* buf = calloc(buf_len, sizeof(char));
    if (buf == NULL) { return ESP_ERR_NO_MEM; }

    for (int y=15; y>=0; y--) {
        flipdot_framebuffer_encode_line(flipdot.framebuffer, buf, buf_len, y);
        buf[buf_len - 1] = '\n';
        httpd_resp_send_chunk(req, buf, buf_len);
    }

    httpd_resp_send_chunk(req, NULL, 0);

    free(buf);
    return ESP_OK;
}

static const httpd_uri_t get_framebuffer = {
    .uri = "/framebuffer",
    .method = HTTP_GET,
    .handler = http_api_get_framebuffer,
};

static esp_err_t http_api_post_framebuffer(httpd_req_t *req) {
    size_t buf_len = flipdot.width + 1;
    char* buf = calloc(buf_len, sizeof(char));
    if (buf == NULL) { return ESP_ERR_NO_MEM; }

    int remaining = req->content_len;
    ESP_LOGI(TAG, "content len: %d", remaining);

    for (int y=15; y>=0; y--) {
        // assert that there is at least one line waiting to be read
        if (remaining < flipdot.width + 1) {
            ESP_LOGE(TAG, "Framebuffer was malformed, I was expecting more data");
            free(buf);
            return ESP_ERR_INVALID_ARG;
        }

        // read data of the request until we have filled the buf
        int received_bytes = 0;
        while (received_bytes < flipdot.width + 1) {
            ESP_LOGI(TAG, "Trying to receive %d bytes", flipdot.width + 1 - received_bytes);
            int ret = httpd_req_recv(req, &buf[received_bytes], flipdot.width + 1 - received_bytes);
            if (ret <= 0) {
                ESP_LOGE(TAG, "Framebuffer receiving timed out or errored: %d", ret);
                if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                    httpd_resp_send_408(req);
                }

                free(buf);
                return ESP_FAIL;
            }
            received_bytes += ret;
        }

        // assert that the last char we read was a \n
        if (buf[flipdot.width] != '\n') {
            free(buf);
            return ESP_ERR_INVALID_ARG;
        }

        buf[flipdot.width] = 0;

        // decode the received line
        esp_err_t error = flipdot_framebuffer_decode_line(flipdot.framebuffer, buf, buf_len, y);
        if (error != ESP_OK) {
            ESP_LOGE(TAG, "flipdot_framebuffer_decode_line returned %s", esp_err_to_name(error));
            free(buf);
            return error;
        }
    }

    free(buf);

    flipdot_set_dirty_flag(&flipdot);

    return http_api_get_framebuffer(req);
}

static const httpd_uri_t post_framebuffer = {
    .uri = "/framebuffer",
    .method = HTTP_POST,
    .handler = http_api_post_framebuffer,
};

static esp_err_t http_api_pixel_handler(httpd_req_t *req) {
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;

    if (buf_len <= 1) {
        return ESP_ERR_INVALID_ARG;
    }

    char* buf = malloc(buf_len);

    if (buf == NULL) {
        return ESP_ERR_NO_MEM;
    }

    char param_x[4], param_y[4];
    uint8_t x = 0, y = 0;
    esp_err_t error;

    error = httpd_req_get_url_query_str(req, buf, buf_len);
    if (error != ESP_OK) { goto ERROR; }
    
    error = httpd_query_key_value(buf, "x", param_x, sizeof(param_x));
    if (error != ESP_OK) { goto ERROR; }
    x = atoi(param_x);
    
    error = httpd_query_key_value(buf, "y", param_y, sizeof(param_y));
    if (error != ESP_OK) { goto ERROR; }
    y = atoi(param_y);

    switch (req->method) {
        case HTTP_DELETE:
            error = flipdot_framebuffer_set_pixel(flipdot.framebuffer, x, y, 0);
            flipdot_set_dirty_flag(&flipdot);
            break;
        case HTTP_POST:
            error = flipdot_framebuffer_set_pixel(flipdot.framebuffer, x, y, 1);
            flipdot_set_dirty_flag(&flipdot);
    }

    if (error != ESP_OK) { goto ERROR; }

    char response[2] = {};
    response[0] = flipdot_framebuffer_encode_pixel(
            flipdot_framebuffer_get_pixel(flipdot.framebuffer, x, y));

    error = httpd_resp_send(req, response, sizeof(response));
ERROR:
    free(buf);
    return error;
}

static const httpd_uri_t get_pixel = {
    .uri = "/pixel",
    .method = HTTP_GET,
    .handler = http_api_pixel_handler,
};

static const httpd_uri_t delete_pixel = {
    .uri = "/pixel",
    .method = HTTP_DELETE,
    .handler = http_api_pixel_handler,
};

static const httpd_uri_t post_pixel = {
    .uri = "/pixel",
    .method = HTTP_POST,
    .handler = http_api_pixel_handler,
};

esp_err_t httpd_initialize(httpd_handle_t server) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        ERROR_CHECK(httpd_register_uri_handler(server, &get_framebuffer));
        ERROR_CHECK(httpd_register_uri_handler(server, &post_framebuffer));
        ERROR_CHECK(httpd_register_uri_handler(server, &get_pixel));
        ERROR_CHECK(httpd_register_uri_handler(server, &delete_pixel));
        ERROR_CHECK(httpd_register_uri_handler(server, &post_pixel));
    }

    return ESP_OK;
}
