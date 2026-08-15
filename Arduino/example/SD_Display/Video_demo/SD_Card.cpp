#include "SD_Card.h"

uint16_t SDCard_Size;
uint16_t Flash_Size;


SPIClass sdSPI(SD_SPI_HOST);

bool SD_Init() {
  printf("Initializing MicroSD card with SPI...\r\n");
  printf("Pin configuration: CS=%d, MOSI=%d, CLK=%d, MISO=%d\r\n", 
         SD_CS, SD_MOSI, SD_SCK, SD_MISO);
  
  // 配置CS引脚
  pinMode(SD_CS, OUTPUT);    
  digitalWrite(SD_CS, HIGH);
  
  // MISO引脚添加上拉
  pinMode(SD_MISO, INPUT_PULLUP);
  
  delay(100);
  
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  delay(50);
  
  if (!SD.begin(SD_CS, sdSPI, 20000000)) {
    printf("\r\n========================================\r\n");
    printf("SD SPI initialization FAILED!\r\n");
    printf("========================================\r\n");
    return false;
  }
  
  printf("\r\nSD SPI initialization successful!\r\n");
  
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    printf("No SD card detected!\r\n");
    return false;
  }
  
  printf("Card detected successfully\r\n");
  printf("Card Type: ");
  if(cardType == CARD_MMC){
    printf("MMC\r\n");
  } else if(cardType == CARD_SD){
    printf("SDSC (Standard Capacity)\r\n");
  } else if(cardType == CARD_SDHC){
    printf("SDHC/SDXC (High/Extended Capacity)\r\n");
  } else {
    printf("UNKNOWN\r\n");
  }
  
  uint64_t totalBytes = SD.totalBytes();
  uint64_t usedBytes = SD.usedBytes();
  SDCard_Size = totalBytes/(1024*1024);
  
  printf("\r\nStorage Information:\r\n");
  printf("  Total: %llu MB (%.2f GB)\r\n", totalBytes/(1024*1024), totalBytes/(1024.0*1024*1024));
  printf("  Used:  %llu MB\r\n", usedBytes/(1024*1024));
  printf("  Free:  %llu MB\r\n", (totalBytes - usedBytes)/(1024*1024));
  printf("\r\nSD card ready to use!\r\n");
  
  return true;
}
bool File_Search(const char* directory, const char* fileName)    
{
  File Path = SD.open(directory);
  if (!Path) {
    printf("Path: <%s> does not exist\r\n",directory);
    return false;
  }
  File file = Path.openNextFile();
  while (file) {
    if (strcmp(file.name(), fileName) == 0) {                           
      if (strcmp(directory, "/") == 0)
        printf("File '%s%s' found in root directory.\r\n",directory,fileName);  
      else
        printf("File '%s/%s' found in root directory.\r\n",directory,fileName); 
      Path.close();                                                     
      return true;                                                     
    }
    file = Path.openNextFile();                                        
  }
  if (strcmp(directory, "/") == 0)
    printf("File '%s%s' not found in root directory.\r\n",directory,fileName);           
  else
    printf("File '%s/%s' not found in root directory.\r\n",directory,fileName);          
  Path.close();                                                         
  return false;                                                         
}
uint16_t Folder_retrieval(const char* directory, const char* fileExtension, char File_Name[][100],uint16_t maxFiles)    
{
  File Path = SD.open(directory);
  if (!Path) {
    printf("Path: <%s> does not exist\r\n",directory);
    return false;
  }
  
  uint16_t fileCount = 0;
  char filePath[100];
  File file = Path.openNextFile();
  while (file && fileCount < maxFiles) {
    if (!file.isDirectory() && strstr(file.name(), fileExtension)) {
      strncpy(File_Name[fileCount], file.name(), sizeof(File_Name[fileCount])); 
      if (strcmp(directory, "/") == 0) {                                      
        snprintf(filePath, 100, "%s%s", directory, file.name());   
      } else {                                                            
        snprintf(filePath, 100, "%s/%s", directory, file.name());
      }
      printf("File found: %s\r\n", filePath);
      fileCount++;
    }
    file = Path.openNextFile();                                      
  }
  Path.close();                                                         
  if (fileCount > 0) {
    printf(" %d <%s> files were retrieved\r\n",fileCount,fileExtension);
    return fileCount;                                                 
  } else {
    printf("No files with extension '%s' found in directory: %s\r\n", fileExtension, directory);
    return 0;                                                         
  }
}

void remove_file_extension(char *file_name) {
  char *last_dot = strrchr(file_name, '.');
  if (last_dot != NULL) {
    *last_dot = '\0'; 
  }
}

void Flash_test()
{
  printf("/********** RAM Test**********/\r\n");
  // Get Flash size
  uint32_t flashSize = ESP.getFlashChipSize();
  Flash_Size = flashSize/1024/1024;
  printf("Flash size: %d MB \r\n", flashSize/1024/1024);

  printf("/******* RAM Test Over********/\r\n\r\n");
}