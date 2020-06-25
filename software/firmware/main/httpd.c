#include "esp_log.h"

#include "httpd.h"
#include "flipdot.h"
#include "main.h"
#include "util.h"
#include "font_rendering.h"

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

esp_err_t http_api_get_rendering_mode(httpd_req_t *req) {
    char response[3] = {};
    snprintf(response, sizeof(response), "%d\n", flipdot.rendering_options->mode);
    return httpd_resp_send(req, response, sizeof(response));
}

static const httpd_uri_t get_rendering_mode = {
    .uri = "/rendering/mode",
    .method = HTTP_GET,
    .handler = http_api_get_rendering_mode,
};

static esp_err_t http_api_put_rendering_mode(httpd_req_t *req) {
    size_t received_bytes = 0;
    char* buf;
    esp_err_t error = ESP_OK;

    if (req->content_len > 10) {
        return ESP_ERR_INVALID_ARG;
    }
    
    buf = calloc(req->content_len + 1, sizeof(char));

    if (buf == NULL) {
        return ESP_ERR_NO_MEM;
    }

    while (received_bytes < req->content_len) {
        int ret = httpd_req_recv(req, &buf[received_bytes], req->content_len - received_bytes);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }

            error = ESP_FAIL;
            goto ERROR;
        }
        received_bytes += ret;
    }
    buf[req->content_len] = 0;
        
    int rendering_mode = atoi(buf);
    
    switch (rendering_mode) {
        case FULL:
        case DIFFERENTIAL:
            flipdot.rendering_options->mode = rendering_mode;
            break;
        default:
            error = ESP_ERR_INVALID_ARG;
            goto ERROR;
    }

    return http_api_get_rendering_mode(req);

ERROR:
    free(buf);
    return error;
}

static const httpd_uri_t put_rendering_mode = {
    .uri = "/rendering/mode",
    .method = HTTP_PUT,
    .handler = http_api_put_rendering_mode,
};

esp_err_t http_api_get_rendering_timings(httpd_req_t *req) {
    char buf[19];

    for (int x=0; x<flipdot.width; x++) {
        snprintf(buf,
                sizeof(buf),
                "%05d\n%05d\n%05d\n",
                flipdot.rendering_options->delay_options[x].pre_delay,
                flipdot.rendering_options->delay_options[x].clear_delay,
                flipdot.rendering_options->delay_options[x].set_delay);
        httpd_resp_send_chunk(req, buf, sizeof(buf));
    }
    
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t get_rendering_timings = {
    .uri = "/rendering/timings",
    .method = HTTP_GET,
    .handler = http_api_get_rendering_timings,
};

esp_err_t http_api_post_rendering_timings(httpd_req_t *req) {
    char buf[19];

    for (int x=0; x<flipdot.width; x++) {
        size_t received = 0;
        while (received < sizeof(buf)) {
            int ret = httpd_req_recv(req, &buf[received], sizeof(buf) - received);
            if (ret <= 0) {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                    httpd_resp_send_408(req);
                }
                return ESP_FAIL;
            }
            received += ret;
        }
        if (sscanf(buf, "%05hd\n%05hd\n%05hd",
                    &(flipdot.rendering_options->delay_options[x].pre_delay),
                    &(flipdot.rendering_options->delay_options[x].clear_delay),
                    &(flipdot.rendering_options->delay_options[x].set_delay)) == 0) {
            return ESP_ERR_INVALID_ARG;
        }
    }

    return http_api_get_rendering_timings(req);
}

static const httpd_uri_t post_rendering_timings = {
    .uri = "/rendering/timings",
    .method = HTTP_POST,
    .handler = http_api_post_rendering_timings,
};

