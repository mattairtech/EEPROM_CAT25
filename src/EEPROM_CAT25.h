/*
 * Driver for On Semiconductor CAT25 SPI EEPROM chips for
 * AVR, SAM3X (Due), and SAM M0+ (SAMD, SAML, SAMC) microcontrollers
 * 
 * Copyright (c) 2017-2018, Justin Mattair (justin@mattair.net)
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

#ifndef EEPROM_CAT25_h_
#define EEPROM_CAT25_h_

#include "Arduino.h"
#include <SPI.h>
#include <stdbool.h>

#if (!defined(SPI_HAS_TRANSACTION) || SPI_HAS_TRANSACTION == 0)
#error "EEPROM_CAT25 library requires SPI library transaction support (SPI_HAS_TRANSACTION = 1)"
#endif


typedef enum _EEPROM_CAT25_Device
{
  CAT25M01=0,
  CAT25512,
  CAT25256,
  CAT25128,
  CAT25640,
  CAT25320,
  CAT25160,
  CAV25160,
  CAT25080,
  CAV25080,
  CAT25040,
  CAT25020,
  CAT25010,
} EEPROM_CAT25_Device;

#define EEPROM_CAPACITY_CAT25M01	0x20000
#define EEPROM_CAPACITY_CAT25512	0x10000
#define EEPROM_CAPACITY_CAT25256	0x8000
#define EEPROM_CAPACITY_CAT25128	0x4000
#define EEPROM_CAPACITY_CAT25640	0x2000
#define EEPROM_CAPACITY_CAT25320	0x1000
#define EEPROM_CAPACITY_CAT25160	0x800
#define EEPROM_CAPACITY_CAV25160	0x800
#define EEPROM_CAPACITY_CAT25080	0x400
#define EEPROM_CAPACITY_CAV25080	0x400
#define EEPROM_CAPACITY_CAT25040	0x200
#define EEPROM_CAPACITY_CAT25020	0x100
#define EEPROM_CAPACITY_CAT25010	0x80

#define EEPROM_PAGE_SIZE_CAT25M01	256
#define EEPROM_PAGE_SIZE_CAT25512	128
#define EEPROM_PAGE_SIZE_CAT25256	64
#define EEPROM_PAGE_SIZE_CAT25128	64
#define EEPROM_PAGE_SIZE_CAT25640	64
#define EEPROM_PAGE_SIZE_CAT25320	32
#define EEPROM_PAGE_SIZE_CAT25160	32
#define EEPROM_PAGE_SIZE_CAV25160	32
#define EEPROM_PAGE_SIZE_CAT25080	32
#define EEPROM_PAGE_SIZE_CAV25080	32
#define EEPROM_PAGE_SIZE_CAT25040	16
#define EEPROM_PAGE_SIZE_CAT25020	16
#define EEPROM_PAGE_SIZE_CAT25010	16

// Maximum write time in milliseconds
#define EEPROM_CAT25_MAX_WRITE_TIME_MS		5
#define EEPROM_CAT25_TIMEOUT_TIME_MS		(EEPROM_CAT25_MAX_WRITE_TIME_MS + 1)

#define EEPROM_CAT25_DUMMY_BYTE			0xFF

// Commands supported by all chips
#define EEPROM_CAT25_COMMAND_READ		0x03
#define EEPROM_CAT25_COMMAND_WRITE		0x02
#define EEPROM_CAT25_COMMAND_RDSR		0x05
#define EEPROM_CAT25_COMMAND_WRSR		0x01
#define EEPROM_CAT25_COMMAND_WREN		0x06
#define EEPROM_CAT25_COMMAND_WRDI		0x04

/* The CAT25040/CAT25020/CAT25010 use 8 address bits, while the
 * larger EEPROMs use 16 bits (or 24 bits for the 1MBit chips and above).
 * The CAT25040 however, needs 9 bits, so bit position 3 of the
 * READ or WRITE instrutions is used as the 9th bit of the address.
 */
