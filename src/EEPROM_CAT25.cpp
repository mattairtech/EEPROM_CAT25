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

#include "EEPROM_CAT25.h"

EEPROM_CAT25::EEPROM_CAT25(SPIClass * spi, const uint8_t chipSelect, const EEPROM_CAT25_Device device)
{
  switch (device)
  {
    case CAT25M01: _capacity = EEPROM_CAPACITY_CAT25M01; _pageSize = EEPROM_PAGE_SIZE_CAT25M01; break;
    case CAT25512: _capacity = EEPROM_CAPACITY_CAT25512; _pageSize = EEPROM_PAGE_SIZE_CAT25512; break;
    case CAT25256: _capacity = EEPROM_CAPACITY_CAT25256; _pageSize = EEPROM_PAGE_SIZE_CAT25256; break;
    case CAT25128: _capacity = EEPROM_CAPACITY_CAT25128; _pageSize = EEPROM_PAGE_SIZE_CAT25128; break;
    case CAT25640: _capacity = EEPROM_CAPACITY_CAT25640; _pageSize = EEPROM_PAGE_SIZE_CAT25640; break;
    case CAT25320: _capacity = EEPROM_CAPACITY_CAT25320; _pageSize = EEPROM_PAGE_SIZE_CAT25320; break;
    case CAT25160: _capacity = EEPROM_CAPACITY_CAT25160; _pageSize = EEPROM_PAGE_SIZE_CAT25160; break;
    case CAV25160: _capacity = EEPROM_CAPACITY_CAV25160; _pageSize = EEPROM_PAGE_SIZE_CAV25160; break;
    case CAT25080: _capacity = EEPROM_CAPACITY_CAT25080; _pageSize = EEPROM_PAGE_SIZE_CAT25080; break;
    case CAV25080: _capacity = EEPROM_CAPACITY_CAV25080; _pageSize = EEPROM_PAGE_SIZE_CAV25080; break;
    case CAT25040: _capacity = EEPROM_CAPACITY_CAT25040; _pageSize = EEPROM_PAGE_SIZE_CAT25040; break;
    case CAT25020: _capacity = EEPROM_CAPACITY_CAT25020; _pageSize = EEPROM_PAGE_SIZE_CAT25020; break;
    case CAT25010: _capacity = EEPROM_CAPACITY_CAT25010; _pageSize = EEPROM_PAGE_SIZE_CAT25010; break;
  }

  _device = device;
  _spi = spi;
  _chipSelect = chipSelect;
  _spiSettings = SPISettings();	// use default settings
}

void EEPROM_CAT25::begin(void)
{
  begin(4000000UL);
}

void EEPROM_CAT25::begin(const uint32_t clkSpeed)
{
  digitalWrite(_chipSelect, HIGH);
  pinMode(_chipSelect, OUTPUT);

  _clkSpeed = clkSpeed;
  _spiSettings = SPISettings(_clkSpeed, MSBFIRST, SPI_MODE0);

  _spi->begin();
}

void EEPROM_CAT25::end(void)
{
  pinMode(_chipSelect, INPUT);
}

uint8_t EEPROM_CAT25::getStatusRegister(void)
{
  startCommand(EEPROM_CAT25_COMMAND_RDSR, 0);
  uint8_t ret = _spi->transfer(EEPROM_CAT25_DUMMY_BYTE);
  endCommand();
  return(ret);
}

bool EEPROM_CAT25::isReady(void)
{
  if ((getStatusRegister() & EEPROM_CAT25_RDY_Msk) == EEPROM_CAT25_RDY_BUSY) {
    return(false);
  }
  return(true);
}

void EEPROM_CAT25::enableWrite(void)
{
  startCommand(EEPROM_CAT25_COMMAND_WREN, 0);
  endCommand();
}

void EEPROM_CAT25::disableWrite(void)
{
  startCommand(EEPROM_CAT25_COMMAND_WRDI, 0);
  endCommand();
}

