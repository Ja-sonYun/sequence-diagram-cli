#!/bin/sh

if [ -d "/usr/local/bin" ]
then
    echo "** Downloading source file to ~/.sequence-diagram-cli from github..."
else
    echo "ERROR: couldn't find path!"
    exit 1
fi

git clone https://github.com/Ja-sonYun/sequence-diagram-cli.git ~/.sequence-diagram-cli

echo "** Running make..."
make -C ~/.sequence-diagram-cli

echo "** Moving binary file to '/usr/local/bin'."
mv ~/.sequence-diagram-cli/seqdia /usr/local/bin

echo "** Deleting repository..."
rm -rf ~/.sequence-diagram-cli

echo "** Installed!"
echo "** You can test your installation with command \$( seqdia diagram.txt ) in this directory."
