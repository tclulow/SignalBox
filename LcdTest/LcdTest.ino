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

#define LINE_WIDTH  10      // Line width

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


/** A point.
 */
typedef struct
{
    uint16_t locX;
    uint16_t locY;
} point_t;

/** A map of points.
 */
typedef struct
{
    uint16_t colour;
    uint8_t  width;
    point_t  points[];
} map_t;

/** A text.
 */
typedef struct
{
    uint16_t colour;
    uint8_t  size;
    point_t  loc;
    char*    text;    
} text_t;


map_t outerMain = { BLUE, LINE_WIDTH,
                    { {  20, 100 }, {  20,  70 }, {  70,  20 }, { 120,  20 }, { 170,  70 },
                      { 320,  70 }, { 360,  20 }, { 410,  20 }, { 460,  70 },
                      { 460, 250 }, { 410, 300 }, { 360, 300 }, { 300, 240 },
                      {  90, 240 }, {  20, 170 }, {  20, 100 }, {   0,   0 }
                    }
                  };

map_t innerMain = { GREEN, LINE_WIDTH,
                    { {  40,  80 }, {  80,  40 }, { 110,  40 }, { 160,  90 },
                      { 330,  90 }, { 370,  40 }, { 400,  40 }, { 440,  80 },
                      { 440, 240 }, { 400, 280 }, { 370, 280 }, { 310, 220 },
                      { 100, 220 }, {  40, 160 }, {  40,  80 }, {   0,   0 }
                    }
                  };

map_t crossOver1 = { CYAN,     LINE_WIDTH, { { 370, 280 }, { 340, 280 }, {   0,   0 } } };
map_t crossOver2 = { CYAN,     LINE_WIDTH, { { 300, 240 }, { 280, 220 }, {   0,   0 } } };
map_t crossOver3 = { CYAN,     LINE_WIDTH, { { 100, 220 }, {  70, 220 }, {   0,   0 } } };
map_t crossOver4 = { CYAN,     LINE_WIDTH, { { 180,  90 }, { 200,  70 }, {   0,   0 } } };
map_t crossOver5 = { CYAN,     LINE_WIDTH, { { 320,  70 }, { 345,  70 }, {   0,   0 } } };
map_t crossOver6 = { CYAN,     LINE_WIDTH, { { 340, 220 }, { 310, 220 }, {   0,   0 } } };

map_t northSide1 = { DARKGREY, LINE_WIDTH, { { 320, 260 }, { 280, 260 }, { 260, 280 }, {  70, 280 }, {  50, 300 }, {  20, 300 }, {   0,   0 } } };
map_t northSide2 = { DARKGREY, LINE_WIDTH, { { 320, 300 }, { 280, 300 }, { 260, 280 }, {   0,   0 }, } };
map_t northSide3 = { DARKGREY, LINE_WIDTH, { { 110, 240 }, {  70, 280 }, {   0,   0 } } };
map_t northSide4 = { DARKGREY, LINE_WIDTH, { {  90, 240 }, {  20, 240 }, {   0,   0 } } };

map_t southSide1 = { DARKGREY, LINE_WIDTH, { { 220,  70 }, { 270,  20 }, { 360,  20 }, {   0,   0 } } };
map_t southSide2 = { DARKGREY, LINE_WIDTH, { { 170,  40 }, { 250,  40 }, {   0,   0 } } };

map_t branchLine = { MAGENTA,  LINE_WIDTH, { {  70,  70 }, { 140, 140 }, 
                                             { 390, 140 }, { 420, 170 }, { 420, 210 }, { 390, 240 },
                                             { 360, 240 }, { 300, 180 }, {  60, 180 }, {   0,   0 } } };
map_t branchSide = { MAGENTA,  LINE_WIDTH, { {  82,  57 }, { 165, 140 }, {   0,   0 } } };
map_t branchLink = { MAGENTA,  LINE_WIDTH, { { 105,  80 }, { 105, 105 }, {   0,   0 } } };

map_t* allMaps[] = { &crossOver1, &crossOver2, &crossOver3, &crossOver4, &crossOver5, &crossOver6,
                     &northSide1, &northSide2, &northSide3, &northSide4,
                     &southSide1, &southSide2,
                     &branchLine, &branchSide, &branchLink,
                     &outerMain,  &innerMain
                   };


// All the static texts.
text_t texts[]   = { { BLACK, 2, { 100, 315 }, "Demo"  },
                     { BLACK, 1, { 230, 315 }, "(c)Copyright Tony Clulow 2021" }
                   };


/** Draw a map.
 *  Connect all the points in the map with a line in the given colour and width.
 */
