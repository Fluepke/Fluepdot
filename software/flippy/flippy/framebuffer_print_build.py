#!/bin/env python

# Note: we instantiate the same 'cffi.FFI' class as in the previous
# example, but call the result 'ffibuilder' now instead of 'ffi';
# this is to avoid confusion with the other 'ffi' object you get below
"""Map reduce a framebuffer to box drawing chars efficiently"""

from cffi import FFI
BUILDER = FFI()

BUILDER.set_source("_framebuffer_print",
                   r""" // passed to the real C compiler,
        // contains implementation of things declared in cdef()
        #include <stdint.h>
        #include <stdio.h>
        #include <wchar.h>

        static const unsigned char BitReverseTable256[256] =
        {
        #   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
        #   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
        #   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
            R6(0), R6(2), R6(1), R6(3)
        };


        wchar_t box_drawing_chars[]  = {L' ', L'▖', L'▗', L'▄', L'▘', L'▌',
            L'▚', L'▙', L'▝', L'▞', L'▐', L'▟', L'▀', L'▛', L'▜', L'█' };

        static wchar_t* print_framebuffer_vertical(uint8_t* framebuffer) {
            wchar_t* result = (wchar_t*)malloc(sizeof(wchar_t) * 465);
            for (int x=0; x<58; x++) {
                uint16_t line1 = BitReverseTable256[framebuffer[x * 4]] << 8 |
                    BitReverseTable256[framebuffer[x * 4 + 1]];
                uint16_t line2 = BitReverseTable256[framebuffer[x * 4 + 2]] << 8 |
                    BitReverseTable256[framebuffer[x * 4 + 3]];

                for (int y=0; y<8; y++) {
                    // reduce 2x2 pixels to a single box drawing char
                    uint8_t f = y << 1;
                    uint8_t data = (line1 & (3 << f)) >> f;
                    data <<= 2;
                    data |= (line2 & (3 << f)) >> f;
                    result[x * 8 + y] = box_drawing_chars[data];
                }
            }
            result[464] = 0;
            return result;
        }

        static uint8_t get_pixel_data(uint8_t* framebuffer, int x, int y) {
            return framebuffer[x * 2 + (y / 8)] & (1 << (y % 8));
        }

        static wchar_t* print_framebuffer_horizontal(uint8_t* framebuffer) {
            wchar_t* result = (wchar_t*)malloc(sizeof(wchar_t) * 465);
            for (int y=0; y<8; y++) {
                for (int x=0; x<58; x++) {
                    uint8_t data = (get_pixel_data(framebuffer, x * 2 + 1, y * 2 + 1) ? 1 : 0) +
                           (get_pixel_data(framebuffer, x * 2    , y * 2 + 1) ? 2 : 0) +
                           (get_pixel_data(framebuffer, x * 2    , y * 2    ) ? 4 : 0) +
                           (get_pixel_data(framebuffer, x * 2 + 1, y * 2    ) ? 8 : 0);
                    result[y * 58 + x] = box_drawing_chars[data];
                }
            }
            result[464] = 0;
            return result;
        }

    """,
                   libraries=[])   # or a list of libraries to link with
# (more arguments like setup.py's Extension class:
# include_dirs=[..], extra_objects=[..], and so on)

BUILDER.cdef("""
    // declarations that are shared between Python and C
    static wchar_t* print_framebuffer_vertical(uint8_t* framebuffer);
    static wchar_t* print_framebuffer_horizontal(uint8_t* framebuffer);
    void free(void *ptr);
""")

if __name__ == "__main__":
    BUILDER.compile(verbose=True)
