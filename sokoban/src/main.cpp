#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <limits>
#include <string.h>
#include <time.h>
#include "NautilusRand.h"

using namespace std;

// easy to use macros
#define IS_EMPTY(tile) (tile == EMPTY || tile == '\x00')
#define PUSHABLE(tile) (\
  tile != PLAYER \
  && tile != PLAYER_ON_GOAL \
  && tile != PLAYER_ON_DOOR \
  && tile != WALL \
  && tile != DOOR \
  && tile != KEY \
  && tile != EMPTY \
  && tile != GOAL \
)

// game objects
const char PLAYER = '@';
const char BOX = 'O';
const char GOAL = '.';
const char WALL = '#';
const char EMPTY = ' ';
const char BOX_ON_GOAL = '*';
const char BOX_ON_DOOR = 'o';
const char PLAYER_ON_GOAL = '+';
const char PLAYER_ON_DOOR = 'd';
const char DOOR = 'D';
const char KEY = '$';

const int FULLWIDTH = 20;

extern "C" {

// Level data structure
typedef struct level_t {
  char width;
  char height;
  char keyX;
  char keyY;
  char playerX;
  char playerY;
  char originalTile;
  bool hasKey;
  char map[20 * 21];
} Level;

// Game maps
const int LEVELS = 5;
const int TMP_LEVEL = LEVELS;
int lastAuthenticated = 0x68;

// TODO: More maps!
Level levels[LEVELS + 1] = {
  {
    7, 6, -1, -1, 1, 1,
    ' ', false,
    {
      "#####-#             "
      "#     #             "
      "# O   #             "
      "#  ## #             "
      "#    .#             "
      "#######             "
    }
  },
  {
    9, 9, -1, -1, 1, 1,
    ' ', false,
    {
      "#####               "
      "#   #               "
      "# OO# ###           "
      "# O # #.#           "
      "### ###.#           "
      " ##    .#           "
      " #   #  #           "
      " #   ####           "
      " #####              "
    }
  },
  {
    8, 8, -1, -1, 4, 4,
    ' ', false,
    {
      "  ###               "
      "  #.#               "
      "  #O####            "
      "###  O.#            "
      "#.O  ###            "
      "####O#              "
      "   #.#              "
      "   ###              "
    }
  },
  {
    10, 7, -1, -1, 2, 1,
    ' ', false,
    {
      " #######            "
      " #     ###          "
      "##O###   #          "
      "#   O  O #          "
      "# ..# O ##          "
      "##..#   #           "
      " ########           "
    }
  },
  {
    20, 20, -1, -1, 2, 1,
    ' ', false,
    {
      "####################"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#        O         #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                 .#"
      "####################"
    }
  },
  {
    10, 3, -1, -1, 1, 1,
    ' ', false,
    {
      "##########          "
      "#    O.  #          "
      "##########          ",
    }
  }
};

}

Level *backupLevels[LEVELS + 1] = {NULL}; 

