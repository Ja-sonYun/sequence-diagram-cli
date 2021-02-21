#!/bin/sh

if [ -d "/usr/local/bin" ]
then
    echo "\x1B[32m** Downloading file from github...\033[0m"
else
    echo "\x1B[31mERROR: couldn't find path!\033[0m"
    exit 1
fi

D=`uname -a | awk '{print $1}'`
if [ "$D" = "Darwin" ]; then
    wget https://github.com/Ja-sonYun/sequence-diagram-cli/releases/download/v1.3.3/Darwin_seqdia -P ~/.seqdia
    mv ~/.seqdia/Darwin_seqdia ~/.seqdia/seqdia
    CHECKSUMD="070ced401cd599f43d7cfcabc78ed8ab"
elif [ "$D" = "Linux" ]; then
    echo "Clone repository..."
    git clone https://github.com/Ja-sonYun/sequence-diagram-cli.git ~/.seqdia
    echo "installing dependencies..."
    apt-get install libcurl4-gnutls-dev -y
    echo "running make"
    make -C ~/.seqdia
    echo "move binary to /usr/local/bin"
    sudo mv ~/.seqdia/seqdia /usr/local/bin
    echo "remove temp folder"
    rm -rf ~/.seqdia
    echo "Installed!"
    exit 1
else
    echo "This architecture does not supported!"
    echo "Download source file from my github, and please compile yourself."
    exit 1
fi

CHECKSUM=`md5 ~/.seqdia/seqdia | awk '{ print $4 }'`

if [ "$CHECKSUM" != "" ]; then
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

CHECKSUM=`md5 /usr/local/bin/seqdia | awk '{ print $4 }'`
if [ -x "/usr/local/bin/seqdia"  ] || [ "$CHECKSUM" = "$CHECKSUMD" ];
then
    echo "\x1B[32m** Installed!\033[0m"
else
    echo "\x1B[33mPermission Error! Please run \`sudo chmod 755 /usr/local/bin/seqdia\`"
    exit 1
fi

