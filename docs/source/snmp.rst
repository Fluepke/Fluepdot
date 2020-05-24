SNMP
====

To serious business people and for optimal integration into your existing enterprise network, the *fluepboard* firmware offers SNMP integration.

The SNMP implementation provides **monitoring**, full control over **framebuffer** and rendering options as well as an **IF-MIB**, **IP-MIB** and **TCP/UDP-MIB** implementation.

You might find the SNMP implementation useful in combination with the prometheus `snmp_exporter`_.

You can find the **MIB** (Mangement information base) file in ``util/FLUEPDOT.mib``.

Communities
-----------

Private (write-only) community
    private

Public (read-only) community
    public

Version
    v2c

Tree
----

.. code:: bash

   snmptranslate -m ./util/FLUEPDOT.mib -Tp .1.3.6.1.4.1.54722

.. code:: bash

   +--fluepke(54722)
   |
   +--projects(1)
      |
      +--fluepdot(1)
         |
         +--framebuffer(1)
         |  |
         |  +-- -R-- Integer32 width(1)
         |  +-- -R-- Integer32 height(2)
         |  |
         |  +--pixelsTable(3)
         |     |
         |     +--pixelEntry(1)
         |        |  Index: pixelX, pixelY
         |        |
         |        +-- -R-- Integer32 pixelX(1)
         |        |        Range: 0..255
         |        +-- -R-- Integer32 pixelY(2)
         |        |        Range: 0..255
         |        +-- -RW- EnumVal   pixelState(3)
         |                 Values: dark(0), bright(1)
         |
         +--panels(2)
         |  |
         |  +-- -R-- Integer32 panelCount(1)
         |  |        Range: 0..5
         |  |
         |  +--panelTable(2)
         |     |
         |     +--panelTableEntry(1)
         |        |  Index: panelIndex
         |        |
         |        +-- -R-- Integer32 panelIndex(1)
         |        |        Range: 0..5
         |        +-- -R-- Integer32 panelWidth(2)
         |        |        Range: 20..25
         |        +-- -R-- Integer32 panelX(3)
         |                 Range: 0..255
         |
         +--renderingOptions(3)
         |  |
         |  +--delayTable(1)
         |  |  |
         |  |  +--delayEntry(1)
         |  |     |  Index: column
         |  |     |
         |  |     +-- -R-- Integer32 column(1)
         |  |     |        Range: 0..255
         |  |     +-- -RW- Integer32 columnPreDelay(2)
         |  |     +-- -RW- Integer32 columnSetDelay(3)
         |  |     +-- -RW- Integer32 columnClearDelay(4)
         |  |
         |  +--panelOrderTable(2)
         |  |  |
         |  |  +--panelOrderEntry(1)
         |  |     |  Index: orderIndex
         |  |     |
         |  |     +-- -R-- Integer32 orderIndex(1)
         |  |     |        Range: 0..5
         |  |     +-- -RW- Integer32 panelOrderIndex(2)
         |  |              Range: 0..5
         |  |
         |  +-- -RW- EnumVal   renderingMode(3)
         |           Values: full(0), differential(1)
         |
         +-- -R-- Counter64 pixelsFlipped(4)
         +-- --W- Integer32 dirtyBit(69)

Usage examples
--------------

- Set the pixel *x=23, y=4* to bright

    .. code:: bash

        snmpset -v 2c -c private -m ./util/FLUEPDOT.mib fluepdot0.cluster.ap-south-1.yolo.network FLUEPDOT-MIB::pixelState.23.4 i bright

- Set the dirty bit (aka tell the *fluepboard* to render framebuffer contents)

    .. code:: bash

        snmpset -v 2c -c private -m ./util/FLUEPDOT.mib fluepdot0.cluster.ap-south-1.yolo.network FLUEPDOT-MIB::dirtyBit.0 i 1

- Get the number of flipped pixels

    .. code:: bash

        snmpget -v 2c -c public -m ../util/FLUEPDOT.mib 192.168.178.94 FLUEPDOT-MIB::pixelsFlipped.0

.. _snmp_exporter: https://github.com/prometheus/snmp_exporter
