#!/bin/sh

if [ -d "/usr/local/bin" ]
then
    echo "\x1B[32m** Downloading binary file from github...\033[0m"
else
    echo "\x1B[31mERROR: couldn't find path!\033[0m"
    exit 1
fi

wget https://github.com/Ja-sonYun/sequence-diagram-cli/blob/main/seqdia\?raw\=true -P ~/.seqdia

echo "\x1B[32m** chmod 755 /usr/local/bin/seqdia...\033[0m"
`sudo chmod 755 /usr/local/bin/seqdia`

echo "\x1B[32m** Moving binary file to '/usr/local/bin'.\033[0m"
mv ~/.seqdia/seqdia?raw=true /usr/local/bin/seqdia

echo "\x1B[32m** Removing temp folder ~/.seqdia\033[0m"
rm -rf ~/.seqdia

if [ -f "/usr/local/bin/seqdia" ]
then
    echo "\x1B[32m** Installed!\033[0m"
else
    echo "\x1B[33mWARNING: something is wrong! Please move binary file to your PATH manually."
    echo "\x1B[33mWARNING: you can also compile this package from github source."
    echo "\x1B[33mWARNING: https://github.com/Ja-sonYun/sequence-diagram-cli\033[0m"
    exit 1
fi

