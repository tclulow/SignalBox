#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>


#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define RGB(r, g, b) (((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3))

#define GREY      RGB(127, 127, 127)
#define DARKGREY  RGB(64, 64, 64)
#define TURQUOISE RGB(0, 128, 128)
#define PINK      RGB(255, 128, 192)
#define OLIVE     RGB(128, 128, 0)
#define PURPLE    RGB(128, 0, 128)
#define AZURE     RGB(0, 128, 255)
#define ORANGE    RGB(255,128,64)
 

uint16_t tftID;

MCUFRIEND_kbv tft;
TouchScreen   ts = TouchScreen(6, A1, A2, 7, 300);        // XP, YP, XM, YM, resistance

Adafruit_GFX_Button button;


//void drawButton(Adafruit_GFX *gfx, const uint16_t x,      const uint16_t y, 
//                                   const  int16_t width,  const  int16_t height, const uint16_t radius,
//                                   const uint16_t colour, const uint16_t textColour, 
//                                   const char*    text)
//{
//    int16_t textX;
//    int16_t textY;
//    int16_t textWidth;
//    int16_t textHeight;
//    int16_t rectWidth  = width;
//    int16_t rectHeight = height;
//
//    // Calculate the bounds of the text in the given font.
//    gfx->getTextBounds(text, x, 0, &textX, &textY, &textWidth, &textHeight);
//
//    if (width < 0)
//    {
//        rectWidth = 2 * (-width) + textWidth;     // Make button large enough for border wound text.
//    }
//    
//    if (height < 0)
//    {
//        rectHeight = 2 * (-height) + textHeight;  // Make button large enough for border wound text.
//    }
//
//    textX = x + (rectWidth  - textWidth)  / 2;
//    textY = y + (rectHeight - textHeight) / 2 + textHeight * 6 / 7;
//    
//    Serial.print("X=");
//    Serial.print(x);
//    Serial.print(", textX=");
//    Serial.print(textX);
//    Serial.print(", textWidth=");
//    Serial.print(textWidth);
//    Serial.print(", rectWidth=");
//    Serial.print(rectWidth);
//    Serial.println();
//    Serial.print("Y=");
//    Serial.print(y);
//    Serial.print(", textY=");
//    Serial.print(textY);
//    Serial.print(", textHeight=");
//    Serial.print(textHeight);
//    Serial.print(", rectHeight=");
//    Serial.print(rectHeight);
//    Serial.println();
//    
//    gfx->fillRoundRect(x, y, rectWidth, rectHeight, radius, colour);
//    gfx->setCursor(textX, textY);
//    gfx->setTextColor(textColour);
//    gfx->print(text);
//}


typedef struct
{
    uint16_t toX;
    uint16_t toY;
} step_t;

typedef struct
{
    uint16_t colour;
    uint8_t  width;
    step_t   steps[];
} map_t;


map_t outerMain = { RED, 3,
                    { {  20,  70 },
                      {  70,  20 },
                      { 120,  20 },
                      { 150,  50 },
                      { 330,  50 },
                      { 360,  20 },
                      { 410,  20 },
                      { 460,  70 },
                      { 460, 250 },
                      { 410, 300 },
                      {  70, 300 },
                      {  20, 250 },
                      {  20,  70 },
                      {   0,   0 }
                    }
                  };

map_t innerMain = { BLUE, 3,
                    { {  30,  70 },
                      {  80,  20 },
                      { 110,  20 },
                      { 150,  50 },
                      { 330,  50 },
                      { 360,  20 },
                      { 410,  20 },
                      { 460,  70 },
                      { 460, 250 },
                      { 410, 300 },
                      {  70, 300 },
                      {  20, 250 },
                      {  20,  70 },
                      {   0,   0 }
                    }
                  };

map_t* allMaps[] = { &outerMain, &innerMain };


void drawMap(map_t* aMap)
{
    uint8_t  index = 0;
    step_t  *step  = aMap->steps;
    uint16_t fromX = step->toX;
    uint16_t fromY = step->toY;

    step += 1;
    while (   (step->toX != 0)
           || (step->toY != 0))
    {
        Serial.print("step=");
        Serial.print((int)step, HEX);
        Serial.print(", fromX=");
        Serial.print(fromX);
        Serial.print(", fromY=");
        Serial.print(fromY);
        Serial.print(", toX=");
        Serial.print(step->toX);
        Serial.print(", toY=");
        Serial.print(step->toY);
        Serial.println();
        
        boolean xWidth = fromY != step->toY;

        for (uint8_t width = 0; width < aMap->width; width++)
        {
            tft.writeLine(fromX     + (xWidth ? width : 0), fromY     + (xWidth ? 0 : width),
                          step->toX + (xWidth ? width : 0), step->toY + (xWidth ? 0 : width),
                          aMap->colour);
        }

        fromX = step->toX;
        fromY = step->toY;

        step += 1;
    }
}



void setup()
{
    Serial.begin(9600);
    
    tft.reset();
    tftID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(tftID, HEX);
    //    if (ID == 0xD3D3) ID = 0x9481; // write-only shield
    if (tftID == 0xD3D3) tftID = 0x9486; // write-only shield
    
    tft.begin(tftID);
    tft.setRotation(1);
    tft.fillScreen(WHITE);

    // Draw all the maps.
    for (uint16_t map = 0; map < (sizeof(allMaps) / sizeof(allMaps[0])); map++)
    {
        drawMap(allMaps[map]);
    }


//    tft.fillRoundRect(  50, 10, 100, 50, 10, RED);
//    tft.fillRoundRect( 200, 10, 100, 50, 10, GREEN);
//    tft.fillRoundRect( 350, 10, 100, 50, 10, BLUE);
//
//    button.initButton(&tft, 100, 110, 100, 50, WHITE, RED,    WHITE, "Red",   1);
//    button.drawButton(false);
//    button.initButton(&tft, 250, 110, 100, 50, WHITE, GREEN,  WHITE, "Green", 2);
//    button.drawButton(false);
//    button.initButton(&tft, 400, 110, 100, 50, WHITE, YELLOW, WHITE, "Yelly", 3);
//    button.drawButton(false);
//
//    tft.setFont(&FreeSans12pt7b);
//    tft.setTextSize(1);
//    drawButton(&tft,  50, 170, 100,  50, 10, RED,    WHITE, "Red");
//    drawButton(&tft, 200, 170, 100,  50, 10, GREEN,  WHITE, "Green");
//    drawButton(&tft, 350, 170, 100,  50, 10, YELLOW, WHITE, "Yell-y");
//
//    drawButton(&tft,  50, 230, -20, -10, 10, RED,    WHITE, "Red");
//    drawButton(&tft, 200, 230, -20, -10, 10, GREEN,  WHITE, "Green");
//    drawButton(&tft, 350, 230, -20, -10, 10, YELLOW, WHITE, "Yelly");

//    tft.setCursor(0, 300);
//    tft.setTextColor(WHITE);
//    tft.setTextSize(1);
//    tft.setTextWrap(true);
//    tft.setFont(&FreeSans9pt7b);  tft.print("Nine ");
//    tft.setFont(&FreeSans12pt7b); tft.print("twelve ");
//    tft.setFont(&FreeSans18pt7b); tft.print("eighteen ");
//    tft.setFont(&FreeSans24pt7b); tft.print("twentyfour ");

    
}

void loop(void)
{
}
