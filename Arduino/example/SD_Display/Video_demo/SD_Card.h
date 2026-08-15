#pragma once
#include "Arduino.h"
#include <cstring>
#include "Display_ST7789.h"
#include "SD.h"
#include "FS.h"
#include <SPI.h>

// Digital I/O used
#define SD_CS     11       // SD_CS
#define SD_MOSI   12       // SD_MOSI
#define SD_MISO   14       // SD_MISO
#define SD_SCK    13       // SD_SCK


#define SD_SPI_HOST  HSPI   // ESP32-S3: HSPI = SPI3_HOST

extern SPIClass sdSPI;
extern uint16_t SDCard_Size;
extern uint16_t Flash_Size;

bool SD_Init();
void Flash_test();

bool File_Search(const char* directory, const char* fileName);
uint16_t Folder_retrieval(const char* directory, const char* fileExtension, char File_Name[][100],uint16_t maxFiles);
void remove_file_extension(char *file_name);