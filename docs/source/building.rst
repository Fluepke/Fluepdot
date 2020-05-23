Compiling
=========

A multistage **Docker** environment is used to build firmware and documentation.

1. Clone the repository recursively.
   .. code:: bash

        git clone --recursive https://gitlab.com/fluepke/fluepdot.git

2. Build or pull the build container.

    1. If you have a high bandwidth internet connection downloading is fastest (~10 GB):
        .. code:: bash

            docker pull fluepke/fluepdot-esp-idf

    2. If you have a low bandwidth internet connection building is faster (~ 3 GB Download required)

        .. code:: bash
            
            docker build -t fluepdot-esp-idf .

3. Build the second stage container

        .. code:: bash

            docker build -t fluepdot -f Dockerfile.build .
