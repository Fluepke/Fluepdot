#include "cmd_font_rendering.h"

#include "esp_console.h"
#include "argtable3/argtable3.h"

#include "main.h"
#include "font_rendering.h"

static int do_show_fonts(int argc, char **argv) {
    const struct mf_font_list_s* font_list = mf_get_font_list();

    printf("Installed fonts:\n");
    while (font_list)
    {
        printf("* '%s'\n", font_list->font->full_name);
        printf("  * shortname: '%s'\n", font_list->font->short_name);
        printf("  * geometry: %dx%d\n", font_list->font->width, font_list->font->height);
        font_list = font_list->next;
    }

    return 0;
}

esp_err_t console_register_show_fonts(void) {
    const esp_console_cmd_t show_fonts_cmd = {
        .command = "show_fonts",
        .help = "List installed fonts",
        .func = &do_show_fonts,
    };

    return esp_console_cmd_register(&show_fonts_cmd);
}

static struct {
    struct arg_int *x;
    struct arg_int *y;
    struct arg_str *font_name;
    struct arg_str *text;
    struct arg_end *end;
} render_font_args;

static int do_render_font(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&render_font_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, render_font_args.end, argv[0]);
        return 1;
    }

    int x = 0, y = 0;

    const struct mf_font_s* font = mf_get_font_list()->font;

    if (render_font_args.x->count > 0) {
        x = render_font_args.x->ival[0];
    }
    if (render_font_args.y->count > 0) {
        y = render_font_args.y->ival[0];
    }
    if (render_font_args.font_name->count > 0) {
        font = mf_find_font(render_font_args.font_name->sval[0]);
    }

    if (flipdot.framebuffer == NULL) {
        printf("Dafuq\n");
        return 1;
    }

    font_rendering_state_t state = {
        .font = font,
        .framebuffer = flipdot.framebuffer,
    };

    mf_render_justified(font, x, y, flipdot.width, render_font_args.text->sval[0],
            0, character_callback, (void*)&state);

    flipdot_set_dirty_flag(&flipdot);

    return 0;
}

esp_err_t console_register_render_font(void) {
    render_font_args.x = arg_int0("x", "X", "<int>", "Depending on aligned, either left, center or right edge of target.");
    render_font_args.y = arg_int0("y", "Y", "<int>", "Upper edge of the target area.");
    render_font_args.font_name = arg_str0("f", "font", "<font>", "Name of font to use");
    render_font_args.text = arg_str1(NULL, NULL, "<text>", "Text to display");
    render_font_args.end = arg_end(1);

    const esp_console_cmd_t render_font_cmd = {
        .command = "render_font",
        .help = "Render some text given some font",
        .func = &do_render_font,
        .argtable = &render_font_args,
    };

    return esp_console_cmd_register(&render_font_cmd);
}
