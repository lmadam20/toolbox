dolheader
==========
Prints out the header of a executable dol file in
a human readable format.

Usage
------------------------------------------------
**Usage:** `./dolheader [-a] <dol file>`  
**Option(s):**
* `-a` to display _all_ sections (even empty ones)

Example usage:
------------------------------------------------
`./dolheader -a boot.dol`

Compiling
-----------------------------------------------
With GCC: `gcc -o dolheader dolheader.c`

(If your compiler outputs "Endianness of 
your Machine is not supported!", try to
specify the endianness by defining
`ENDIANNESS_LITTLE` or `ENDIANNESS_BIG`)

License
-----------------------------------------------
See the file 'LICENSE'
