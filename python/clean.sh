#/bin/sh
[ -f "fakevim.sip" -a -f "configure.py" ] || exit 1
rm -f *.cpp *.h *.o *.sbf Makefile fakevimconfig.py

