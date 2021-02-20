#!/bin/sh

if [ `whoami` != root ]; then
    echo Please run this script as root or using sudo
    exit
fi

if [ -d "/usr/local/bin" ]
then
    echo "** Downloading binary file from github..."
else
    echo "ERROR: couldn't find path!"
    exit 1
fi

wget https://github.com/Ja-sonYun/sequence-diagram-cli/blob/main/seqdia\?raw\=true -P ~/.seqdia

echo "** Moving binary file to '/usr/local/bin'."
mv ~/.seqdia/seqdia?raw=true /usr/local/bin/seqdia

echo "** chmod 755 /usr/local/bin/seqdia..."

chmod 755 /usr/local/bin/seqdia

echo "** Installed!"