// Display the game map
void displayMap(int level)
{
  int playerX = levels[level].playerX;
  int playerY = levels[level].playerY;
  int keyX = levels[level].keyX;
  int keyY = levels[level].keyY;

  // Make the map 2x larger
  vector<vector<char>> resizedMap;

  // Initialize
  for (int i = 0; i < levels[level].height; i++) {
    resizedMap.push_back(vector<char>(levels[level].width * 2, ' '));
    resizedMap.push_back(vector<char>(levels[level].width * 2, ' '));
  }

  // Map
  for (int i = 0; i < levels[level].height; i++) {
    const char* row = &levels[level].map[i * FULLWIDTH];

    for (int j = 0; j < levels[level].width; j++) {
      switch (row[j]) {
        case BOX:
          resizedMap[i * 2][j * 2] = BOX;
          resizedMap[i * 2][j * 2 + 1] = BOX;
          resizedMap[i * 2 + 1][j * 2] = BOX;
          resizedMap[i * 2 + 1][j * 2 + 1] = BOX;
          break;
        case GOAL:
          resizedMap[i * 2][j * 2] = GOAL;
          resizedMap[i * 2][j * 2 + 1] = GOAL;
          resizedMap[i * 2 + 1][j * 2] = GOAL;
          resizedMap[i * 2 + 1][j * 2 + 1] = GOAL;
          break;
        case EMPTY:
          resizedMap[i * 2][j * 2] = EMPTY;
          resizedMap[i * 2][j * 2 + 1] = EMPTY;
          resizedMap[i * 2 + 1][j * 2] = EMPTY;
          resizedMap[i * 2 + 1][j * 2 + 1] = EMPTY;
          break;
        case BOX_ON_GOAL:
          resizedMap[i * 2][j * 2] = BOX_ON_GOAL;
          resizedMap[i * 2][j * 2 + 1] = BOX_ON_GOAL;
          resizedMap[i * 2 + 1][j * 2] = BOX_ON_GOAL;
          resizedMap[i * 2 + 1][j * 2 + 1] = BOX_ON_GOAL;
          break;
        case BOX_ON_DOOR:
          resizedMap[i * 2][j * 2] = BOX_ON_DOOR;
          resizedMap[i * 2][j * 2 + 1] = BOX_ON_DOOR;
          resizedMap[i * 2 + 1][j * 2] = BOX_ON_DOOR;
          resizedMap[i * 2 + 1][j * 2 + 1] = BOX_ON_DOOR;
          break;
        case PLAYER_ON_GOAL:
          resizedMap[i * 2][j * 2] = PLAYER_ON_GOAL;
          resizedMap[i * 2][j * 2 + 1] = PLAYER_ON_GOAL;
          resizedMap[i * 2 + 1][j * 2] = PLAYER_ON_GOAL;
          resizedMap[i * 2 + 1][j * 2 + 1] = PLAYER_ON_GOAL;
          break;
        case DOOR:
          resizedMap[i * 2][j * 2] = DOOR;
          resizedMap[i * 2][j * 2 + 1] = DOOR;
          resizedMap[i * 2 + 1][j * 2] = DOOR;
          resizedMap[i * 2 + 1][j * 2 + 1] = DOOR;
          break;
        case PLAYER_ON_DOOR:
          resizedMap[i * 2][j * 2] = PLAYER_ON_DOOR;
          resizedMap[i * 2][j * 2 + 1] = PLAYER_ON_DOOR;
          resizedMap[i * 2 + 1][j * 2] = PLAYER_ON_DOOR;
          resizedMap[i * 2 + 1][j * 2 + 1] = PLAYER_ON_DOOR;
          break;
        case KEY:
          resizedMap[i * 2][j * 2] = KEY;
          resizedMap[i * 2][j * 2 + 1] = KEY;
          resizedMap[i * 2 + 1][j * 2] = KEY;
          resizedMap[i * 2 + 1][j * 2 + 1] = KEY;
          break;
        default: // case WALL
          resizedMap[i * 2][j * 2] = WALL;
          resizedMap[i * 2][j * 2 + 1] = WALL;
          resizedMap[i * 2 + 1][j * 2] = WALL;
          resizedMap[i * 2 + 1][j * 2 + 1] = WALL;
          break;
      }
    }
  }

  // Player location
  if (playerY * 2 >= 0 && playerY * 2 < resizedMap.size()) {
    if (playerX * 2 >= 0 && playerX * 2 < resizedMap[playerY * 2].size()) {
      if (levels[level].map[playerY * FULLWIDTH + playerX] == PLAYER_ON_GOAL) {
        resizedMap[playerY * 2][playerX * 2] = PLAYER_ON_GOAL;
        resizedMap[playerY * 2][playerX * 2 + 1] = PLAYER_ON_GOAL;
        resizedMap[playerY * 2 + 1][playerX * 2] = PLAYER_ON_GOAL;
        resizedMap[playerY * 2 + 1][playerX * 2 + 1] = PLAYER_ON_GOAL;
      } else if (levels[level].map[playerY * FULLWIDTH + playerX] == PLAYER_ON_DOOR) {
        resizedMap[playerY * 2][playerX * 2] = PLAYER_ON_DOOR;
        resizedMap[playerY * 2][playerX * 2 + 1] = PLAYER_ON_DOOR;
        resizedMap[playerY * 2 + 1][playerX * 2] = PLAYER_ON_DOOR;
        resizedMap[playerY * 2 + 1][playerX * 2 + 1] = PLAYER_ON_DOOR;
      } else {
        resizedMap[playerY * 2][playerX * 2] = PLAYER;
        resizedMap[playerY * 2][playerX * 2 + 1] = PLAYER;
        resizedMap[playerY * 2 + 1][playerX * 2] = PLAYER;
        resizedMap[playerY * 2 + 1][playerX * 2 + 1] = PLAYER;
      }
    }
  }
  // Key location
  if (keyY * 2 >= 0 && keyY * 2 < resizedMap.size()) {
    if (keyX * 2 >= 0 && keyX * 2 < resizedMap[keyY * 2].size()) {
      resizedMap[keyY * 2][keyX * 2] = KEY;
      resizedMap[keyY * 2][keyX * 2 + 1] = KEY;
      resizedMap[keyY * 2 + 1][keyX * 2] = KEY;
      resizedMap[keyY * 2 + 1][keyX * 2 + 1] = KEY;
    }
  }

  cout << "===++===" << endl;
  for (const vector<char>& row : resizedMap) {
    string rowStr(row.begin(), row.end());
    cout << rowStr << endl;
  }
  cout << "========" << endl;
}

