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

EEPROM_CAT25::EEPROM_CAT25(SPIClass * const spi, const uint8_t chipSelect, const EEPROM_CAT25_Device device)
{
  _capacity = device.capacity;
  _pageSize = device.pageSize;
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

  if (!waitForReady()) {
    return(0);
  }

  startCommand(EEPROM_CAT25_COMMAND_READ, address);
  uint8_t ret = _spi->transfer(EEPROM_CAT25_DUMMY_BYTE);
  endCommand();

  return(ret);
}

size_t EEPROM_CAT25::writeByte(const uint32_t address, const uint8_t byte)
{
  return writeBlock(address, sizeof(byte), &byte);
}

size_t EEPROM_CAT25::updateByte(const uint32_t address, const uint8_t byte)
{
  return updateBlock(address, sizeof(byte), &byte);
}

size_t EEPROM_CAT25::readBlock(const uint32_t address, const size_t length, void * const buffer)
{
  if (length == 0 || (length + address) > _capacity) {
    return(0);
  }

  if (!waitForReady()) {
    return(0);
  }

  startCommand(EEPROM_CAT25_COMMAND_READ, address);
  _spi->transfer(buffer, length);
  endCommand();

  return(length);
}

size_t EEPROM_CAT25::writeBlock(uint32_t address, const size_t length, const void * const buffer)
{
  return writeOrUpdateBlock(false, address, length, buffer);
}

size_t EEPROM_CAT25::updateBlock(uint32_t address, const size_t length, const void * const buffer)
{
  return writeOrUpdateBlock(true, address, length, buffer);
}

size_t EEPROM_CAT25::writeOrUpdateBlock(bool update, uint32_t address, const size_t length, const void * const buffer)
{
  const uint8_t *buf = reinterpret_cast<const uint8_t *>(buffer);
  size_t len = length;

  if (length == 0 || (length + address) > _capacity) {
    return(0);
  }

  uint16_t remainderFirstPage = (_pageSize - (address % _pageSize));
  if (len < remainderFirstPage) {
    remainderFirstPage = len;
  }

  if (remainderFirstPage) {
    if (!writeOrUpdatePage(update, address, remainderFirstPage, buf)) {
      return(0);
    }
    buf += remainderFirstPage;
    len -= remainderFirstPage;
    address += remainderFirstPage;
  }

  while (len > _pageSize) {
    if (!writeOrUpdatePage(update, address, _pageSize, buf)) {
      return(0);
    }
    buf += _pageSize;
    len -= _pageSize;
    address += _pageSize;
  }
  if (len) {
    if (!writeOrUpdatePage(update, address, len, buf)) {
      return(0);
    }
  }

  return(length);
}

size_t EEPROM_CAT25::writePage(const uint32_t address, const size_t length, const void * const buffer)
{
  return writeOrUpdatePage(false, address, length, buffer);
}

size_t EEPROM_CAT25::updatePage(const uint32_t address, const size_t length, const void * const buffer)
{
  return writeOrUpdatePage(true, address, length, buffer);
}

size_t EEPROM_CAT25::writeOrUpdatePage(bool update, const uint32_t address, const size_t length, const void * const buffer)
{
  const uint8_t *buf = reinterpret_cast<const uint8_t *>(buffer);
  size_t len = length;
  uint32_t addr = address;

  if (address >= _capacity || length == 0 || length > (_pageSize - (address % _pageSize))) {
    return(0);
  }

  if (!waitForReady()) {
    return(0);
  }

  if (update) {
    // To prevent writing bytes that are unchanged, read bytes and
    // compare them, skipping any bytes that are unchanged. This only
    // skips initial bytes. To skip all non-changed bytes, the page
    // write might need to be split into multiple writes, which needs
    // multiple write cycles so takes significantly longer (and, I
    // guess, might even increase wear rather than decrease it?). You
    // could also try to skip bytes from the end, which would not need
    // an extra write cycle, but to do this efficiently, would require
    // reading memory backwards, which the EEPROM does not support, or
    // require reading all bytes to maybe discover the last one is
    // changed and all should be written.
    //
    // Note that this does not use readBlock but runs the read command
    // directly, to prevent allocating up to a full page of memory for
    // the bytes read, and to allow ending the read as soon as we found
    // a modified byte.
    startCommand(EEPROM_CAT25_COMMAND_READ, address);
    while (len) {
      uint8_t value = _spi->transfer(0);
      if (value != *buf)
        break;
      buf++;
      addr++;
      len--;
    }
    endCommand();
    if (!len)
      return(length);
  }

  enableWrite();
  startCommand(EEPROM_CAT25_COMMAND_WRITE, addr);
  while (len--) {
    _spi->transfer(*buf);
    buf++;
  }
  endCommand();

  return(length);
}

size_t EEPROM_CAT25::capacity() {
  return this->_capacity;
}

size_t EEPROM_CAT25::pageSize() {
  return this->_pageSize;
}

void EEPROM_CAT25::startCommand(uint8_t command, const uint32_t address)
{
  _spi->beginTransaction(_spiSettings);
  digitalWrite(_chipSelect, LOW);

  if (_capacity == 0x200 && address >= 0x100) {
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

  // Note that 4kbit (0x200) EEPROMS need 9 bits, but put the upper bit
  // in the command byte, so the second address byte is only used for
  // 8kbit and larger.
  if ((_capacity > 0x200)) {
    _spi->transfer((uint8_t)((address >> 8) & 0xFF));
  }

  _spi->transfer((uint8_t)(address & 0xFF));
}

void EEPROM_CAT25::endCommand(void)
{
  digitalWrite(_chipSelect, HIGH);
  _spi->endTransaction();
}

bool EEPROM_CAT25::waitForReady(void)
{
  uint32_t start = micros();
  uint32_t timeout = EEPROM_CAT25_TIMEOUT_TIME_MS * 1000;
  while (true) {
    if (isReady()) {
      return(true);
    }
    if (micros() - start > timeout) {
      return(false);
    }
    yield();
  }
}
