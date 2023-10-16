FROM ubuntu:20.04
RUN apt-get update
RUN apt-get upgrade -y

ARG DEBIAN_FRONTEND=noninteractive TZ=Europe/Rome
RUN apt-get install build-essential libsqlite3-dev libboost-all-dev libssl-dev git python3-setuptools castxml -y
ARG DEBIAN_FRONTEND=noninteractive TZ=Europe/Rome
RUN apt-get install gir1.2-goocanvas-2.0 gir1.2-gtk-3.0 libgirepository1.0-dev python3-dev python3-gi python3-gi-cairo python3-pip python3-pygraphviz python3-pygccxml -y
ARG DEBIAN_FRONTEND=noninteractive TZ=Europe/Rome
RUN apt-get install wget g++ pkg-config sqlite3 qt5-default mercurial ipython3 openmpi-bin openmpi-common openmpi-doc libopenmpi-dev autoconf cvs bzr unrar gdb valgrind uncrustify doxygen graphviz imagemagick python3-sphinx dia tcpdump libxml2 libxml2-dev cmake libc6-dev libclang-6.0-dev llvm-6.0-dev automake -y

WORKDIR /home

RUN wget -c https://www.nsnam.org/releases/ns-allinone-3.38.tar.bz2

RUN tar -xvjf ns-allinone-3.38.tar.bz2

WORKDIR /home/ns-allinone-3.38/ns-3.38

RUN ./ns3 configure --disable-werror --enable-examples --enable-tests

RUN ./ns3

RUN ./test.py

# start a new terminal on entry
CMD /bin/sh 