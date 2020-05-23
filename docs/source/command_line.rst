Commandline interface
=====================

For debugging and configuration purposes the *fluepboard* firmware provides a simple CLI.

The serial interface is available by connecting the *fluepboard* to a computer via USB.

Serial interface works at 115200/8-N-1 [#]_.


.. note::
   
    After performing configuration changes, please issue a ``config_save`` followed by a ``reboot`` in order to apply the configuration.

Available commands
------------------

Type ``help`` to get a list of available commands:

.. code:: bash

    help
      Print the list of registered commands
    
    ping  [-W <t>] [-i <t>] [-s <n>] [-c <n>] [-Q <n>] <host>
      send ICMP ECHO_REQUEST to network hosts
      -W, --timeout=<t>  Time to wait for a response, in seconds
      -i, --interval=<t>  Wait interval seconds between sending each packet
      -s, --size=<n>  Specify the number of data bytes to be sent
      -c, --count=<n>  Stop after sending count packets
      -Q, --tos=<n>  Set Type of Service related bits in IP datagrams
            <host>  Host address
    
    host  <host>
      Perform a DNS lookup
            <host>  Host to look up
    
    traceroute  <host>
      Perform a traceroute
            <host>  Host to traceroute
    
    reboot
      Perform a software reset
    
    show_version
      Get version of chip and SDK
    
    show_tasks
      Get information about running tasks
    
    config_save
      Save the current system configuration to flash
    
    config_load
      Load the system configuration from flash
    
    config_show
      Show the current system configuration
    
    config_reset
      Factory reset the system configuration
    
    config_wifi_ap  <ssid> [<password>]
      Configure AP mode
            <ssid>  WiFi SSID
        <password>  WiFi Password
    
    config_wifi_station  <ssid> [<password>]
      Configure station mode
            <ssid>  WiFi SSID
        <password>  WiFi Password
    
    config_hostname  <hostname>
      Set system hostname
        <hostname>  Hostname
    
    config_panel_layout  <panel_size> [<panel_size>]...
      Configure panel count and sizes
      <panel_size>  Panel size
    
    flipdot_clear  [--invert]
      Clear the flipdot
          --invert  Set all pixels to white instead of black
    
    show_fonts
      List installed fonts
    
    render_font  [-x <int>] [-y <int>] [-f <font>] <text>
      Render some text given some font
      -x, --X=<int>  Depending on aligned, either left, center or right edge of target.
      -y, --Y=<int>  Upper edge of the target area.
      -f, --font=<font>  Name of font to use
            <text>  Text to display
    
Usage examples
--------------

- Connect to a wireless network

    .. code:: bash
    
       config_wifi_station NetworkName Password
       config_show
       config_save
       reboot

- Connect to an unencrypted wireless network

    .. code:: bash
    
       config_wifi_station NetworkName
       config_show
       config_save
       reboot

- MTU test

    .. code:: bash

        ping fritz.box -s 1472

- Cycle all pixels

    .. code:: bash

        flipdot_clear
        flipdot_clear --invert

Footnotes
---------

.. [#] See https://en.wikipedia.org/wiki/8-N-1
