#include "LCD_Image.h"
  
PNG png;
File Image_file;

uint16_t Image_CNT;   
char SD_Image_Name[100][100] ;    
char File_Image_Name[100][100] ;  

int16_t xpos = 0;
int16_t ypos = 0;
void * pngOpen(const char *filePath, int32_t *size) {
  Image_file = SD.open(filePath);
  *size = Image_file.size();
  return &Image_file;
}

void pngClose(void *handle) {
  File Image_file = *((File*)handle);
  if (Image_file) Image_file.close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
  if (!Image_file) return 0;
  page = page; // Avoid warning
  return Image_file.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position) {
  if (!Image_file) return 0;
  page = page; // Avoid warning
  return Image_file.seek(position);
}
//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
static uint16_t lineBuffer[MAX_IMAGE_WIDTH];
void pngDraw(PNGDRAW *pDraw) {
    uint16_t originalWidth = pDraw->iWidth;
    uint16_t size = originalWidth;
    
    //如果 PNG 解码器报告的宽度超过缓冲区容量，
    // 临时限制 iWidth 防止 getLineAsRGB565 写入越界
    if (size > MAX_IMAGE_WIDTH) {
        printf("WARNING: pngDraw iWidth=%d exceeds MAX_IMAGE_WIDTH=%d, clamping\r\n",
               size, MAX_IMAGE_WIDTH);
        pDraw->iWidth = MAX_IMAGE_WIDTH;
        size = MAX_IMAGE_WIDTH;
    }
    
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
    
    // 恢复原始宽度，避免影响 PNG 解码器后续行处理
    pDraw->iWidth = originalWidth;
    
    uint16_t y_start = pDraw->y;
    uint16_t y_end = pDraw->y;
    
    // 确保显示位置正确
    if (xpos < 0) xpos = 0;
    if (xpos + size > LCD_WIDTH) {
        size = LCD_WIDTH - xpos;
    }
    
    if (size > 0) {
        LCD_addWindow(xpos, y_start, xpos + size - 1, y_end, lineBuffer);
    }
    

    
//   png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
//   uint32_t size = pDraw->iWidth;
//   for (size_t i = 0; i < size; i++) {
//     lineBuffer[i] = (((lineBuffer[i] >> 8) & 0xFF) | ((lineBuffer[i] << 8) & 0xFF00));        //  所有数据修正
//   }
//   LCD_addWindow(xpos, pDraw->y, xpos + pDraw->iWidth, ypos + pDraw->y + 1,lineBuffer);                   // x_end End index on x-axis (x_end not included)
//  return 1;
}
/////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

void Search_Image(const char* directory, const char* fileExtension) {        
  Image_CNT = Folder_retrieval(directory,fileExtension,SD_Image_Name,100);
  if(Image_CNT) {  
    printf("Search_Image: Found %d '%s' files in %s\r\n", Image_CNT, fileExtension, directory);
    for (int i = 0; i < Image_CNT; i++) {
      strcpy(File_Image_Name[i], SD_Image_Name[i]);
      remove_file_extension(File_Image_Name[i]); 
      printf("  [%d] %s\r\n", i, SD_Image_Name[i]);
    }                  
  } else {
    printf("Search_Image: No '%s' files found in %s!\r\n", fileExtension, directory);
  }                                                             
}
void Show_Image(const char * filePath)
{
 // printf("Currently display picture %s\r\n",filePath);
  int16_t ret = png.open(filePath, pngOpen, pngClose, pngRead, pngSeek, pngDraw);  
 // printf("DEBUG: png.open returned: %d\r\n", ret); // 添加这行！               
  if (ret == PNG_SUCCESS) {                                                                          
  //  printf("image specs: (%d x %d), %d bpp, pixel type: %d\r\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType()); 
    
    uint32_t dt = millis();                                                                   
    if (png.getWidth() > MAX_IMAGE_WIDTH) {                                                 
   //   printf("Image too wide for allocated line buffer size!\r\n");                          
    }
    else {                                                                                  
      ret = png.decode(NULL, 0);                                                             
      png.close();                                                                      
    }                                                                        
   // printf("%d ms\r\n",millis()-dt);              
  }  
}

// 全局变量：是否已经扫描过文件
static bool imageListScanned = false;

void Display_Image(const char* directory, const char* fileExtension, uint16_t ID)
{
  // 只在第一次扫描文件列表
  if (!imageListScanned) {
    Search_Image(directory, fileExtension);
    imageListScanned = true;
  }
  
  if(Image_CNT && ID < Image_CNT) {
    String FilePath;
    if (String(directory) == "/") {                               // Handle the case when the directory is the root
      FilePath = String(directory) + SD_Image_Name[ID];
    } else {
      FilePath = String(directory) + "/" + SD_Image_Name[ID];
    }
    const char* filePathCStr = FilePath.c_str();          // Convert String to c_str() for Show_Image function
    printf("Show Image %d/%d: %s\r\n", ID+1, Image_CNT, filePathCStr);
    Show_Image(filePathCStr);                             // Show the image using the file path
  }
  else {
    printf("No files with extension '%s' found in directory: %s\r\n", fileExtension, directory);
  }
}
uint16_t Now_Image = 0;
void Image_Next(const char* directory, const char* fileExtension)
{
  if(!digitalRead(BOOT_KEY_PIN)){ 
    while(!digitalRead(BOOT_KEY_PIN));
    Now_Image ++;
    if(Now_Image == Image_CNT)
      Now_Image = 0;
    Display_Image(directory,fileExtension,Now_Image);
  }
}
void Image_Next_Loop(const char* directory, const char* fileExtension, uint32_t NextTime)
{
  static uint32_t lastSwitchTime = 0;
  static bool firstCall = true;
  uint32_t now = millis();
  
  // 只有0或1张图片时无需切换
  if (Image_CNT <= 1) {
    if (firstCall) {
      printf("Image_Next_Loop: Only %d image(s) found, auto-switch disabled\r\n", Image_CNT);
      firstCall = false;
    }
    return;
  }
  
  // 首次调用：初始化计时起点
  if (firstCall) {
    lastSwitchTime = now;
    firstCall = false;
    printf("Image_Next_Loop: Started with %d images, switch interval = %d ms\r\n", Image_CNT, NextTime);
    return;
  }
  
  // 到达切换时间
  if (now - lastSwitchTime >= NextTime) {
    lastSwitchTime = now;
    Now_Image++;
    if (Now_Image >= Image_CNT) {
      Now_Image = 0;
    }
    printf("Switching to image %d/%d\r\n", Now_Image + 1, Image_CNT);
    
    // 保存当前值
    uint16_t savedNow = Now_Image;
    Display_Image(directory, fileExtension, Now_Image);
    
    if (Now_Image != savedNow) {
      printf("ERROR: Now_Image corrupted! %d -> %d, restoring\r\n", savedNow, Now_Image);
      Now_Image = savedNow; 
    }
  }
}