// Check if the player has won the game
bool checkWinCondition(int level)
{
  // is there any remaining goal tiles?
  bool hasGoalTiles = false;
  bool hasBoxTiles = false;
  bool hasBoxOnGoalTiles = false;
  for (int i = 0; i < levels[level].height; i++) {
    for (int j = 0; j < levels[level].width; j++) {
      if (levels[level].map[i * FULLWIDTH + j] == GOAL) {
        hasGoalTiles = true;
      }
      else if (levels[level].map[i * FULLWIDTH + j] == BOX) {
        hasBoxTiles = true;
      }
      else if (levels[level].map[i * FULLWIDTH + j] == BOX_ON_GOAL) {
        hasBoxOnGoalTiles = true;
    }
    }
  }
  return hasBoxOnGoalTiles && !hasBoxTiles && !hasGoalTiles;
}

// Move the player in the specified direction
void movePlayer(int level, int dx, int dy)
{
  Level &currentLevel = levels[level];
  int playerX = currentLevel.playerX;
  int playerY = currentLevel.playerY;

  int newX = playerX + dx;
  int newY = playerY + dy;
  char *nextTile = &currentLevel.map[newY * FULLWIDTH + newX];

#ifdef DEBUG
  printf("lastAuthenticated: %p\n", &lastAuthenticated);
  printf("the current tile (if the move was successful): %p %02x\n", nextTile, *nextTile);
#endif

  // Handle player movement
  if (IS_EMPTY(*nextTile) || *nextTile == GOAL || (*nextTile == DOOR && currentLevel.hasKey)) {
    if (currentLevel.map[playerY * FULLWIDTH + playerX] == PLAYER_ON_GOAL) {
      // restore the goal tile
      currentLevel.map[playerY * FULLWIDTH + playerX] = GOAL;
    } else if (currentLevel.map[playerY * FULLWIDTH + playerX] == PLAYER_ON_DOOR) {
      // restore the door tile
      currentLevel.map[playerY * FULLWIDTH + playerX] = DOOR;
    }
    currentLevel.playerX = newX;
    currentLevel.playerY = newY;
    if (*nextTile == GOAL) {
      *nextTile = PLAYER_ON_GOAL;
    } else if (*nextTile == DOOR) {
      *nextTile = PLAYER_ON_DOOR;
    }
  }

  // Handle box movement
  else if (PUSHABLE(*nextTile)) {
    int boxX = newX + dx;
    int boxY = newY + dy;
    char *nextBoxTile = &currentLevel.map[boxY * FULLWIDTH + boxX];

#ifdef DEBUG
    printf("Next box tile: %02x\n", *nextBoxTile);
    printf("Next box tile: %p\n", nextBoxTile);
#endif
    if (IS_EMPTY(*nextBoxTile) || *nextBoxTile == GOAL || (*nextBoxTile == DOOR && currentLevel.hasKey) || (*nextBoxTile == BOX_ON_DOOR && currentLevel.hasKey)) {
      char currentBoxTile = *nextTile;

      // set the old player tile
      if (currentLevel.map[playerY * FULLWIDTH + playerX] == PLAYER_ON_GOAL) {
        currentLevel.map[playerY * FULLWIDTH + playerX] = GOAL;
      } else if (currentLevel.map[playerY * FULLWIDTH + playerX] == PLAYER_ON_DOOR) {
        currentLevel.map[playerY * FULLWIDTH + playerX] = DOOR;
      } else {
        currentLevel.map[playerY * FULLWIDTH + playerX] = EMPTY;
      }
      // update player location
      currentLevel.playerX = newX;
      currentLevel.playerY = newY;
      // set next player tile
      if (*nextTile == BOX_ON_GOAL) {
        *nextTile = PLAYER_ON_GOAL;
      } else if (*nextTile == BOX_ON_DOOR) {
        *nextTile = PLAYER_ON_DOOR;
      } else {
        *nextTile = EMPTY;
      }
      // set next box tile
      if (*nextBoxTile == GOAL) {
        *nextBoxTile = BOX_ON_GOAL;
      } else if (*nextBoxTile == DOOR) {
        *nextBoxTile = BOX_ON_DOOR;
      } else {
        *nextBoxTile = currentBoxTile == BOX_ON_DOOR? BOX: currentBoxTile;
      }
    }
  }

  // Pick up keys
  if (!currentLevel.hasKey
      && currentLevel.keyX != -1
      && currentLevel.keyY != -1) {
#ifdef DEBUG
    printf("There is a key\n");
    printf("%d %d %d %d\n", currentLevel.playerX, currentLevel.playerY, currentLevel.keyX, currentLevel.keyY);
#endif
    if (currentLevel.playerX >= currentLevel.keyX - 1
        && currentLevel.playerX <= currentLevel.keyX + 1
        && currentLevel.playerY >= currentLevel.keyY - 1
        && currentLevel.playerY <= currentLevel.keyY + 1) {
      currentLevel.hasKey = true;
      // BUG: rewrite the tile to empty
      currentLevel.map[currentLevel.keyY * FULLWIDTH + currentLevel.keyX] = EMPTY;
      // the key is no longer on the map
      currentLevel.keyX = -1;
      currentLevel.keyY = -1;
      cout << "You picked up a key!" << endl;
    }
  }
}

