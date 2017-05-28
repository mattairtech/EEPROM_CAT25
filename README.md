# EEPROM_CAT25

Driver for On Semiconductor CAT25 SPI EEPROM chips for
AVR, SAM3X (Due), and SAM M0+ (SAMD, SAML, SAMC) microcontrollers

# Supported Chips

* CAT25M01 (128KB, 256B page, ECC, ID Page)
* CAT25512 (64KB, 128B page, ECC, ID Page)
* CAT25256 (32KB, 64B page, ECC (Rev E), ID Page (Rev E))
* CAT25128 (16KB, 64B page, ECC (Rev E), ID Page (Rev E))
* CAT25640 (8KB, 64B page)
* CAT25320 (4KB, 32B page)
* CAT25160 (2KB, 32B page)
* CAV25160 (2KB, automotive, 32B page)
* CAT25080 (1KB, 32B page)
* CAV25080 (1KB, automotive, 32B page)
* CAT25040 (512B, 16B page)
* CAT25020 (256B, 16B page)
* CAT25010 (128B, 16B page)

## Page Write



## ECC



# Methods

Without parameters, begin() uses the default speed for this
library (4MHz). Note that SPI transaction support is required.

# Possible Future Additions/Changes

* Optimizations
* Block Protection / WP pin support
* Identification Page support
* HOLD support?

# License

Copyright (c) 2017, Justin Mattair (justin@mattair.net)

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of the author not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

The author disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall the author be liable for any
special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
