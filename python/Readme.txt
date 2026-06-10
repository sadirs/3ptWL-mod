wlcfpy python module

To use this python wrapper for wlcf, you should wlcf with 'make all': this is important in order to create libraries and python module. Be sure that you did not remove the -fPIC compiling flag in the Makefile, important for compatibility between OpenMP and the python wrapper.

Then, execute:

$ make all

You can check that these steps work by typing

$ python
>>> from wlcfpy import wlcf

If python does not complain, the wlcfpy module has been correctly installed in your python distribution. You can now import it and use its functions from your python codes.  

Notes:
1. GSL is working so far. Go Makefile_settings (USEGSL) and turn it off.