// Generate a key on the map with a random position
void generateKey(int level)
{
  int tiles = levels[level].width * levels[level].height;

  while (true) {
    uint32_t randPos = NautilusGetRandVal() % tiles;
    int x = randPos % levels[level].width;
    int y = randPos / levels[level].width;
    if (levels[level].map[y * FULLWIDTH + x] != GOAL
        && levels[level].map[y * FULLWIDTH + x] != PLAYER
        && levels[level].map[y * FULLWIDTH + x] != BOX
        && levels[level].map[y * FULLWIDTH + x] != DOOR) {
      levels[level].keyX = randPos % levels[level].width;
      levels[level].keyY = randPos / levels[level].width;
#ifdef DEBUG
      printf("Key position: %d %d\n", levels[level].keyX, levels[level].keyY);
#endif
      break;
    }
  }
}

// Make door on a wall
void makeDoor(int level, int dx, int dy)
{
  Level &currentLevel = levels[level];
  int playerX = currentLevel.playerX;
  int playerY = currentLevel.playerY;

  int newX = playerX + dx;
  int newY = playerY + dy;
  char *nextTile = &currentLevel.map[newY * FULLWIDTH + newX];

  // you can only have one door at a time

  // create a map overlay
  char tempMap[20 * 20] = {0};
  for (int i = 0; i < currentLevel.height; i++) {
    for (int j = 0; j < currentLevel.width; j++) {
      tempMap[i * FULLWIDTH + j] = currentLevel.map[i * FULLWIDTH + j];
    }
  }
  // player
  tempMap[playerY * FULLWIDTH + playerX] = PLAYER;
  // key
  tempMap[currentLevel.keyY * FULLWIDTH + currentLevel.keyX] = KEY;

  for (int i = 0; i < currentLevel.height; i++) {
    for (int j = 0; j < currentLevel.width; j++) {
      if (tempMap[i * FULLWIDTH + j] == DOOR) {
        // there is already a door...
        cout << "There is already a door present on the map." << endl;
        return;
      }
    }
  }

  if (*nextTile == WALL) {
    // you cannot make a door on the surrounding wall tiles
    if (newY == 0) {
      if (currentLevel.map[0 * FULLWIDTH + newX] == WALL) {
        cout << "Invalid wall location." << endl;
        return;
      }
    }
    if (newY == currentLevel.height) {  // bug
      if (currentLevel.map[(currentLevel.height - 1) * FULLWIDTH + newX] == WALL) {
        cout << "Invalid wall location." << endl;
        return;
      }
    }
    if (newX == 0) {
      if (currentLevel.map[newY * FULLWIDTH + 0] == WALL) {
        cout << "Invalid wall location." << endl;
        return;
      }
    }
    if (newX == currentLevel.width - 1) {
      if (currentLevel.map[newY * FULLWIDTH + (currentLevel.width - 1)] == WALL) {
        cout << "Invalid wall location." << endl;
        return;
      }
    }
    // ok let's make a door
    *nextTile = DOOR;
    // generate a key as well
    generateKey(level);
  } else {
    cout << "Invalid wall location." << endl;
  }
}

