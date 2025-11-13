#include <Adafruit_NeoPixel.h>
#include <vector>
#include <algorithm>

#define MATRIX_PIN 33
#define ANALOG_X_PIN 34
#define ANALOG_Y_PIN 35
#define ANALOG_BUTTON_PIN 27

#define ROW 8
#define COL 8
#define STARTX 1
#define STARTY 1
#define GOALX  6
#define GOALY  6

Adafruit_NeoPixel matrix(ROW * COL, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

bool goalReached = false;
bool run = false;
bool canPress = true;

void setPixel(uint16_t x, uint16_t y, uint32_t color);

struct Node {
  Node* parent;
  uint16_t x, y, g, h, f;
  bool start, goal, solid, open, checked, path;

  Node()
      : parent(nullptr), x(0), y(0), g(0), h(0), f(0),
        start(false), goal(false), solid(false), open(false), checked(false), path(false) {}

  Node(uint16_t x, uint16_t y)
      : parent(nullptr), x(x), y(y), start(false), goal(false), solid(false), open(false), checked(false), path(false) {
        uint16_t xDistance = abs(x - STARTX);
        uint16_t yDistance = abs(y - STARTY);
        g = xDistance + yDistance;

        xDistance = abs(x - GOALX);
        yDistance = abs(y - GOALY);
        h = xDistance + yDistance;

        f = g + h;
      }
  bool operator==(const Node& other) const {
    return x == other.x && y == other.y;
  }
  void setAsStart(void) {
    start = true;
    //setPixel(x, y, matrix.Color(0, 200, 200));
  }
  void setAsGoal(void) {
    goal = true;
    //setPixel(x, y, matrix.Color(0, 230, 0));
  }
  void setAsSolid(void) {
    solid = true;
    //setPixel(x, y, matrix.Color(200, 200, 200));
  }
  void unset(void) {
    start = false;
    goal = false;
    solid = false;
    open = false;
    checked = false;
    path = false;
    //setPixel(x, y, 0);
  }

  void setAsOpen(void) {
    open = true;
  }
  void setAsChecked(void) {
    checked = true;
    // if(start == false && goal == false) {
    //   setPixel(x, y, matrix.Color(255, 165, 0));
    // }
  }
  void setAsPath(void) {
    path = true;
  }
};

Node grid[(uint16_t)ROW][(uint16_t)COL];
Node* currentNode = &grid[STARTY][STARTX];
std::vector<Node*> openList;
std::vector<Node*> checkedList;

void setPixel(uint16_t x, uint16_t y, uint32_t color) {
  matrix.setPixelColor((y * (uint16_t)ROW)+x, color);
}

void resetMatrix(void) {
  goalReached = false;
  canPress = true;
  run = false;

  for (uint16_t y = 0; y < ROW; y++) {
    for (uint16_t x = 0; x < COL; x++) {
      grid[y][x] = Node(x, y);
    }
  }
  grid[STARTY][STARTX].setAsStart();
  grid[GOALY][GOALX].setAsGoal();

  
  grid[5][5].setAsSolid();
  grid[6][5].setAsSolid();
  grid[4][5].setAsSolid();
  grid[3][5].setAsSolid();
  grid[2][5].setAsSolid();
  grid[1][5].setAsSolid();
  grid[1][4].setAsSolid();
  grid[1][3].setAsSolid();

}

void setCursor(uint16_t x, uint16_t y) {
  setPixel(x, y, matrix.Color(255, 255, 255));
}

void openNode(Node& node) {
  if (!node.open && !node.checked && !node.solid) {
    node.setAsOpen();
    node.parent = currentNode;
    openList.push_back(&node);
  }
}

void trackPath(void) {
  Node* cur = &grid[GOALY][GOALX];
  while(cur->x != STARTX || cur->y != STARTY) {
    cur = cur->parent;
    if(cur->x != STARTX || cur->y != STARTY) {
      cur->setAsPath();
    }
  }
}

void search(void) {
  if (!goalReached) {
    uint16_t y = currentNode->y;
    uint16_t x = currentNode->x;

    currentNode->setAsChecked();
    checkedList.push_back(currentNode);
    openList.erase(std::remove(openList.begin(), openList.end(), currentNode), openList.end());

    if (y - 1 >= 0)  openNode(grid[y - 1][x]);
    if (y + 1 < ROW) openNode(grid[y + 1][x]);
    if (x - 1 >= 0)  openNode(grid[y][x - 1]);
    if (x + 1 < COL) openNode(grid[y][x + 1]);

    if (openList.empty()) return;

    uint16_t bestNodeIndex = 0;
    uint16_t bestNodeF = 999;

    for (size_t i = 0; i < openList.size(); i++) {
      if (openList[i]->f < bestNodeF) {
        bestNodeIndex = i;
        bestNodeF = openList[i]->f;
      } else if (openList[i]->f == bestNodeF) {
        if (openList[i]->g < openList[bestNodeIndex]->g)
          bestNodeIndex = i;
      }
    }

    currentNode = openList[bestNodeIndex];

    if (currentNode->x == GOALX && currentNode->y == GOALY) {
      goalReached = true;
      canPress = true;
      trackPath();
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);

  matrix.begin();
  matrix.setBrightness(1); 
  matrix.clear();
  
  resetMatrix();
  
  matrix.show();
}

void render(void) {
  for(uint16_t y = 0; y < ROW; y++) {
    for(uint16_t x = 0; x < COL; x++) {
      if(grid[y][x].start) {
        setPixel(x, y, matrix.Color(0, 200, 200));
      } else if(grid[y][x].goal) {
        setPixel(x, y, matrix.Color(0, 230, 0));
      } else if(grid[y][x].solid) {
        setPixel(x, y, matrix.Color(200, 200, 200));
      } else if(grid[y][x].path) {
        setPixel(x, y, matrix.Color(255, 0, 0));
      } else if(grid[y][x].checked) {
        if(!grid[y][x].start && !grid[y][x].goal) {
         setPixel(x, y, matrix.Color(255, 165, 0));
        }
      }
    }
  }
}

void softReset(void) {
  openList.clear();
  checkedList.clear();
  // goalReached = false;
  for(uint16_t y = 0; y < ROW; y++) {
    for(uint16_t x = 0; x < COL; x++) {
      if(grid[y][x].checked || grid[y][x].open) {
        if(!grid[y][x].start && !grid[y][x].goal){
          grid[y][x].unset();
          setPixel(x, y, 0);
        }else {
          grid[y][x].checked = false;
          grid[y][x].open = false;
        }
      }
    }
  }
}

int cursorX = 0;
int cursorY = 0;
bool canMoveX = true;
bool canMoveY = true;

void loop() {
  
  //handle joystick and buttons

  int cursorRawX = analogRead(ANALOG_X_PIN);
  int cursorRawY = analogRead(ANALOG_Y_PIN);
  int pressed = digitalRead(ANALOG_BUTTON_PIN);

  if(!pressed && canPress) {
      canPress = false;
      run = !run;
  }
  if(!pressed && goalReached) {
    ESP.restart();
  }



  Serial.print(cursorRawX);
  Serial.print(" | ");
  Serial.print(cursorRawY);
  Serial.print(" | ");
  Serial.print(pressed);
  Serial.print(" | ");
  Serial.print(run);
  Serial.print(" | ");
  Serial.println(goalReached);

  if(cursorRawX >= 240 && cursorRawX <= 270) canMoveX = true;
  if(cursorRawY >= 240 && cursorRawY <= 270) canMoveY = true;

  if(canMoveX && !run) {
    if(cursorRawX >= 2000) {
      setPixel(cursorX, cursorY, 0);
      cursorX++;
      if(cursorX > 7) cursorX = 7;
      canMoveX = false;
    } else if(cursorRawX <= 30) {
      setPixel(cursorX, cursorY, 0);
      cursorX--;
      if(cursorX < 0) cursorX = 0;
      canMoveX = false;
    }
  }
  if(canMoveY && !run) {
    if(cursorRawY >= 2000) {
      setPixel(cursorX, cursorY, 0);
      cursorY++;
      if(cursorY > 7) cursorY = 7;
      canMoveY = false;
    } else if(cursorRawY <= 30) {
      setPixel(cursorX, cursorY, 0);
      cursorY--;
      if(cursorY < 0) cursorY = 0;
      canMoveY = false;
    }
  }
  


  

  //button = place walls



  if(run) {
    setPixel(cursorX, cursorY, 0);
    search();
  }
  render();

  if(!run) {
    setPixel(cursorX, cursorY, matrix.Color(255, 255, 255));
    softReset();
  } 

  matrix.show();



  if(run) delay(100);

  // if(!running){
  //   setCursor(cursorX, cursorY);
  //   matrix.show();
  //   delay(500);
  //   cursorX++;
  //   cursorY++;
  // }
}