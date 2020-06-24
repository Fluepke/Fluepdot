#include "flipdot.h"

#include "string.h"
#include "esp_log.h"

static const char* TAG = "framebuffer.c";

esp_err_t flipdot_framebuffer_init(framebuffer_t* framebuffer, uint8_t width) {
    FLIPDOT_ASSERT_NOT_NULL(framebuffer, ESP_ERR_INVALID_ARG);
    if (width == 0) {
        ESP_LOGE(TAG, "Framebuffer must have width > 0");
        return ESP_ERR_INVALID_ARG;
    }

    framebuffer->width = width;
    framebuffer->columns = calloc(width, sizeof(uint16_t));
    if (framebuffer->columns == NULL) {
        FLIPDOT_LOGE(TAG, "calloc failed");
        return ESP_ERR_NO_MEM;;
    }

    return ESP_OK;
}

void flipdot_framebuffer_clear(framebuffer_t* framebuffer) {
    bzero(framebuffer->columns, framebuffer->width * sizeof(uint16_t));
}

bool flipdot_framebuffer_get_pixel(framebuffer_t* framebuffer, uint8_t x, uint8_t y) {
    FLIPDOT_ASSERT_NOT_NULL(framebuffer, false);

    if ((x >= framebuffer->width) || (y > 16)) {
        return false;
    }

    uint16_t column = framebuffer->columns[x];

    uint8_t offset = 0;
    if (y >= 8) {
        offset = 15 - (y % 8);
    } else {
        offset = 7 - y;
    }
    return column & (1 << offset);
}

esp_err_t flipdot_framebuffer_set_pixel(framebuffer_t* framebuffer, uint8_t x, uint8_t y, bool value) {
    FLIPDOT_ASSERT_NOT_NULL(framebuffer, ESP_ERR_INVALID_ARG);

    if ((x >= framebuffer->width) || (y > 16)) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t offset = 0;
    if (y >= 8) {
        offset = 15 - (y % 8);
    } else {
        offset = 7 - y;
    }

    if (value) {
        framebuffer->columns[x] |= (1 << offset);
    } else {
        framebuffer->columns[x] &= ~(1 << offset);
    }
    return ESP_OK;
}

char flipdot_framebuffer_encode_pixel(bool value) {
    return value ? FLIPDOT_PIXEL_SET : FLIPDOT_PIXEL_CLEAR;
}

char* flipdot_framebuffer_encode_line(framebuffer_t* framebuffer, char* dst, size_t dst_len, uint8_t y) {
    FLIPDOT_ASSERT_NOT_NULL(framebuffer, NULL);

    for (uint8_t x=0; x < MIN(framebuffer->width, dst_len - 1); x++) {
        dst[x] = flipdot_framebuffer_encode_pixel(
                flipdot_framebuffer_get_pixel(framebuffer, x, y));
    }
    dst[dst_len - 1] = 0;

    if (dst_len - 1 < framebuffer->width) {
        return NULL;
    }
    return dst;
}

bool flipdot_framebuffer_decode_pixel(char src) {
    return src == FLIPDOT_PIXEL_SET;
}

esp_err_t flipdot_framebuffer_printf(framebuffer_t* framebuffer) {
    FLIPDOT_ASSERT_NOT_NULL(framebuffer, ESP_ERR_INVALID_ARG);
    
    char* line_buf = malloc(framebuffer->width + 1);
    if (!line_buf) {
        return ESP_ERR_NO_MEM;
    }
    for (int y=15; y>=0; y--) {
        flipdot_framebuffer_encode_line(framebuffer, line_buf,  framebuffer->width + 1, y);
        printf("%s\n", line_buf);
    }
    free(line_buf);

    return ESP_OK;
}

esp_err_t flipdot_framebuffer_decode_line(framebuffer_t* framebuffer, char* src, size_t src_len, uint8_t y) {
    FLIPDOT_ASSERT_NOT_NULL(framebuffer, ESP_ERR_INVALID_ARG);

    if (framebuffer->width != src_len - 1) {
        return ESP_ERR_INVALID_ARG;
    }

    for (uint8_t x=0; x<MIN(framebuffer->width, src_len - 1); x++) {
        flipdot_framebuffer_set_pixel(framebuffer, x, y, flipdot_framebuffer_decode_pixel(src[x]));
    }

    return ESP_OK;
}

esp_err_t flipdot_framebuffer_copy(framebuffer_t* dest, framebuffer_t* src) {
    FLIPDOT_ASSERT_NOT_NULL(dest, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(src, ESP_ERR_INVALID_ARG);

    if (src->width == 0) { 
        FLIPDOT_LOGE(TAG, "framebuffer src width must not be 0");
        return ESP_ERR_INVALID_ARG;
    }

    if (dest->columns != NULL) { free(dest->columns); }

    dest->columns = calloc(src->width, sizeof(uint16_t));
    FLIPDOT_ASSERT_NOT_NULL(dest->columns, ESP_ERR_NO_MEM);

    memcpy(dest->columns,
            src->columns,
            src->width * sizeof(uint16_t));

    dest->width = src->width;

    return ESP_OK;
}

esp_err_t flipdot_framebuffer_compare(framebuffer_t* a, framebuffer_t* b, unsigned int* diff) {
    return flipdot_framebuffer_compare_partial(a, b, 0, a->width, diff);
}

esp_err_t flipdot_framebuffer_compare_partial(framebuffer_t* a, framebuffer_t* b, uint8_t start_x, uint8_t width, unsigned int* diff) {
    FLIPDOT_ASSERT_NOT_NULL(a, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(b, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(diff, ESP_ERR_INVALID_ARG);

    if (a->width != b->width) { return ESP_ERR_INVALID_ARG; }
    if (width == 0) { return ESP_ERR_INVALID_ARG; }
    if (start_x + width > a->width) { return ESP_ERR_INVALID_ARG; }

    *diff = 0;

    for (uint8_t x=0; x<width; x++) {
        *diff += __builtin_popcount(
                (a->columns[x + start_x]) ^ (b->columns[x + start_x]));
    }

    return ESP_OK;
}

esp_err_t flipdot_framebuffer_compare_column(framebuffer_t* a, framebuffer_t* b, uint8_t x, unsigned int* diff) {
    FLIPDOT_ASSERT_NOT_NULL(a, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(b, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(diff, ESP_ERR_INVALID_ARG);

    if ((a->width <= x) || (b->width <= x)) { return ESP_ERR_INVALID_ARG; }

    *diff = __builtin_popcount(a->columns[x] ^ b->columns[x]);

    return ESP_OK;
}

void flipdot_framebuffer_free(framebuffer_t* framebuffer) {
    if (framebuffer == NULL) { return; }
    free(framebuffer->columns);
    framebuffer->columns = NULL;
    free(framebuffer);
    framebuffer = NULL;
}
