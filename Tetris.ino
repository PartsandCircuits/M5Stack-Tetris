/*=========================================================================================
                           TETRIS for M5STACK 

  *  Ported 2018.01.20 to M5Stack by Macsbug  -  https://macsbug.wordpress.com/   or English translation:  https://translate.google.com/translate?sl=auto&tl=en&js=y&prev=_t&hl=en&ie=UTF-8&u=https%3A%2F%2Fmacsbug.wordpress.com%2F&edit-text=&act=url
  
  *  Original code from MhageGH  -  MhageGH/esp32_ILI9328_Tetris  -  https://github.com/MhageGH/esp32_ILI9328_Tetris   

  * SETUP:  Copy image file tetris.jpg to a micro SD card & insert in M5Stack.

       
   Controller : Buttons A = LEFT, B = RIGHT, C = START, ROTATE
   Display    : Left = 100x240, Center = 120x240, Right = 100x240
   Block      : 8ea, 12x12 pixel
   SD         : tetris.jpg : BackGround Image : R.G.B 320x240 pixel

=========================================================================================== */


#include <M5Stack.h>                                       // M5STACK

uint16_t BlockImage[8][12][12];                            // Block
uint16_t backBuffer[240][120];                             // GAME AREA
const int Length = 12;     // the number of pixels for a side of a block
const int Width  = 10;     // the number of horizontal blocks
const int Height = 20;     // the number of vertical blocks
int screen[Width][Height] = {0}; //it shows color-numbers of all positions
struct Point {int X, Y;};
struct Block {Point square[4][4]; int numRotate, color;};
Point pos; Block block;
int rot, fall_cnt = 0;
bool started = false, gameover = false;
boolean but_A = false, but_LEFT = false, but_RIGHT = false;
int game_speed = 25; // 25msec
Block blocks[7] = {
  {{{{-1,0},{0,0},{1,0},{2,0}},{{0,-1},{0,0},{0,1},{0,2}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,1},
  {{{{0,-1},{1,-1},{0,0},{1,0}},{{0,0},{0,0},{0,0},{0,0}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},1,2},
  {{{{-1,-1},{-1,0},{0,0},{1,0}},{{-1,1},{0,1},{0,0},{0,-1}},
  {{-1,0},{0,0},{1,0},{1,1}},{{1,-1},{0,-1},{0,0},{0,1}}},4,3},
  {{{{-1,0},{0,0},{0,1},{1,1}},{{0,-1},{0,0},{-1,0},{-1,1}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,4},
  {{{{-1,0},{0,0},{1,0},{1,-1}},{{-1,-1},{0,-1},{0,0},{0,1}},
  {{-1,1},{-1,0},{0,0},{1,0}},{{0,-1},{0,0},{0,1},{1,1}}},4,5},
  {{{{-1,1},{0,1},{0,0},{1,0}},{{0,-1},{0,0},{1,0},{1,1}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,6},
  {{{{-1,0},{0,0},{1,0},{0,-1}},{{0,-1},{0,0},{0,1},{-1,0}},
  {{-1,0},{0,0},{1,0},{0,1}},{{0,-1},{0,0},{0,1},{1,0}}},4,7}
};
//========================================================================
void setup(void) {
  Serial.begin(115200);         // SERIAL
  M5.begin();                   // M5STACK INITIALIZE
  M5.Lcd.setBrightness(200);    // BRIGHTNESS = MAX 255
  M5.Lcd.fillScreen(BLACK);     // CLEAR SCREEN
  M5.Lcd.setRotation(0);        // SCREEN ROTATION = 0
  //----------------------------// Make Block ----------------------------
  make_block( 0, BLACK);        // Type No, Color
  make_block( 1, 0x00F0);       // DDDD     RED
  make_block( 2, 0xFBE4);       // DD,DD    PUPLE 
  make_block( 3, 0xFF00);       // D__,DDD  BLUE
  make_block( 4, 0xFF87);       // DD_,_DD  GREEN 
  make_block( 5, 0x87FF);       // __D,DDD  YELLO
  make_block( 6, 0xF00F);       // _DD,DD_  LIGHT GREEN
  make_block( 7, 0xF8FC);       // _D_,DDD  PINK
  //----------------------------------------------------------------------
  M5.Lcd.drawJpgFile(SD, "/tetris.jpg");     // Load background from SD
  PutStartPos();                             // Start Position
  for (int i = 0; i < 4; ++i) screen[pos.X + 
   block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
  Draw();                                    // Draw block
}
//========================================================================
void loop() {
  if (gameover) return;
  Point next_pos;
  int next_rot = rot;
  GetNextPosRot(&next_pos, &next_rot);
  ReviseScreen(next_pos, next_rot);
  M5.update();
  delay(game_speed);                                      // SPEED ADJUST
}
//========================================================================
void Draw() {                               // Draw 120x240 in the center
  for (int i = 0; i < Width; ++i) for (int j = 0; j < Height; ++j)
   for (int k = 0; k < Length; ++k) for (int l = 0; l < Length; ++l)
    backBuffer[j * Length + l][i * Length + k] = BlockImage[screen[i][j]][k][l];
    M5.Lcd.drawBitmap(100, 0, 120, 240, (uint16_t *)backBuffer);
}
//========================================================================
void PutStartPos() {
  pos.X = 4; pos.Y = 1;
  block = blocks[random(7)];
  rot = random(block.numRotate);
}
//========================================================================
bool GetSquares(Block block, Point pos, int rot, Point* squares) {
  bool overlap = false;
  for (int i = 0; i < 4; ++i) {
    Point p;
    p.X = pos.X + block.square[rot][i].X;
    p.Y = pos.Y + block.square[rot][i].Y;
    overlap |= p.X < 0 || p.X >= Width || p.Y < 0 || p.Y >= 
      Height || screen[p.X][p.Y] != 0;
    squares[i] = p;
  }
  return !overlap;
}
//========================================================================
void GameOver() {
  for (int i = 0; i < Width; ++i) for (int j = 0; j < Height; ++j)
  if (screen[i][j] != 0) screen[i][j] = 4;
  gameover = true;
}
//========================================================================
void ClearKeys() { but_A=false; but_LEFT=false; but_RIGHT=false;}
//========================================================================
bool KeyPadLoop(){
  if(M5.BtnA.wasPressed()){ClearKeys();but_LEFT =true;return true;}
  if(M5.BtnB.wasPressed()){ClearKeys();but_RIGHT=true;return true;}
  if(M5.BtnC.wasPressed()){ClearKeys();but_A    =true;return true;}
  return false;
}
//========================================================================
void GetNextPosRot(Point* pnext_pos, int* pnext_rot) {
  bool received = KeyPadLoop();
  if (but_A) started = true;
  if (!started) return;
  pnext_pos->X = pos.X;
  pnext_pos->Y = pos.Y;
  if ((fall_cnt = (fall_cnt + 1) % 10) == 0) pnext_pos->Y += 1;
  else if (received) {
    if (but_LEFT) { but_LEFT = false; pnext_pos->X -= 1;}
    else if (but_RIGHT) { but_RIGHT = false; pnext_pos->X += 1;}
    else if (but_A) { but_A = false;
      *pnext_rot = (*pnext_rot + block.numRotate - 1)%block.numRotate; 
    }
  }
}
//========================================================================
void DeleteLine() {
  for (int j = 0; j < Height; ++j) {
    bool Delete = true;
    for (int i = 0; i < Width; ++i) if (screen[i][j] == 0) Delete = false;
    if (Delete) for (int k = j; k >= 1; --k) 
    for (int i = 0; i < Width; ++i) screen[i][k] = screen[i][k - 1];
  }
}
//========================================================================
void ReviseScreen(Point next_pos, int next_rot) {
  if (!started) return;
  Point next_squares[4];
  for (int i = 0; i < 4; ++i) screen[pos.X + 
    block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = 0;
  if (GetSquares(block, next_pos, next_rot, next_squares)) {
   for (int i = 0; i < 4; ++i){
     screen[next_squares[i].X][next_squares[i].Y] = block.color;
   }
   pos = next_pos; rot = next_rot;
  }
  else {
   for (int i = 0; i < 4; ++i) screen[pos.X + 
    block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
   if (next_pos.Y == pos.Y + 1) {
    DeleteLine(); PutStartPos();
    if (!GetSquares(block, pos, rot, next_squares)) {
     for (int i = 0; i < 4; ++i) screen[pos.X + 
      block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
      GameOver();
    }
   }
  }
  Draw();
}
//========================================================================
void make_block( int n , uint16_t color ){            // Make Block color       
  for ( int i =0 ; i < 12; i++ ) for ( int j =0 ; j < 12; j++ ){
    BlockImage[n][i][j] = color;                           // Block color
    if ( i == 0 || j == 0 ) BlockImage[n][i][j] = 0;       // BLACK Line
  } 
}
//========================================================================