#define EEPROM_CAT25_COMMAND_READ_A8_HIGH	0x0B
#define EEPROM_CAT25_COMMAND_WRITE_A8_HIGH	0x0A


typedef union {
    struct {
        uint8_t  WPEN:1;	// Write Protect Enable. Not present on CAT25040/CAT25020/CAT25010
        uint8_t  IPL:1;		// Identification Page Latch. Present only on newer revisions of CAT25128/CAT25256 and all CAT25512/CAT25M01
	uint8_t  :1;		// reserved
	uint8_t  LIP:1;		// Lock Identification Page. Present only on newer revisions of CAT25128/CAT25256 and all CAT25512/CAT25M01
	uint8_t  BP:2;		// Block protection
	uint8_t  WEL:1;		// Write Enable Latch
	uint8_t  RDY:1;		// Ready (ready when low)
    } bit;
    uint32_t word;
} EEPROM_CAT25_Status_Register;

// WPEN - Write Protect Enable. Not present on CAT25040/CAT25020/CAT25010
#define EEPROM_CAT25_WPEN_Pos			7
#define EEPROM_CAT25_WPEN_Msk			(0x1ul << EEPROM_CAT25_WPEN_Pos)
#define EEPROM_CAT25_WPEN(value)		(EEPROM_CAT25_WPEN_Msk & ((value) << EEPROM_CAT25_WPEN_Pos))
#define   EEPROM_CAT25_WPEN_DISABLE_Val		0x0ul
#define   EEPROM_CAT25_WPEN_ENABLE_Val		0x1ul
#define EEPROM_CAT25_WPEN_DISABLE		(EEPROM_CAT25_WPEN_DISABLE_Val << EEPROM_CAT25_WPEN_Pos)
#define EEPROM_CAT25_WPEN_ENABLE		(EEPROM_CAT25_WPEN_ENABLE_Val << EEPROM_CAT25_WPEN_Pos)

// IPL - Identification Page Latch. Present only on newer revisions of CAT25128/CAT25256 and all CAT25512/CAT25M01
#define EEPROM_CAT25_IPL_Pos			6
#define EEPROM_CAT25_IPL_Msk			(0x1ul << EEPROM_CAT25_IPL_Pos)
#define EEPROM_CAT25_IPL(value)			(EEPROM_CAT25_IPL_Msk & ((value) << EEPROM_CAT25_IPL_Pos))
#define   EEPROM_CAT25_IPL_MAIN_Val		0x0ul
#define   EEPROM_CAT25_IPL_ID_PAGE_Val		0x1ul
#define EEPROM_CAT25_IPL_MAIN			(EEPROM_CAT25_IPL_MAIN_Val << EEPROM_CAT25_IPL_Pos)
#define EEPROM_CAT25_IPL_ID_PAGE		(EEPROM_CAT25_IPL_ID_PAGE_Val << EEPROM_CAT25_IPL_Pos)

// LIP - Lock Identification Page. Present only on newer revisions of CAT25128/CAT25256 and all CAT25512/CAT25M01
#define EEPROM_CAT25_LIP_Pos			4
#define EEPROM_CAT25_LIP_Msk			(0x1ul << EEPROM_CAT25_LIP_Pos)
#define EEPROM_CAT25_LIP(value)			(EEPROM_CAT25_LIP_Msk & ((value) << EEPROM_CAT25_LIP_Pos))
#define   EEPROM_CAT25_LIP_DISABLE_Val		0x0ul
#define   EEPROM_CAT25_LIP_ENABLE_Val		0x1ul
#define EEPROM_CAT25_LIP_DISABLE		(EEPROM_CAT25_LIP_DISABLE_Val << EEPROM_CAT25_LIP_Pos)
#define EEPROM_CAT25_LIP_ENABLE			(EEPROM_CAT25_LIP_ENABLE_Val << EEPROM_CAT25_LIP_Pos)