void backupALevel(int level)
{
  if (backupLevels[level] == NULL) {
    backupLevels[level] = (Level*)malloc(sizeof(Level));
  }
  memcpy(backupLevels[level], &levels[level], sizeof(Level));
}

void restoreALevel(int level)
{
  if (backupLevels[level] != NULL) {
    memcpy(&levels[level], backupLevels[level], sizeof(Level));
    free(backupLevels[level]);
    backupLevels[level] = NULL;
  }
}

// Main menu
int mainMenu()
{
  while (true) {
    cout << "Welcome to Sokoban!" << endl;
    cout << "=========================" << endl;
    cout << "1. Play game" << endl;
    cout << "2. Choose a level and play" << endl;
    cout << "3. Upload a map" << endl;
    cout << "4. Exit" << endl;
    cout << endl;
    cout << "Enter your choice: ";

    int choice = 0;
    cin >> choice;
    // flush the input buffer
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (choice) {
      case 1:
        return 1;
      case 2:
        return 2;
      case 3:
        return 3;
      case 4:
        return 4;
      case 99:
        return 99;
      default:
        cout << "Invalid choice. Let's try again." << endl;
        return 3;
    }
  }
}

// Main game loop
int game(int initialLevel)
{
  char input;
  char level = initialLevel;
  bool should_exit = false;
  while (!should_exit) {
    if (backupLevels[level] == NULL) {
      // backup the current level for replaying
      backupALevel(level);
    }

    displayMap(level);
    if (checkWinCondition(level)) {
      cout << "Congratulations! You won Level " << (int)level << "." << endl;
      // restore the current level for replaying
      restoreALevel(level);
      level += 1;
      if (level < LEVELS) {
        cout << "Moving to the next level..." << endl;
        continue;
      } else if (level == LEVELS) {
        cout << "You have completed all levels!" << endl;
        break;
      } else {
        cout << "Good job!" << endl;
        break;
      }
    } else {
      cout << endl;
    }
    cout << "Enter your move (WASD): ";
    cin >> input;
    switch (input) {
      case 'w':
      case 'W':
        movePlayer(level, 0, -1);
        break;
      case 's':
      case 'S':
        movePlayer(level, 0, 1);
        break;
      case 'a':
      case 'A':
        movePlayer(level, -1, 0);
        break;
      case 'd':
      case 'D':
        movePlayer(level, 1, 0);
        break;
      case 'I':
      case 'i':
        makeDoor(level, 0, -1);
        break;
      case 'K':
      case 'k':
        makeDoor(level, 0, 1);
        break;
      case 'J':
      case 'j':
        makeDoor(level, -1, 0);
        break;
      case 'L':
      case 'l':
        makeDoor(level, 1, 0);
        break;
      default:
        cout << "Invalid move!" << endl;
        should_exit = true;
        break;
    }
  }
  return 0;
}

