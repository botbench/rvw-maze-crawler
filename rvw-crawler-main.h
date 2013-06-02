/**
 * Robot Virtual Worlds maze crawling robot.
 *
 * Note: make sure you open the Virtual Display and the DebugStream windows
 *
 * Changelog:
 * - 0.1: Initial release
 *
 * License: You may use this code as you wish, provided you give credit where it's due.
 *
 * THIS CODE WILL ONLY WORK WITH ROBOTC VERSION 3.60 AND HIGHER.

 * Xander Soldaat (xander_at_botbench.com)
 * 21 May 2013
 * version 0.1
 */

// Taken from Driver Suite
#define min2(a, b) (a < b ? a : b)
#define max2(a, b) (a > b ? a : b)
#define clip(a, b, c) min2(c, max2(b, a))

// Easy to use defines for the walls
#define WALL_NORTH    0
#define WALL_EAST     1
#define WALL_SOUTH    2
#define WALL_WEST     3

// Size of the maze
#define MAZE_X        6
#define MAZE_Y        6

// The states for the main state machine
typedef enum {
  MAIN_STATE_BEGIN,
  MAIN_STATE_SCAN,
  MAIN_STATE_TURN,
  MAIN_STATE_CRUISE,
  MAIN_STATE_END,
} tMainState;

// The states for the scanning state machine
typedef enum
{
  SCAN_STATE_BEGIN,
  SCAN_STATE_TURNING_LEFT,
  SCAN_STATE_SCAN_LEFT,
  SCAN_STATE_TURNING_RIGHT,
  SCAN_STATE_SCAN_RIGHT,
  SCAN_STATE_END
} tScanScate;

// This is a tile struct
struct
{
  bool walls[4];
  ubyte scannedWalls;
  int distance;
  int x;
  int y;
} tMazeTile;

// A maze is made of X*Y tiles
tMazeTile maze[MAZE_X][MAZE_Y];

// Pointers to the current and finish tile
tMazeTile *currentTile;
tMazeTile *finishTile;

// Some constants that are used by the program, you shouldn't modify these
const int SCAN_SPEED = 0;
const int SCAN_FIX_DIST_SPEED = 30;
const int SCAN_SIDE_DISTANCE = 20;
const int SCAN_SIDE_DISTANCE_MARGIN = 15;
const int ENCODER_TICK_PER_TILE = 1930;
const int CRUISE_SPEED = 30;
const int TURN_SPEED = 0;

// The heading we'd like to maintain
float requiredHeading = 0.0;

// The power to be sent to the motors
float requiredPower = 0.0;

// The actual heading the robot has
float currentHeading = 0.0;

// The heading relative to the required Heading in -179 to 180 degrees
float currentRelativeHeading = 0.0;

// The distance currently detected by the Ultra Sound sensor
int currentDistance = 0;

// Our position in the maze
int x_pos = 0;
int y_pos = 0;

// The direction (0-3) we're cruising in
int cruiseDir = 0;

// Variables to hold current and previous states
tMainState mainState;
tMainState oldMainState;
tScanScate scanState;
tScanScate oldScanState;

// Function prototypes for the main FSM
void doMainStateBegin();
void doMainStateScan();
void doMainStateTurn();
void doMainStateCruise();

// Function prototypes for the scanning FSM
void doScanStateBegin();
void doScanStateTurnLeft();
void doScanStateScanLeft();
void doScanStateTurnRight();
void doScanStateScanRight();

// Changes the current state for one of the FSMs,
// keeps track of the previous one
void setMainState(tMainState newState);
void setScanState(tScanScate newState);

// Various other functions that are required by the FSMs
void setWall(tMazeTile *tile, float heading, bool hasWall, bool setAdjacent = true);
void updateMotors(int power, float heading);
int headingToDirection(float heading);
float normaliseHeading(float abormalHeading);
float readRelativeHeading(float relativeTarget);
void doCorrectSide(int desired, int margin);