uint8_t EEPROM_CAT25::readByte(const uint32_t address)
{
  if (address >= _capacity) {
    return(0);
  }

  while (!isReady()) {
    yield();
  }

  startCommand(EEPROM_CAT25_COMMAND_READ, address);
  uint8_t ret = _spi->transfer(EEPROM_CAT25_DUMMY_BYTE);
  endCommand();

  return(ret);
}

size_t EEPROM_CAT25::writeByte(const uint32_t address, const uint8_t byte)
{
  if (address >= _capacity) {
    return(0);
  }

  while (!isReady()) {
    yield();
  }

  enableWrite();
  startCommand(EEPROM_CAT25_COMMAND_WRITE, address);
  _spi->transfer(byte);
  endCommand();

  return(1);
}

size_t EEPROM_CAT25::readBlock(const uint32_t address, const size_t length, void * buffer)
{
  if (length == 0 || (length + address) > _capacity) {
    return(0);
  }

  while (!isReady()) {
    yield();
  }

  startCommand(EEPROM_CAT25_COMMAND_READ, address);
  _spi->transfer(buffer, length);
  endCommand();

  return(length);
}

size_t EEPROM_CAT25::writeBlock(uint32_t address, const size_t length, void * buffer)
{
  uint8_t *buf = reinterpret_cast<uint8_t *>(buffer);
  size_t len = length;

  if (length == 0 || (length + address) > _capacity) {
    return(0);
  }

  uint16_t remainderFirstPage = (_pageSize - (address % _pageSize));
  if (len < remainderFirstPage) {
    remainderFirstPage = len;
  }

  if (remainderFirstPage) {
    writePage(address, remainderFirstPage, buf);
    buf += remainderFirstPage;
    len -= remainderFirstPage;
    address += remainderFirstPage;
  }

  while (len > _pageSize) {
    writePage(address, _pageSize, buf);
    buf += _pageSize;
    len -= _pageSize;
    address += _pageSize;
  }
  writePage(address, len, buf);

  return(length);
}

size_t EEPROM_CAT25::writePage(const uint32_t address, const size_t length, void * buffer)
{
  uint8_t *buf = reinterpret_cast<uint8_t *>(buffer);
  size_t len = length;

  if (address >= _capacity || length == 0 || length > (_pageSize - (address % _pageSize))) {
    return(0);
  }

  while (!isReady()) {
    yield();
  }

  enableWrite();
  startCommand(EEPROM_CAT25_COMMAND_WRITE, address);
  while (len--) {
    _spi->transfer(*buf);
    buf++;
  }
  endCommand();

  return(length);
}

void EEPROM_CAT25::startCommand(uint8_t command, const uint32_t address)
{
  _spi->beginTransaction(_spiSettings);
  digitalWrite(_chipSelect, LOW);

  if (_device == CAT25040 && address >= 0x100) {
    if (command == EEPROM_CAT25_COMMAND_READ) {
      command = EEPROM_CAT25_COMMAND_READ_A8_HIGH;
    } else if (command == EEPROM_CAT25_COMMAND_WRITE) {
      command = EEPROM_CAT25_COMMAND_WRITE_A8_HIGH;
    }
  }

  _spi->transfer(command);

  if (command == EEPROM_CAT25_COMMAND_READ || command == EEPROM_CAT25_COMMAND_WRITE) {
    sendAddressBytes(address);
  }
}

void EEPROM_CAT25::sendAddressBytes(const uint32_t address)
{
  if (_capacity > 0x10000) {
    _spi->transfer((uint8_t)((address >> 16) & 0xFF));
  }

  if ((_capacity > 0x100) && (_device != CAT25040)) {
    _spi->transfer((uint8_t)((address >> 8) & 0xFF));
  }

  _spi->transfer((uint8_t)(address & 0xFF));
}

void EEPROM_CAT25::endCommand(void)
{
  digitalWrite(_chipSelect, HIGH);
  _spi->endTransaction();
}
