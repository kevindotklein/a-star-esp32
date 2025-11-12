#include <Adafruit_NeoPixel.h>
#include <vector>
#include <algorithm>

#define MATRIX_PIN 33
#define ROW 8
#define COL 8
#define STARTX 1
#define STARTY 1
#define GOALX  6
#define GOALY  6

Adafruit_NeoPixel matrix(ROW * COL, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

bool running = false;
bool goalReached = false;

void setPixel(uint16_t x, uint16_t y, uint32_t color);

struct Node {
  Node* parent;
  uint16_t x, y, g, h, f;
  bool start, goal, solid, open, checked;

  Node()
      : parent(nullptr), x(0), y(0), g(0), h(0), f(0),
        start(false), goal(false), solid(false), open(false), checked(false) {}

  Node(uint16_t x, uint16_t y)
      : parent(nullptr), x(x), y(y), start(false), goal(false), solid(false), open(false), checked(false) {
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
    setPixel(x, y, matrix.Color(0, 200, 200));
  }
  void setAsGoal(void) {
    goal = true;
    setPixel(x, y, matrix.Color(0, 230, 0));
  }
  void setAsSolid(void) {
    solid = true;
    setPixel(x, y, matrix.Color(200, 200, 200));
  }
  void unset(void) {
    start = false;
    goal = false;
    solid = false;
    open = false;
    checked = false;
    setPixel(x, y, 0);
  }

  void setAsOpen(void) {
    open = true;
  }
  void setAsChecked(void) {
    checked = true;
    if(start == false && goal == false) {
      setPixel(x, y, matrix.Color(255, 165, 0));
    }
  }
  void setAsPath(void) {
    setPixel(x, y, matrix.Color(255, 0, 0));
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
      trackPath();
    }
  }
}


void setup() {
  matrix.begin();
  matrix.setBrightness(1); 
  matrix.clear();
  
  resetMatrix();
  
  matrix.show();
}

uint16_t cursorX = 0, cursorY = 0;

void loop() {

  //if(!running) setPixel(cursorX-1, cursorY-1, 0);


  search();
  matrix.show();
  delay(500);

  // if(!running){
  //   setCursor(cursorX, cursorY);
  //   matrix.show();
  //   delay(500);
  //   cursorX++;
  //   cursorY++;
  // }
}