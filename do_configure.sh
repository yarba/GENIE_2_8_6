#!/usr/bin/env bash

./configure \
  --enable-validation-tools \
  --enable-debug \
  --enable-test \
  --enable-numi \
  --enable-rwght \
  --with-optimiz-level=O0 \
  --with-log4cpp-inc=$LOG4CPP_INC \
  --with-log4cpp-lib=$LOG4CPP_LIB \
  >& log.config

  # --enable-doxygen-doc \
  # --enable-vle-extension \
  # --with-doxygen-path=/usr/bin/doxygen \
  # --with-libxml2-inc=/usr/include/libxml2 \
  # --with-libxml2-lib=/usr/lib64 \
  # --enable-gsl \
