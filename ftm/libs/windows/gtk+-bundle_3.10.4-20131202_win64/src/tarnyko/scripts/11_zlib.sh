
cd ../../libs/11_zlib
unzip -o zlib128.zip
cd zlib-1.2.8


echo Patching the Makefile with the correct options

cd ..
patch -p0 < zlib128-Makefile-amd64.patch
cd zlib-1.2.8

echo Compile...

make -fwin32/Makefile.gcc 2>&1 | tee ../../../z_Install/win64_build/logs/11_zlib-make.log

echo Install final files -including custom .pc- to their destination

cp zlib1.dll "$PREFIX/bin"
cp zconf.h "$PREFIX/include"
cp zlib.h "$PREFIX/include"
cp libz.a "$PREFIX/lib"
cp libz.dll.a "$PREFIX/lib"
cd ..
cp zlib.pc "$PREFIX/lib/pkgconfig/"



rm -rf zlib-1.2.8
