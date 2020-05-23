========
HTTP API
========

Framebuffer
===========

For simple integration into your projects, the *fluepboard* firmwares ships with a simple to use HTTP API.

Framebuffer encoding
--------------------

Framebuffers are **ASCII**-encoded. The only allowed characters are **<space>** ``0x20``, **X** ``0x58`` and **``\\n``** ``\0x0A``.

A **bright** (set) pixel is encoded as an X.

A **dark** (cleared) pixel is encoded as a space character.

Each line in a framebuffer has **exactly** the same amount of characters, which is **exactly** the *fluepdot's* width plus one newline character.

Each framebuffer has **exactly** 16 lines.

Lines are terminated by a single ``\n``. No carriage return is used.

Framebuffer manipulations
-------------------------

GET /framebuffer
^^^^^^^^^^^^^^^^
Params
    GET
        **None**
    POST
        **None**

Gets the current framebuffer encoded as explained above.
This endpoint can be used to calculate the framebuffer dimensions, thus there is no seperate endpoint for retrieving the geometry.

POST /framebuffer
^^^^^^^^^^^^^^^^^

Params
    GET
        **None**
    POST
        Raw framebuffer encoded as explained above

Draw the posted framebuffer to the *fluepdot*.

GET /pixel
^^^^^^^^^^

Params
    GET
        x - The x coordinate
        y - The y coordinate

Get the current pixel at the given coordinate encoded as above


POST /pixel
^^^^^^^^^^^

Params
    GET
        x
            The x coordinate (ascii encoded decimal value)
        y
            The y coordinate (ascii encoded decimal value)

Sets the pixel at the given coordinate to bright.

DELETE /pixel
^^^^^^^^^^^^^
Params
    GET
        x
            The x coordinate (ascii encoded decimal value)
        y
            The y coordinate (ascii encoded decimal value)

Set the pixel at the give coordinate to dark.

Rendering options
=================

Rendering mode
--------------

GET /rendering/mode
^^^^^^^^^^^^^^^^^^^

Retuns an ASCII printed integer which value is defined as follows:

    .. doxygenenum:: flipdot_rendering_mode_t
       :project: fluepdot

PUT /rendering/mode
^^^^^^^^^^^^^^^^^^^
Params
    GET
        **None**
    POST
        ASCII printed integer, which value is to be interpreted as stated above.

Rendering timings encoding
--------------------------

Rendering timings are **ASCII**-encoded. The only allowed characters are 0-9 (``0x30-0x39``) and ``\\n``.

For each *fluepdot* column, there are **exactly** 3 rows:

1. **Pre delay**
   
    How long to wait (50 microseconds steps) before rendering to the column.

2. **Clear delay**

    How long to power the coils in order to clear the column (in 50 microseconds steps).

3. **Set delay**

    How long to power the coils in order to set the column (in 50 microseconds steps)

Each line has **exactly** 5 characters and one trailing ``\\n``. You have to pad with zeros.
Each line contains the decimal value in ascii-encoded form.

.. warning::

   Powering a coil repeatedly for a long duration *might* cause the coil to overheat and or fail.
   Decreasing the timings might result in higher **framerate**, but might result in not all pixels flipping.

   Usually **1600uS** are enough to reliably flip all pixels.
   This is the **default**.

GET /rendering/timings
^^^^^^^^^^^^^^^^^^^^^^

Returns the timing configuration encoded as explained above.

POST /rendering/timings
^^^^^^^^^^^^^^^^^^^^^^^

Params
    GET
        None
    POST
        Timing configuration as explained above.

Set the timing configuration.
