FROM archlinux:20200505
RUN pacman -Syu --noconfirm --needed gcc git make ncurses flex bison gperf python-pyserial python-click python-cryptography python-future python-pyparsing python-pyelftools cmake ninja ccache dfu-util \ 
    gawk gperf grep gettext ncurses python2 python2-pip python-pip automake bison flex texinfo help2man libtool make autoconf unzip which patch doxygen cxxtest freetype2 pkgconf
RUN mkdir /esp
RUN git clone -b fluepke/snmp --recursive https://github.com/Fluepke/esp-idf.git /esp/esp-idf
ENV IDF_PATH="/esp/esp-idf"
RUN pip2.7 install -r /esp/esp-idf/requirements.txt && pip install -r /esp/esp-idf/requirements.txt
RUN groupadd -g 1337 esp && useradd -r -u 1337 -g esp esp
RUN chown -R esp:esp /esp
COPY ./docs/requirements.txt /tmp/requirements.txt
RUN pip install -r /tmp/requirements.txt
USER esp
RUN git clone https://github.com/espressif/crosstool-NG.git /esp/crosstool-NG && git -C /esp/crosstool-NG checkout esp-2020r1 && git -C /esp/crosstool-NG submodule update --init
RUN sed -i 's/--enable-newlib-long-time_t //g' /esp/crosstool-NG/samples/xtensa-esp32-elf/crosstool.config
WORKDIR /esp/crosstool-NG
RUN ./bootstrap && ./configure --enable-local && make -j
RUN ./ct-ng xtensa-esp32-elf && ./ct-ng build -j
RUN chmod -R u+w /esp/crosstool-NG/builds/xtensa-esp32-elf
ENV PATH="/esp/crosstool-NG/builds/xtensa-esp32-elf/bin/:${PATH}"
