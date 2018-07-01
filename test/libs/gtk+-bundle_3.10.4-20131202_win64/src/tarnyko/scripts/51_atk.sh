
cd ../../libs/51_atk
tar xfvJ atk-2.10.0.tar.xz
cd atk-2.10.0


echo Compile...

./configure --host=x86_64-w64-mingw32 --enable-static --enable-shared  --prefix="$PREFIX"
make clean
make 2>&1 | tee ../../../z_Install/win64_build/logs/51_atk-make.log
make install 2>&1 | tee ../../../z_Install/win64_build/logs/51_atk-makeinstall.log


cd ..
rm -rf atk-2.10.0
