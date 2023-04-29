Compiling
=========

A multistage **Docker** environment is used to build firmware and documentation.

1. Clone the repository *recursively*.

   .. code:: bash

        git clone --recursive https://gitlab.com/fluepke/fluepdot.git

2. Build or pull the build container.

    * If you have a high bandwidth internet connection downloading is fastest (~ 10 GB):

        .. code:: bash
    
            docker pull fluepke/fluepdot-build-environment

    * If you have a low bandwidth internet connection building is faster (still ~ 3 GB)

        .. code:: bash
            
            docker build --force-rm -t fluepke/fluepdot-build-environment -f Dockerfile .

3. Build the second stage container

        .. code:: bash

            docker build --force-rm -t fluepdot -f Dockerfile.build .

4. Run the second stage container and flash the image (assuming your flipdot's serial device is at ``/dev/ttyUSB0``)

        .. code:: bash
           
            docker run -d --name fluepdot --device /dev/ttyUSB0:/fluepdot-device:rwm fluepdot
            docker exec -it -w "/fluepdot/software/firmware" -e ESPTOOL_PORT='/fluepdot-device' -e ESPTOOL_BAUD='480000' fluepdot make flash

5. NOTE: the esp user in the container might not have the rights to access ``/dev/ttyUSB0`` (you will see a permissions denied error when trying to flash). A quick and dirty way to temporarily circumvent this issue is the following

        .. code:: bash

            sudo chmod 666 /dev/ttyUSB0

