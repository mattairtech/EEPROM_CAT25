/*
 * Driver for On Semiconductor CAT25 SPI EEPROM chips for
 * AVR, SAM3X (Due), and SAM M0+ (SAMD, SAML, SAMC) microcontrollers
 * 
 * Copyright (c) 2017, Justin Mattair (justin@mattair.net)
 * 
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

/* This example uses writeBlock() and readBlock(). Any start
 * address within the device range can be used. The maximum length
 * is the device capacity minus the start address (no wrapping).
 * All writes to the EEPROM make use of an internal page buffer.
 * Only byte addresses that are written to within the page will be
 * modified. The pageWrite() method handles the actual writing,
 * with a maximum length being the page size minus the relative
 * start address (no page wrapping). The writeBlock() method
 * handles splitting the write into multiple calls to pageWrite().
 * Reading does not require page mode access.
 */

#include <SPI.h>
#include <EEPROM_CAT25.h>

// SPI bus can be SPI, SPI1 (if present), etc.
#define SPI_PERIPHERAL		SPI
#define CHIP_SELECT_PIN		27

/* Device can be:
 * CAT25M01 (128KB)
 * CAT25512 (64KB)
 * CAT25256 (32KB)
 * CAT25128 (16KB)
 * CAT25640 (8KB)
 * CAT25320 (4KB)
 * CAT25160 (2KB)
 * CAV25160 (2KB, automotive)
 * CAT25080 (1KB)
 * CAV25080 (1KB, automotive)
 * CAT25040 (512B)
 * CAT25020 (256B)
 * CAT25010 (128B)
 */
EEPROM_CAT25 EEPROM(&SPI_PERIPHERAL, CHIP_SELECT_PIN, CAT25512);

// Additional EEPROM chips
// EEPROM_CAT25 EEPROM1(&SPI_PERIPHERAL1, CHIP_SELECT_PIN1, CAT25256);

#define START_ADDRESS   250

//uint8_t buffer[BUFFER_SIZE];
//#define BUFFER_SIZE  320

char buffer[] = "The MattairTech MT-D21E is a development board for the 32-pin Microchip / Atmel SAMx21E ARM Cortex M0+ microcontrollers. Choose between the D21E, L21E, or C21E. Arduino compatible core files for all 3 chips is provided.";
#define BUFFER_SIZE  (sizeof(buffer) / sizeof(uint8_t))

// If using writePage(), you can set the buffer size to the page size by using the header file defines.
//uint8_t pageBuffer[EEPROM_PAGE_SIZE_CAT25512];

void setup(void)
{
  /* Without parameters, begin() uses the default speed for this
   * library (4MHz). Note that SPI transaction support is required.
   */
  EEPROM.begin();
  //EEPROM.begin(8000000UL);	// or specify speed

  Serial.begin(9600);
}

void loop(void)
{
  while (!Serial); // Wait for serial monitor to connect

  // Initial Read Byte for previously written contents, print to serial monitor
  Serial.print("Initial Read Byte:  ");
  for (size_t i=0; i < BUFFER_SIZE; i++) {
    Serial.write(EEPROM.readByte(START_ADDRESS + i));
  }
  Serial.println();

  // Write Byte, print to serial monitor
  Serial.print("Write Byte:  ");
  for (size_t i=0; i < BUFFER_SIZE; i++) {
    Serial.write(buffer[i]);
    EEPROM.writeByte(START_ADDRESS + i, buffer[i]);
  }
  Serial.println();

  // Clear buffer
  memset(&buffer[0], 0, BUFFER_SIZE);

  // Read block
  Serial.print("Read Block:  ");
  if (!EEPROM.readBlock(START_ADDRESS, BUFFER_SIZE, buffer)) {
    Serial.println("Read Block Failure");
  }

  // Print buffer to serial monitor
  for (size_t i=0; i < BUFFER_SIZE; i++) {
    Serial.write(buffer[i]);
  }
  Serial.println();

  // Print buffer to serial monitor
  Serial.print("Write Block: ");
  for (size_t i=0; i < BUFFER_SIZE; i++) {
    Serial.write(buffer[i]);
  }
  Serial.println();

  // Write block
  if (!EEPROM.writeBlock(START_ADDRESS, BUFFER_SIZE, buffer)) {
    Serial.println("Write Block Failure");
  }

  // Clear buffer
  memset(&buffer[0], 0, BUFFER_SIZE);

  // Read Byte, print to serial monitor
  Serial.print("Read Byte:  ");
  for (size_t i=0; i < BUFFER_SIZE; i++) {
    buffer[i] = EEPROM.readByte(START_ADDRESS + i);
    Serial.write(buffer[i]);
  }
  Serial.println();

  while(1);
}