void drawMap(map_t* aMap)
{
    uint8_t index = 1;
    
    point_t  *pointP  = aMap->points;  // Pointer to array of points.
    uint16_t fromX    = pointP->locX;   // Previous X co-ordinate.
    uint16_t fromY    = pointP->locY;   // Previous Y co-ordinate.

    int16_t  slope    = 0;              // Slope of the line.
    int16_t  adjustX  = 0;              // Slide line in X-direction.
    int16_t  adjustY  = 0;              // Slide line in Y-direction.
    boolean  vertical = false;          // Previous line was vertical.
    
    pointP += 1;
    while (   (pointP->locX != 0)       // End of map marker.
           || (pointP->locY != 0))
    {
        // Assume a simple case.
        adjustX = 0;
        adjustY = 0;
        slope   = (fromX - pointP->locX) * (fromY - pointP->locY);      

        if (slope == 0)
        {
            if (fromX == pointP->locX)
            {
                adjustX = fromY > pointP->locY ? 1 : -1;    // North / South.
            }
            else
            {
                adjustY = (fromX > pointP->locX ? -1 : 1);  // West / East.
            }
        }
        else if (vertical)
        {
            adjustX = fromY > pointP->locY ?  1 : -1;       // NW,NE - SW/SE
        }
        else
        {
            adjustY = fromX > pointP->locX ? -1 :  1;       // NW,SW - NE,SE
        }
       
//        Serial.print("step=");
//        Serial.print((int)pointP, HEX);
//        Serial.print(", ");
//        Serial.print(index);
//        Serial.print(",\tfromX=");
//        Serial.print(fromX);
//        Serial.print(",\tfromY=");
//        Serial.print(fromY);
//        Serial.print(",\tlocX=");
//        Serial.print(pointP->locX);
//        Serial.print(", \tlocY=");
//        Serial.print(pointP->locY);
//        Serial.print(", \tslope=");
//        Serial.print(slope);
//        Serial.print(",\tadjustX=");
//        Serial.print(adjustX);
//        Serial.print(",\tadjustY=");
//        Serial.print(adjustY);
//        Serial.println();
        
        for (uint8_t line = 0; line < aMap->width; line++)
        {
            tft.writeLine(fromX        + (adjustX * line), fromY        + (adjustY * line),
                          pointP->locX + (adjustX * line), pointP->locY + (adjustY * line),
                          aMap->colour);
        }

        vertical = (slope == 0) && (fromX == pointP->locX);

        fromX = pointP->locX;
        fromY = pointP->locY;

//        tft.setCursor(fromX, fromY);
//        tft.print(index);
//        tft.print(adjustX);
//        tft.print(adjustY);

        pointP += 1;
        index   += 1;
    }
}


/** Draw all the maps.
 */
void drawMaps()
{
    for (uint16_t map = 0; map < (sizeof(allMaps) / sizeof(map_t *)); map++)
    {
        drawMap(allMaps[map]);
    }
}


void drawText(text_t* aText)
{
    Serial.print(aText->text);
    Serial.print(" @ ");
    Serial.print(aText->loc.locX);
    Serial.print(",");
    Serial.print(aText->loc.locY);
    Serial.print(", size=");
    Serial.print(aText->size);
    Serial.println();
    
    tft.setTextColor(aText->colour);
    tft.setTextSize(aText->size);
    tft.setCursor(aText->loc.locX, aText->loc.locY);
    tft.print(aText->text);
}

/** Draw all the texts.
 */
void drawTexts()
{
    Serial.print("text=");
    Serial.print(sizeof(texts));
    Serial.print(", text_t=");
    Serial.print(sizeof(text_t));
    Serial.print(", texts[0]=");
    Serial.print(sizeof(texts[0]));
    Serial.print(", all=");
    Serial.print(sizeof(texts) / sizeof(texts[0]));
    Serial.println();
        
    for (uint16_t ind = 0; ind < (sizeof(texts) / sizeof(text_t)); ind++)
    {
        drawText(&texts[ind]);
    }
}


/** Sketch startup.
 */
void setup()
{
    Serial.begin(19200);
    
    tft.reset();
    tftID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(tftID, HEX);
    //    if (ID == 0xD3D3) ID = 0x9481; // write-only shield
    if (tftID == 0xD3D3) tftID = 0x9486; // write-only shield
    
    tft.begin(tftID);
    tft.setRotation(1);
    tft.fillScreen(WHITE);

    tft.setTextColor(BLACK);
    tft.setTextSize(1);
    tft.setTextWrap(false);
    tft.setFont(&FreeSans9pt7b);

    drawMaps();
    drawTexts();


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
