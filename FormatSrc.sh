#! /bin/sh
./call_Artistic_Style.sh . 'hpp'
./call_Artistic_Style.sh . 'cpp'
find -name "*.orig" -delete

