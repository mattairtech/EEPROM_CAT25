# EEPROM_CAT25

Arduino library for standard SPI EEPROM chips.


## Supported Chips

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
* ST M95xxx range

Other compatible chips can also be supported, see below.


## Addressing

The 1/2/4kbit chips use 8 address bits, while the larger EEPROMs use
16 bits (or 24 bits for the 1MBit chips and above). The 4kbit chips however, need
9 bits, so bit position 3 of the READ or WRITE instrutions is used as the 9th
bit of the address. This is handled automatically by the library. There are no
address alignment considerations for either byte or block read/write methods.


## Startup

Call the constructor with the SPI bus, chip select, and device. Devices are defined
in EEPROM_CAT25.h and have names like CAT25512. More than one device can be instantiated.

```
EEPROM_CAT25 EEPROM(&SPI, 22, CAT25512);
EEPROM_CAT25 EEPROM1(&SPI, 23, CAT25M01);
```

Call begin(). Without parameters, begin() uses the default speed for this
library (4MHz). Note that SPI transaction support is required.

```
EEPROM.begin();
EEPROM1.begin(8000000UL);      // or specify SPI clock speed in Hz
```

Call end() to set the chip select pin back to INPUT.

```
EEPROM.end();
```

## Supporting extra devices

A lot of different devices all support the same protocol. These devices are often
referred to as "standard SPI EEPROM" or "standard serial EEPROM". Any compatible device
can be used with this library, you just have to specify the capacity and pagesize. You
can define a new device type in the same way as the library does it. For example, if the
CAT25010 would not be supported yet, you could do:

    const EEPROM_CAT25_Device CAT25010 = { .capacity = 0x80, .pageSize = 16};

And then use that variable as normal:

    EEPROM_CAT25 EEPROM(&SPI, 22, CAT25010);

## Ready Flag

All reading and writing methods will first check that the EEPROM is ready (not busy with
a previous write) by calling the isReady() method, which returns a bool. If busy, yield()
will be called repeatedly until the EEPROM is ready, which may take up to 5ms (for
CAT25xxx devices) if called immediately after a full page write. This method can be
called directly:

```
while (!isReady()) {
  yield();
}
```

All these methods implement a timeout while waiting for the ready flag: if the chip is
not ready within 6ms (5ms write time plus some margin), the operation is aborted.

Additionally, getStatusRegister() can be called to get the status register contents directly.
See EEPROM_CAT25.h for register definitions.

```
uint8_t ret = getStatusRegister();
```


## Write Enable

Before writing, the write latch must be enabled. This is handled automatically by the write
methods, however, it can also be called directly:

```
enableWrite();
```

Additionally, disableWrite() can be called:

```
disableWrite();
```


## Byte Transfers

Call readByte with the address to read one byte. Returns 0 if address >= capacity, or
when a timeout occurred. Because valid data could also be 0, use readBlock() to catch
this error.

```
uint32_t address = 0x000A;
uint8_t byte = EEPROM.readByte(address);
```

Call writeByte with the address and the byte to write. Returns 0 if address >= capacity,
otherwise, returns 1.

```
uint32_t address = 0x000A;
uint8_t byte = 0x1F;
size_t ret = EEPROM.writeByte(address, byte);
```


## Block Transfers

Call readBlock() with the address, length to read, and a pointer to the buffer used to
store the data read. Any length can be used, limited by the size of the buffer. If the length
exceeds the last address, it will NOT wrap to address 0. Note that this method knows nothing about
the size of the buffer used, so ensure length is not set too high or a buffer overflow will
occur. Returns 0 if (length + address) > capacity, if length = 0 or when a timeout occurred.

```
#define START_ADDRESS   0x001A
#define BUFFER_SIZE     64
uint8_t buffer[BUFFER_SIZE];
size_t ret = EEPROM.readBlock(START_ADDRESS, BUFFER_SIZE, buffer);        // entire buffer
size_t ret1 = EEPROM1.readBlock(START_ADDRESS, 4, &buffer[16]);           // partial buffer
```

