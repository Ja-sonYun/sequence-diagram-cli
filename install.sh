#!/bin/sh

if [ -d "/usr/local/bin" ]
then
    echo "\x1B[32m** Downloading binary file from github...\033[0m"
else
    echo "\x1B[31mERROR: couldn't find path!\033[0m"
    exit 1
fi

wget https://github.com/Ja-sonYun/sequence-diagram-cli/releases/download/v1.2.1/seqdia -P ~/.seqdia
CHECKSUM=""

if command -v md5sum file &> /dev/null
then
    CHECKSUM=($(md5sum ~/.seqdia/seqdia))
elif command -v md5 file &> /dev/null
then
    CHECKSUM=($(md5 ~/.seqdia/seqdia))
fi

if [ "$CHECKSUM" != "" ]; then
    CHECKSUMD="71ebfc64b1885086aa939dbee0ee8270"
    echo "Compare Checksum $CHECKSUMD"
    if [ "$CHECKSUM" = "$CHECKSUMD" ]; then
        echo "PASSED"
    else
        echo "FAILED. Please download manually."
        echo "\x1B[32m** Removing temp folder ~/.seqdia\033[0m"
        rm -rf ~/.seqdia
        exit 1
    fi


    if [ -f "/usr/local/bin/seqdia" ]; then
        CHECKSUMO=($(md5sum /usr/local/bin/seqdia))
        if [ "$CHECKSUMD" = "$CHECKSUMO" ]; then
            echo "Nothing changed."
            echo "\x1B[32m** Removing temp folder ~/.seqdia\033[0m"
            rm -rf ~/.seqdia
            exit 1
        else
            echo "Updating..."
        fi
    fi
fi

echo "\x1B[32m** Moving binary file to '/usr/local/bin'.\033[0m"
sudo mv ~/.seqdia/seqdia /usr/local/bin/seqdia

echo "\x1B[32m** chmod 755 /usr/local/bin/seqdia...\033[0m"
sudo chmod 755 /usr/local/bin/seqdia

echo "\x1B[32m** Removing temp folder ~/.seqdia\033[0m"
rm -rf ~/.seqdia

if [ -f "/usr/local/bin/seqdia" ]
then
    :
else
    echo "\x1B[33mWARNING: something is wrong! Please move binary file to your PATH manually."
    echo "\x1B[33mWARNING: you can also compile this package from github source."
    echo "\x1B[33mWARNING: https://github.com/Ja-sonYun/sequence-diagram-cli\033[0m"
    exit 1
fi

if [ -x "/usr/local/bin/seqdia" ]
then
    echo "\x1B[32m** Installed!\033[0m"
else
    echo "\x1B[33mPermission Error! Please run \`sudo chmod 755 /usr/local/bin/seqdia\`"
    exit 1
fi

