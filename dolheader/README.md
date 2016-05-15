dolheader
==========
Copyright (c) 2016 Leon Adam
(see file 'LICENSE' for licensing information)

Prints out the header of a executable dol file in
a human readable format.

Usage
------------------------------------------------
**Usage:** _./<executable_name> [-a] <dol file>_

**Example usage:**
_./dolheader -a boot.dol_

Compiling
-----------------------------------------------
With GCC: _gcc -o dolheader dolheader.c_

(If your compiler outputs "Endianness of 
your Machine is not supported!", try to
specify the endianness by define
ENDIANNESS_LITTLE or ENDIANNESS_BIG)
