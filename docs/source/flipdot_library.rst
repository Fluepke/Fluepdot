Flipdot API reference
=====================

Flipdot
-------

Flipdot structs
^^^^^^^^^^^^^^^

.. doxygenstruct:: flipdot_t
   :project: fluepdot
   :members:

.. doxygenstruct:: flipdot_panel_t
   :project: fluepdot
   :members:

.. doxygenstruct:: flipdot_configuration_t
   :project: fluepdot
   :members:

Flipdot functions
^^^^^^^^^^^^^^^^^

.. doxygenfunction:: flipdot_initialize
   :project: fluepdot

.. doxygenfunction:: flipdot_set_power
   :project: fluepdot

.. doxygenfunction:: flipdot_get_power
   :project: fluepdot

.. doxygenfunction:: flipdot_set_dirty_flag
   :project: fluepdot

Framebuffer
-----------

Framebuffer struct
^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: framebuffer_t
   :project: fluepdot
   :members:

.. doxygenfunction:: flipdot_framebuffer_init
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_free
   :project: fluepdot

Framebuffer manipulation
^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: flipdot_framebuffer_clear
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_get_pixel
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_set_pixel
   :project: fluepdot

Framebuffer de/encoding
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: flipdot_framebuffer_encode_line
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_decode_line
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_encode_pixel
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_decode_pixel
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_printf
   :project: fluepdot

Framebuffer diffing
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: flipdot_framebuffer_compare
   :project: fluepdot

.. doxygenfunction:: flipdot_framebuffer_compare_partial
   :project: fluepdot

Rendering options
-----------------

.. doxygenstruct:: flipdot_rendering_options_t
   :project: fluepdot
   :members:

.. doxygenstruct:: flipdot_rendering_delay_options_t
   :project: fluepdot
   :members:

.. doxygenenum:: flipdot_rendering_mode_t
   :project: fluepdot

.. doxygenfunction:: flipdot_rendering_options_initialize
   :project: fluepdot

.. doxygenfunction:: flipdot_rendering_options_copy
   :project: fluepdot

.. doxygenfunction:: flipdot_rendering_options_free
   :project: fluepdot
