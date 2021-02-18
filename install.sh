#!/bin/sh
make
if [ -d "/usr/local/bin" ]
then
    echo "moving binary file to '/usr/local/bin'."
else
    echo "ERROR: couldn't find path!"
    exit 1
fi
sudo mv ./seqdia /usr/local/bin
make clean
echo "Installed!"
echo "You can test your installation with command \$( seqdia diagram.txt ) in this directory."