Call writeBlock() with the address, length to write, and a pointer to the source buffer used
for the data to be written. Any length can be used, limited by the size of the buffer. If the
length exceeds the last address, it will NOT wrap to address 0. Any address can be used within
a page. Bytes within a page that are not written will be left unchanged. Note that this method
knows nothing about the size of the buffer used, so ensure length is not set too high or data
will be read past the end of the buffer. Returns 0 if (length + address) > capacity, if
length = 0 or when a timeout occurred. This method calls writePage() as many times as
needed to transfer the entire block, but there are no address alignment considerations as
writeBlock() handles this automatically.

```
size_t ret = EEPROM.writeBlock(START_ADDRESS, BUFFER_SIZE, buffer);
```


## Page Write

While writeBlock() is normally used for block transfers, which in turn calls writePage() as
many times as needed to transfer the entire block, writePage() can be called directly.
Call writePage() with the address, length to write, and a pointer to the source buffer used
for the data to be written. Any length can be used up to the size of the buffer or the size
of the page (see EEPROM_CAT25.h for page sizes). If the length exceeds the last address of
the page, it will NOT wrap to the beginning of the page. Any address can be used within a
page. Bytes within a page that are not written will be left unchanged. Note that this method
knows nothing about the size of the buffer used, so ensure length is not set too high or data
will be read past the end of the buffer. Returns 0 if (length + address) > capacity,
if length = 0, if writing past the last page address (on page size boundaries) or when a
timeout occurred.

```
size_t ret = EEPROM.writePage(0x001A, 10, buffer);
```

## Only writing modified data
The `writeByte`, `writeBlock` and `writePage` methods write the data to
the EEPROM unconditionally. The `updateByte`, `updateBlock` and
`updatePage` methods are equivalent, except that they first check
whether the data is actually different before writing.

This check is currently implemented per EEPROM page. For each page any
unchanged bytes at the *start* of the page are skipped. Any unchanged
bytes after the first changed bytes are written as normal. This approach
ensures that pages that no additional write cycles are needed, at most
one extra SPI read transaction per page, while still guaranteeing that
any completely unchanged pages are skipped entirely.

## ECC

Higher capacity devices uses ECC (Error Correction Code) logic with 6 ECC bits to correct
one bit error in 4 data bytes. Therefore, when a single byte has to be written, 4 bytes
(including the ECC bits) are re−programmed. It is recommended to write in multiples of 4
bytes in order to maximize the number of write cycles (if needed).

## Block protection

All devices support a feature called "Block Protection", which
allows write-protecting (part of) the EEPROM using a few bits in the
status register. The protection can be disabled, apply to the full
EEPROM, or be applied to the upper quarter or half of the EEPROM:

    bool ret EEPROM.setBlockProtect(EEPROM_CAT25_BP_NONE);
    bool ret EEPROM.setBlockProtect(EEPROM_CAT25_BP_QUARTER);
    bool ret EEPROM.setBlockProtect(EEPROM_CAT25_BP_HALF);
    bool ret EEPROM.setBlockProtect(EEPROM_CAT25_BP_FULL);

This function returns false if an error occurs (invalid argument or
timeout waiting for the chip to be ready) or true otherwise.

## Write protection

In addition to block protection, most devices (except for the smallest
ones) support additional write protection of the status register. This
allows the block protection to become irreversible (or only reversible
by flipping a pin).

To enable the write protection bit in the status register, call:

    EEPROM.setWriteProtectEnable(true);

Note that the `WP` pin must additionally pulled low for the write
protection to become effective (though the bit can be set before pulling
`WP` low).

To disable the write protection bit, call:

    EEPROM.setWriteProtectEnable(false);

Note that this is only possible when the write protection is not
currently active (*i.e.* when `WP` is high).

## Possible Future Additions/Changes

* Optimizations
* Identification Page support
* HOLD support?


## Changelog

v1.0.2 - Fixed demo sketch (test string was not printing).
v1.0.1 - Fixed compilation error on avr architecture.
v1.0.0 - Initial release.


## License

Copyright (c) 2017-2018, Justin Mattair (justin@mattair.net)

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