void godMode()
{
  int currentTime = time(NULL);

  bool authed = false;
#ifdef DEBUG
  printf("Last authenticated: %d\n", lastAuthenticated);
  printf("Time difference: %d\n", currentTime - lastAuthenticated);
#endif
  if (currentTime - lastAuthenticated <= 30) {
    cout << "Sudo mode is still enabled." << endl;
    authed = true;
  } else {
    cout << "Enter the flag for verification: " << endl;
    // flush cout
    cout.flush();

    string playerFlag;
    string theFlag;
    cin >> playerFlag;
    // flush the input buffer
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    // Load the flag from the file system
    ifstream flagFile("/flag");
    if (flagFile.is_open()) {
      getline(flagFile, theFlag);
      flagFile.close();
    } else {
      cout << "Cannot read the flag." << endl;
      return;
    }

    if (!strcmp(playerFlag.c_str(), theFlag.c_str())) {
      authed = true;
      lastAuthenticated = currentTime;
      cout << "God mode activated!" << endl;
    } else {
      cout << "Invalid flag." << endl;
      authed = false;
    }
  }

  if (authed) {

    // execute a command
    char cmd[1024] = {0};
    fgets(cmd, 1023, stdin);
    system(cmd);

    cout << "Now please enjoy the game!" << endl;
  }
}

void chooseLevel()
{
  cout << "Choose a level to play:" << endl;
  for (int i = 0; i < LEVELS; i++) {
    cout << i << ". Level " << i << endl;
  }
  cout << "Enter your choice: ";
  int choice = 0;
  cin >> choice;
  // flush the input buffer
  cin.clear();
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
  if (choice >= 0 && choice < sizeof(levels) / sizeof(Level)) {
    game(choice);
  } else {
    cout << "Invalid choice." << endl;
  }
}

bool verifySolution(const string& solution)
{
  Level level = levels[TMP_LEVEL];
  bool failed = false;

  int playerX = level.playerX;
  int playerY = level.playerY;

  backupALevel(TMP_LEVEL);

  for (char move : solution) {
    switch (move) {
      case 'w':
      case 'W':
        movePlayer(TMP_LEVEL, 0, -1);
        break;
      case 's':
      case 'S':
        movePlayer(TMP_LEVEL, 0, 1);
        break;
      case 'a':
      case 'A':
        movePlayer(TMP_LEVEL, -1, 0);
        break;
      case 'd':
      case 'D':
        movePlayer(TMP_LEVEL, 1, 0);
        break;
      default:
        failed = true;
        break;
    }
  }

  // Check if the player has won the game
  bool result = !failed && checkWinCondition(TMP_LEVEL);
  // restore the level
  restoreALevel(TMP_LEVEL);
  return result;
}

