#!/bin/bash

if [ "$(id -u)" != "0" ]; then
        echo "Sorry, you are not root."
        exit 1
fi

DST=""

if [ $# -eq 1 ];then

        if [ -d $1 ];then
                DST=$1
        fi
fi

#service ddbusd stop || echo
        
mkdir -p build/usr/lib build/usr/bin build/usr/include build/etc/init.d/

cp my-ip-addr build/usr/bin
cp src/ddbus.h build/usr/include
cp ddbusd.service build/etc/init.d/ddbusd
cp -rf build/* $DST

#service ddbusd start
#update-rc.d ddbusd defaults
