Python Bindings for FakeVim
===========================

Few notes:
 * This is **work-in-progress** and still **incomplete**.
 * Bindings build process is handled using
   PyQt's [SIP](http://pyqt.sourceforge.net/Docs/sip4/index.html).
 * Debug build is enabled by default
   (for release verison: delete line containing `debug=1` in `configure.py`).
 * Compile FakeVim library and binndings with same version of Qt.
 * Use same major version of python to build and run.

To build run bash script `build.sh` or just create `build` sub-directory in
this directory and from there run `configure.py` and `make`.

To install run `make install` from `build` sub-directory.