// BP - Block protection
#define EEPROM_CAT25_BP_Pos			2
#define EEPROM_CAT25_BP_Msk			(0x3ul << EEPROM_CAT25_BP_Pos)
#define EEPROM_CAT25_BP(value)			(EEPROM_CAT25_BP_Msk & ((value) << EEPROM_CAT25_BP_Pos))
#define   EEPROM_CAT25_BP_NONE_Val		0x0ul
#define   EEPROM_CAT25_BP_QUARTER_Val		0x1ul
#define   EEPROM_CAT25_BP_HALF_Val		0x2ul
#define   EEPROM_CAT25_BP_FULL_Val		0x3ul
#define EEPROM_CAT25_BP_NONE			(EEPROM_CAT25_BP_NONE_Val << EEPROM_CAT25_BP_Pos)
#define EEPROM_CAT25_BP_QUARTER			(EEPROM_CAT25_BP_QUARTER_Val << EEPROM_CAT25_BP_Pos)
#define EEPROM_CAT25_BP_HALF			(EEPROM_CAT25_BP_HALF_Val << EEPROM_CAT25_BP_Pos)
#define EEPROM_CAT25_BP_FULL			(EEPROM_CAT25_BP_FULL_Val << EEPROM_CAT25_BP_Pos)

// WEL - Write Enable Latch
#define EEPROM_CAT25_WEL_Pos			1
#define EEPROM_CAT25_WEL_Msk			(0x1ul << EEPROM_CAT25_WEL_Pos)
#define EEPROM_CAT25_WEL(value)			(EEPROM_CAT25_WEL_Msk & ((value) << EEPROM_CAT25_WEL_Pos))
#define   EEPROM_CAT25_WEL_DISABLE_Val		0x0ul
#define   EEPROM_CAT25_WEL_ENABLE_Val		0x1ul
#define EEPROM_CAT25_WEL_DISABLE		(EEPROM_CAT25_WEL_DISABLE_Val << EEPROM_CAT25_WEL_Pos)
#define EEPROM_CAT25_WEL_ENABLE			(EEPROM_CAT25_WEL_ENABLE_Val << EEPROM_CAT25_WEL_Pos)

// RDY - Ready (ready when low)
#define EEPROM_CAT25_RDY_Pos			0
#define EEPROM_CAT25_RDY_Msk			(0x1ul << EEPROM_CAT25_RDY_Pos)
#define EEPROM_CAT25_RDY(value)			(EEPROM_CAT25_RDY_Msk & ((value) << EEPROM_CAT25_RDY_Pos))
#define   EEPROM_CAT25_RDY_READY_Val		0x0ul
#define   EEPROM_CAT25_RDY_BUSY_Val		0x1ul
#define EEPROM_CAT25_RDY_READY			(EEPROM_CAT25_RDY_READY_Val << EEPROM_CAT25_RDY_Pos)
#define EEPROM_CAT25_RDY_BUSY			(EEPROM_CAT25_RDY_BUSY_Val << EEPROM_CAT25_RDY_Pos)


class EEPROM_CAT25
{
  public:
    EEPROM_CAT25(SPIClass * spi, const uint8_t chipSelect, const EEPROM_CAT25_Device device);

    void begin(void);
    void begin(const uint32_t clkSpeed);
    void end(void);

    bool isReady(void);
    void enableWrite(void);
    void disableWrite(void);
    uint8_t getStatusRegister(void);

    uint8_t readByte(const uint32_t address);
    size_t writeByte(const uint32_t address, const uint8_t byte);

    size_t readBlock(const uint32_t address, const size_t length, void * buffer);
    size_t writeBlock(uint32_t address, const size_t length, void * buffer);
    size_t writePage(const uint32_t address, const size_t length, void * buffer);

  protected:
    void startCommand(uint8_t command, const uint32_t address);
    void endCommand(void);

  private:
    SPIClass * _spi;
    SPISettings _spiSettings;
    EEPROM_CAT25_Device _device;
    size_t _capacity;
    size_t _pageSize;
    uint8_t _chipSelect;
    uint32_t _clkSpeed;

    void sendAddressBytes(const uint32_t address);
};

#endif

