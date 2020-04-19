#include "flipdot_rendering_options.h"

#include <string.h>

#include "flipdot_util.h"

static const char* TAG = "rendering_options.c";

flipdot_rendering_options_t* flipdot_rendering_options_initialize(uint8_t panel_count, uint8_t width)
{
    flipdot_rendering_options_t* rendering_options = NULL;
    if ((panel_count == 0) || (width == 0)) {
        return NULL;
    }

    rendering_options = calloc(1, sizeof(flipdot_rendering_options_t));
    if (rendering_options == NULL) {
        return NULL;
    }

    rendering_options->width = width;
    rendering_options->panel_count = panel_count;

    rendering_options->delay_options = calloc(width, sizeof(flipdot_rendering_delay_options_t));
    if (rendering_options->delay_options == NULL) {
        free(rendering_options);
        return NULL;
    }

    rendering_options->panel_order = calloc(panel_count, sizeof(uint8_t));
    if (rendering_options->panel_order == NULL) {
        free(rendering_options->delay_options);
        rendering_options->delay_options = NULL;
        free(rendering_options);
        return NULL;
    }

    for (uint8_t i=0; i<panel_count; i++) {
        rendering_options->panel_order[i] = i;
    }

    for (uint8_t x=0; x<width; x++) {
        rendering_options->delay_options[x].pre_delay = FLIPDOT_RENDERING_OPTIONS_PRE_DELAY_DEFAULT;
        rendering_options->delay_options[x].set_delay = FLIPDOT_RENDERING_OPTIONS_SET_DELAY_DEFAULT;
        rendering_options->delay_options[x].clear_delay = FLIPDOT_RENDERING_OPTIONS_CLEAR_DELAY_DEFAULT;
    }

    return rendering_options;
}

esp_err_t flipdot_rendering_options_copy(flipdot_rendering_options_t* dest, flipdot_rendering_options_t* src) {
    FLIPDOT_ASSERT_NOT_NULL(src, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(dest, ESP_ERR_INVALID_ARG);

    if (src->panel_count == 0) {
        FLIPDOT_LOGE(TAG, "panel_count must not be 0");
        return ESP_ERR_INVALID_ARG;
    }

    if (dest->panel_order != NULL) { free(dest->panel_order); }
    dest->panel_order = calloc(src->panel_count, sizeof(uint8_t));
    if (dest->panel_order == NULL) {
        FLIPDOT_LOGE(TAG, "calloc failed");
        return ESP_ERR_NO_MEM;
    }

    if (dest->delay_options != NULL) { free(dest->delay_options); }
    dest->delay_options = calloc(src->width, sizeof(flipdot_rendering_delay_options_t));
    if (dest->delay_options == NULL) {
        FLIPDOT_LOGE(TAG, "calloc failed");
        free(dest->panel_order);
        dest->panel_order = NULL;
        return ESP_ERR_NO_MEM;
    }

    dest->width = src->width;
    dest->panel_count = src->panel_count;
    dest->mode = src->mode;

    memcpy(dest->panel_order,
            src->panel_order,
            src->panel_count);
    memcpy(dest->delay_options,
            src->delay_options,
            src->width * sizeof(flipdot_rendering_delay_options_t));

    return ESP_OK;
}

void flipdot_rendering_options_free(flipdot_rendering_options_t* rendering_options) {
    if (rendering_options == NULL) { return; }
    free(rendering_options->delay_options);
    rendering_options->delay_options = NULL;
    free(rendering_options->panel_order);
    rendering_options->panel_order = NULL;
    free(rendering_options);
    rendering_options = NULL;
}
