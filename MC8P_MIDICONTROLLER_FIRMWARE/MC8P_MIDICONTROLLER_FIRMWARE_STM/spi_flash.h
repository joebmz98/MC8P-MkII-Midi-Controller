// =============================================================================
// spi_flash.h — W25Q64 SPI flash driver (SPI1)
//
// Sector erase, page program and read, plus writeStructToFlash() /
// readStructFromFlash() for saving whole settings structs. A write erases the
// 4 KB sector first, so each struct gets a sector of its own (see storage.h).
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"

// =============================================
// W25Q64 SPI FLASH FUNCTIONS
// =============================================

// W25Q64 Commands
#define W25Q_CMD_WRITE_ENABLE 0x06
#define W25Q_CMD_WRITE_DISABLE 0x04
#define W25Q_CMD_READ_DATA 0x03
#define W25Q_CMD_FAST_READ 0x0B
#define W25Q_CMD_PAGE_PROGRAM 0x02
#define W25Q_CMD_SECTOR_ERASE 0x20
#define W25Q_CMD_BLOCK_ERASE_32K 0x52
#define W25Q_CMD_BLOCK_ERASE_64K 0xD8
#define W25Q_CMD_CHIP_ERASE 0xC7
#define W25Q_CMD_READ_STATUS1 0x05
#define W25Q_CMD_READ_STATUS2 0x35
#define W25Q_CMD_READ_ID 0x9F
#define W25Q_CMD_POWER_DOWN 0xB9
#define W25Q_CMD_RELEASE_PD 0xAB

// *****************************
// -- FLASH
// *****************************

void initFlash() {
  pinMode(FLASH_CS_PIN, OUTPUT);
  digitalWrite(FLASH_CS_PIN, HIGH);

  // Explicitly configure SPI pins - avoids PA4 hardware NSS conflict
  SPI.setMOSI(FLASH_MOSI_PIN);  // PA7
  SPI.setMISO(FLASH_MISO_PIN);  // PA6
  SPI.setSCLK(FLASH_SCK_PIN);   // PA5

  SPI.begin();
  Serial.println("SPI Flash initialized");
}

// Read Flash ID (for verification)
uint32_t readFlashID() {
  uint32_t id = 0;
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS_PIN, LOW);
  SPI.transfer(W25Q_CMD_READ_ID);
  id |= (uint32_t)SPI.transfer(0x00) << 16;
  id |= (uint32_t)SPI.transfer(0x00) << 8;
  id |= SPI.transfer(0x00);
  digitalWrite(FLASH_CS_PIN, HIGH);
  SPI.endTransaction();
  return id;
}

// Wait for flash to be ready (not busy)
void waitFlashReady() {
  uint8_t status;
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS_PIN, LOW);
  SPI.transfer(W25Q_CMD_READ_STATUS1);
  do {
    status = SPI.transfer(0x00);
  } while (status & 0x01);
  digitalWrite(FLASH_CS_PIN, HIGH);
  SPI.endTransaction();
}

// Write enable command
void flashWriteEnable() {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS_PIN, LOW);
  SPI.transfer(W25Q_CMD_WRITE_ENABLE);
  digitalWrite(FLASH_CS_PIN, HIGH);
  SPI.endTransaction();
}

// Erase a 4KB sector
void flashEraseSector(uint32_t address) {
  flashWriteEnable();
  waitFlashReady();

  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS_PIN, LOW);
  SPI.transfer(W25Q_CMD_SECTOR_ERASE);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  digitalWrite(FLASH_CS_PIN, HIGH);
  SPI.endTransaction();

  waitFlashReady();
  delay(50);
}

// Write multiple bytes to flash
void writeFlash(uint32_t address, const uint8_t* data, uint32_t length) {
  uint32_t offset = 0;
  while (length > 0) {
    uint32_t pageOffset = address & 0xFF;
    uint32_t bytesToWrite = min(length, (uint32_t)(256 - pageOffset));

    flashWriteEnable();
    waitFlashReady();

    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    digitalWrite(FLASH_CS_PIN, LOW);
    SPI.transfer(W25Q_CMD_PAGE_PROGRAM);
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);
    for (uint32_t i = 0; i < bytesToWrite; i++) {
      SPI.transfer(data[offset + i]);
    }
    digitalWrite(FLASH_CS_PIN, HIGH);
    SPI.endTransaction();

    waitFlashReady();
    address += bytesToWrite;
    offset += bytesToWrite;
    length -= bytesToWrite;
    delay(5);
  }
}

// Read multiple bytes from flash
void readFlash(uint32_t address, uint8_t* data, uint32_t length) {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS_PIN, LOW);
  SPI.transfer(W25Q_CMD_READ_DATA);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  for (uint32_t i = 0; i < length; i++) {
    data[i] = SPI.transfer(0x00);
  }
  digitalWrite(FLASH_CS_PIN, HIGH);
  SPI.endTransaction();
}

// Write a structure to flash (erases sector first)
template<typename T>
void writeStructToFlash(uint32_t address, const T& data) {
  // Align address to 4KB sector boundary for erase
  uint32_t sectorAddress = address & ~0xFFF;

  // Erase the sector containing this address
  flashEraseSector(sectorAddress);

  // Write the struct
  const uint8_t* ptr = (const uint8_t*)&data;
  writeFlash(address, ptr, sizeof(T));
}

// Read a structure from flash
template<typename T>
void readStructFromFlash(uint32_t address, T& data) {
  uint8_t* ptr = (uint8_t*)&data;
  readFlash(address, ptr, sizeof(T));
}
