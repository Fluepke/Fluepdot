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

.. runcmd:: snmptranslate -m ../util/FLUEPDOT.mib -Tp .1.3.6.1.4.1.54722
   :syntax: bash

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
