# install x86_64-w64-mingw32-gcc
1. 
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-mingw-w64/gcc-mingw-w64-x86-64_7.3.0-11ubuntu1+20.2build1_amd64.deb
then sudo dpkg -i ...


# How to build it ?

1. 
./init.sh  ==> init build env

2. 
./mk w  ==> Windows executable

3. 
./mk l  ==> Linux executable