void uploadMap()
{
  cout << "Upload a map to play!" << endl;
  // Get the map dimension
  int width, height;
  cout << "Enter the width of the map: ";
  cin >> width;
  cout << "Enter the height of the map: ";
  cin >> height;

  // clear input buffer
  cin.clear();
  cin.ignore(numeric_limits<streamsize>::max(), '\n');

  if (width <= 2 || width >= 20 || height <= 2 || height >= 20) {
    cout << "Invalid map dimension." << endl;
    return;
  }

  // Get each line of the map
  cout << "Enter the map line by line:" << endl;
  string map[20];
  for (int i = 0; i < height; i++) {
    cout << "Line " << i << ": ";
    // flush cout
    cout.flush();

    char buffer[128] = {0};
    fgets(buffer, 127, stdin);
    if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n') {
      buffer[strlen(buffer) - 1] = '\x00';
    }
    map[i] = buffer;
  }

  // Verify the map
  bool has_goal = false, has_box = false;
  for (int i = 0; i < height; i++) {
    if (map[i].length() != width) {
      cout << "Invalid map." << endl;
      return;
    }
    // Check for invalid characters
    for (int j = 0; j < width; j++) {
      if (map[i][j] != WALL
          && map[i][j] != EMPTY
          && map[i][j] != BOX
          && map[i][j] != GOAL) {
        cout << "Invalid map." << endl;
        return;
      }
      if (map[i][j] == GOAL) {
        has_goal = true;
      } else if (map[i][j] == BOX) {
        has_box = true;
      }
    }
  }
  if (!has_goal || !has_box) {
    cout << "Invalid map." << endl;
    return;
  }
  // the map must be wrapped with walls
  for (int i = 0; i < height; i++) {
    if (map[i][0] != WALL || map[i][width - 1] != WALL) {
      cout << "Invalid map." << endl;
      return;
    }
  }

  // Get the initial player position
  int playerX, playerY;
  cout << "Enter the player's initial X position: ";
  cin >> playerX;
  cout << "Enter the player's initial Y position: ";
  cin >> playerY;

  // Vulnerability: Player position can be negative
  if (playerX >= width || playerY >= height) {
    cout << "Invalid player position." << endl;
    return;
  }

  // Get the intended solution
  cout << "Enter the intended solution (WASD): ";
  string solution;
  cin >> solution;

  // Copy the map to the temporary game level
  Level &tmpLevel = levels[TMP_LEVEL];
  tmpLevel.width = width;
  tmpLevel.height = height;
  tmpLevel.playerX = playerX;
  tmpLevel.playerY = playerY;
  tmpLevel.hasKey = false;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      tmpLevel.map[i * FULLWIDTH + j] = map[i][j];
    }
  }

  // Verify the intended solution
  cout << "Verifying the solution..." << endl;
  bool verified = verifySolution(solution);
  if (verified) {
    cout << "Your submission has been accepted!" << endl;
  } else {
    cout << "Incorrect solution. Is your map solvable?" << endl;
    // clear the map
    // Vulnerability: Now the walls are gone...
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        tmpLevel.map[i * FULLWIDTH + j] = EMPTY;
      }
    }
  }
}

// Main function
int main()
{
  while (true) {
    int choice = mainMenu();
    if (choice == 1) {
      game(0);
    } else if (choice == 2) {
      chooseLevel();
    } else if (choice == 3) {
      uploadMap();
    } else if (choice == 4) {
      cout << "See you next time!" << endl;
      break;
    } else if (choice == 99) {
      godMode();
    }
  }
}