static esp_err_t http_api_get_fonts(httpd_req_t *req) {
    const struct mf_font_list_s* font_list = mf_get_font_list();

    while (font_list) {
        httpd_resp_send_chunk(req, font_list->font->full_name, strlen(font_list->font->full_name));
        httpd_resp_send_chunk(req, "\n", 1);
        httpd_resp_send_chunk(req, font_list->font->short_name, strlen(font_list->font->short_name));
        httpd_resp_send_chunk(req, "\n", 1);
        font_list = font_list->next;
    }

    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static const httpd_uri_t get_fonts = {
    .uri = "/fonts",
    .method = HTTP_GET,
    .handler = http_api_get_fonts,
};

static esp_err_t http_api_post_framebuffer_text(httpd_req_t *req) {
    // receive posted text
    size_t received_bytes = 0;
    esp_err_t error = ESP_OK;

    if (req->content_len > 64) {
        return ESP_ERR_INVALID_ARG;
    }
    
    char* text = calloc(req->content_len + 1, sizeof(char));
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    char* buf = malloc(buf_len);

    if (text == NULL || buf == NULL) {
        return ESP_ERR_NO_MEM;
    }

    while (received_bytes < req->content_len) {
        int ret = httpd_req_recv(req, &text[received_bytes], req->content_len - received_bytes);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }

            error = ESP_FAIL;
            goto ERROR;
        }
        received_bytes += ret;
    }
    text[req->content_len] = 0;

    // read GET parameters
    char param_font[32], param_x[4], param_y[4] = { 0 };
    uint8_t x = 0, y = 0;

    error = httpd_req_get_url_query_str(req, buf, buf_len);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "httpd_req_get_url_query_str returned %d", error);
        goto ERROR; }
    error = httpd_query_key_value(buf, "font", param_font, sizeof(param_font));
    if (error != ESP_OK) {
        if (error == ESP_ERR_NOT_FOUND) {
            strcpy(param_font, "DejaVuSans");
        } else { goto ERROR; }
    }
    font_rendering_state_t state = {
        .font = mf_find_font(param_font),
        .framebuffer = flipdot.framebuffer,
    };

    if (state.font == NULL) { 
        ESP_LOGE(TAG, "Could not find font %s", param_font);
        return ESP_ERR_INVALID_ARG;
    }

    error = httpd_query_key_value(buf, "x", param_x, sizeof(param_x));
    if (error != ESP_OK && error != ESP_ERR_NOT_FOUND) { goto ERROR; }
    x = atoi(param_x);
    error = httpd_query_key_value(buf, "y", param_y, sizeof(param_y));
    if (error != ESP_OK && error != ESP_ERR_NOT_FOUND) { goto ERROR; }
    y = atoi(param_y);

    // clear flipdot
    memset(flipdot.framebuffer->columns, 0, 2 * flipdot.width);

    mf_render_justified(state.font, x, y, flipdot.width, text, 0, character_callback, (void*)&state);

    flipdot_set_dirty_flag(&flipdot);

    free(buf);

    return http_api_get_framebuffer(req);

ERROR:
    free(buf);
    return error;
}

static const httpd_uri_t post_framebuffer_text = {
    .uri = "/framebuffer/text",
    .method = HTTP_POST,
    .handler = http_api_post_framebuffer_text,
};


esp_err_t httpd_initialize(httpd_handle_t server) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.max_uri_handlers = 16;

    if (httpd_start(&server, &config) == ESP_OK) {
        ERROR_CHECK(httpd_register_uri_handler(server, &get_framebuffer));
        ERROR_CHECK(httpd_register_uri_handler(server, &post_framebuffer));
        ERROR_CHECK(httpd_register_uri_handler(server, &get_pixel));
        ERROR_CHECK(httpd_register_uri_handler(server, &delete_pixel));
        ERROR_CHECK(httpd_register_uri_handler(server, &post_pixel));
        ERROR_CHECK(httpd_register_uri_handler(server, &put_rendering_mode));
        ERROR_CHECK(httpd_register_uri_handler(server, &get_rendering_mode));
        ERROR_CHECK(httpd_register_uri_handler(server, &get_rendering_timings));
        ERROR_CHECK(httpd_register_uri_handler(server, &post_rendering_timings));
        ERROR_CHECK(httpd_register_uri_handler(server, &get_fonts));
        ERROR_CHECK(httpd_register_uri_handler(server, &post_framebuffer_text));
    }

    return ESP_OK;
}
