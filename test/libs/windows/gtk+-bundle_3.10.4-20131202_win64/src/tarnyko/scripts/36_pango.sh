
cd ../../libs/36_pango
tar xfvJ pango-1.36.1.tar.xz
cd pango-1.36.1


echo Compile...

./configure --host=x86_64-w64-mingw32 --with-included-modules=yes --with-dynamic-modules=yes --enable-static --enable-shared --prefix="$PREFIX"
make clean
make 2>&1 | tee ../../../z_Install/win64_build/logs/36_pango-make.log
make install 2>&1 | tee ../../../z_Install/win64_build/logs/36_pango-makeinstall.log

echo Copy the .def files where they belong

cp pango/pango.def "$PREFIX/lib/pango-1.0.def"
cp pango/pangocairo.def "$PREFIX/lib/pangocairo-1.0.def"
cp pango/pangowin32.def "$PREFIX/lib/pangowin32-1.0.def"


cd ..
rm -rf pango-1.36.1
