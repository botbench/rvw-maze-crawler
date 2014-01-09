#pragma config(StandardModel, "RVW REMBOT")
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/**
 * Robot Virtual Worlds maze crawling robot.
 *
 * Note: make sure you open the Virtual Display and the DebugStream windows
 *
 * Changelog:
 * - 0.1: Initial release
 *
 * License: This code is licensed under GPL v3
 *
 * THIS CODE WILL ONLY WORK WITH ROBOTC VERSION 3.60 AND HIGHER.
 *
 * Xander Soldaat (xander_at_botbench.com)
 */

// main include
#include "rvw-crawler-main.h"

// This includes the wall follower route planner
#include "planner-wall-follower.h"

/**
 * Task to constantly read the sensors and convert the heading
 * into a float
 */

task updateSensors
{
  float tmpHeading;

  while (true)
  {
    tmpHeading = SensorValue[gyro] / 10;
    currentHeading = normaliseHeading(tmpHeading);
    currentRelativeHeading = readRelativeHeading(requiredHeading);
    currentDistance = SensorValue[sonar];
    EndTimeSlice();
  }
}


/**
 * Draws and maintains a HUD (heads up display) on the virtual
 * NXT screen. Displays the maze, heading and distance detected
 * by the sonar
 */
task updateScreen
{
  eraseDisplay();
	int x1, x2, y1, y2;
	int x_compass, y_compass;

	// center of the compass
	int x_center = 89;
	int y_center = 53;

	while (true)
  {
    // We're constantly redrawing the entire maze
    // this is not the most efficient way of doing it
    // but it does the trick
    eraseDisplay();
    switch(mainState)
    {
      case MAIN_STATE_BEGIN: nxtDisplayTextLine(0, "BEGIN");   break;
      case MAIN_STATE_CRUISE: nxtDisplayTextLine(0, "CRUISE"); break;
      case MAIN_STATE_SCAN: nxtDisplayTextLine(0, "SCAN"); break;
      case MAIN_STATE_TURN: nxtDisplayTextLine(0, "TURN"); break;
      case MAIN_STATE_END: nxtDisplayTextLine(0, "END"); break;
    }

    for (int x = 0; x < MAZE_X; x++)
    {
      // Draw the compass and display the heading and distance values
	    nxtDrawCircle(x_center - 10, y_center + 10, 22);
	    x_compass = (cosDegrees(-1 * (currentHeading - 90)) * 10) + x_center;
	    y_compass = (sinDegrees(-1 * (currentHeading - 90)) * 10) + y_center;
	    nxtDrawLine(x_center, y_center, x_compass, y_compass);
	    nxtDisplayStringAt(x_center - 12, y_center - 15, "%3d", round(currentHeading));
      nxtDisplayStringAt(x_center - 12, y_center - 23, "%3d", currentDistance);

      // Draw the robot
      nxtDrawCircle(3 + y_pos*8, (x_pos*8) + 7, 5);

      // Draw the walls of the maze
      for (int y = 0; y < MAZE_Y; y++)
      {
			  x1 = y*8+1;
			  x2 = y*8+9;
			  y1 = x*8+1;
			  y2 = x*8+9;

			  if (maze[x][y].walls[0])
			    nxtDrawLine(x1, y2, x2, y2);

			  if (maze[x][y].walls[2])
			    nxtDrawLine(x1, y1, x2, y1);

				if (maze[x][y].walls[1])
			  	nxtDrawLine(x2, y1, x2, y2);

				if (maze[x][y].walls[3])
				  nxtDrawLine(x1, y2, x1, y1);
			}
		}
		wait1Msec(50);
  }
}

/**
 * Task to control the motors.  Attemps to maintain its heading
 * by using a PID (just the P) regulator.
 */
task controlMotors
{
  // The value of 5.0 seems to work well for the MammalBot
  float kP = 5.0;
  int powerL;
  int powerR;
  while (true)
  {
    powerR = round(requiredPower + currentRelativeHeading * kP);
    powerL = round(requiredPower - currentRelativeHeading * kP);

    // Clip the motor speeds to between -100 and +100
    motor[rightMotor] = clip(-100, powerR, 100);
    motor[leftMotor]  = clip(-100, powerL, 100);

	  wait1Msec(10);
  }
}


/**
 * Main task, it runs the main FSM
 */
task main()
{
  // This makes sure the debugstream window is not cluttered with stuff
  // from a previous run.
  clearDebugStream();

  // Start all of the tasks.
  StartTask(updateSensors);
  StartTask(updateScreen);
  StartTask(controlMotors);
  wait1Msec(100);

  // Start main FSM
  setMainState(MAIN_STATE_BEGIN);

  while (mainState != MAIN_STATE_END)
  {
    switch(mainState)
    {
      case MAIN_STATE_BEGIN: doMainStateBegin();
                             break;

      case MAIN_STATE_SCAN:  doMainStateScan();
                             break;

      case MAIN_STATE_TURN:  doMainStateTurn();
                             break;

      case MAIN_STATE_CRUISE:doMainStateCruise();
                             break;

      default:               writeDebugStreamLine("MAIN_STATE: ILLEGAL STATE");
    }
  }
}


/**
 * MAIN_STATE_BEGIN
 *
 * Initialise the maze tiles and presets the walls
 * into a float
 */
void doMainStateBegin()
{
  writeDebugStreamLine("doMainStateBegin");

  // Initialise the tiles in the maze
  for (int x = 0; x < MAZE_X; x++)
  {
    for (int y = 0; y < MAZE_Y; y++)
    {
      maze[x][y].walls[WALL_NORTH] = false;
      maze[x][y].walls[WALL_EAST] =  false;
      maze[x][y].walls[WALL_SOUTH] = false;
      maze[x][y].walls[WALL_WEST] =  false;
      maze[x][y].x = x;
      maze[x][y].y = y;
      maze.scannedWalls = 0;
      maze[x][y].distance = 0;
    }
  }

  // These are left and right borders, don't update adjacent tiles
  for (int x = 0; x < MAZE_X; x++)
  {
    setWall(maze[x][0], 270, true, false);
    setWall(maze[x][5], 90, true, false);
  }

  // The top and bottom borders, don't update adjacent tiles
  for (int y = 0; y < MAZE_Y; y++)
  {
    setWall(maze[5][y], 0, true, false);
    setWall(maze[0][y], 180, true, false);
  }

  // Initialise the currentTile pointer to the first element in the maze
  currentTile = &maze[0][0];
  finishTile = &maze[5][0];

  // Set the required heading to 0, which is north
  requiredHeading = 0;

  // Go to the next state
  setMainState(MAIN_STATE_SCAN);
}


/**
 * MAIN_STATE_SCAN
 *
 * This is the FSM that handles the scanning of each tile as we enter it.
 */
void doMainStateScan()
{
  writeDebugStreamLine("doMainStateScan");

  scanState = SCAN_STATE_BEGIN;
  while (scanState != SCAN_STATE_END)
  {
    switch(scanState)
    {
      case SCAN_STATE_BEGIN:        doScanStateBegin();
                                    break;

      case SCAN_STATE_TURNING_LEFT: doScanStateTurnLeft();
		                                break;

      case SCAN_STATE_SCAN_LEFT: 	  doScanStateScanLeft();
                                    break;

      case SCAN_STATE_TURNING_RIGHT:doScanStateTurnRight();
		                                break;

      case SCAN_STATE_SCAN_RIGHT:   doScanStateScanRight();
			                              break;

      default:                      writeDebugStreamLine("MAIN_STATE: ILLEGAL STATE");
                                    break;
    }
  }

  // Go to the next main state
  setMainState(MAIN_STATE_TURN);
}


/**
 * MAIN_STATE_TURN
 *
 * Here we see which way we'll go next.  The actual decision is made
 * by a route planning function, routePlannerFollowWall() in this case
 */
void doMainStateTurn()
{
  writeDebugStreamLine("doMainStateTurn");

  int currentDirection = headingToDirection(currentHeading);
  int newHeading = requiredHeading;

  // Get a new heading from the route planning function
  newHeading = planRoute(currentDirection, cruiseDir, maze, currentTile);

  // Don't bother to do anything if the newHeading is the same
  // as the current one.
  if (newHeading == requiredHeading)
  {
    setMainState(MAIN_STATE_CRUISE);
    return;
  }

  writeDebugStreamLine("doMainStateTurn: dir: %d, curr: %d, new: %d", currentDirection, currentHeading, newHeading);
  updateMotors(TURN_SPEED, newHeading);

  // wait for the turning to be completed.
  while (abs(currentRelativeHeading) > 5)
    wait1Msec(10);

  wait1Msec(1000);

  // Go to the next state
  setMainState(MAIN_STATE_CRUISE);
}


/**
 * MAIN_STATE_CRUISE
 *
 * Here we see which way we'll go next.  The actual decision is made
 * by a route planning function, routePlannerFollowWall() in this case
 */
void doMainStateCruise()
{
  writeDebugStreamLine("doMainStateCruise");

  // Reset the encoder counts
  nMotorEncoder[leftMotor] = 0;
	nMotorEncoder[rightMotor] = 0;
	wait1Msec(100);

  // Set the motors to cruising speed
  updateMotors(CRUISE_SPEED, requiredHeading);

  cruiseDir = headingToDirection(requiredHeading);

  // Get the current encoder count total
  long current_total = 0;

  // If we've travelled ENCODER_TICK_PER_TILE encoder ticks, go to the next step
  while((current_total < ENCODER_TICK_PER_TILE))
  {
	  current_total = nMotorEncoder[leftMotor] + nMotorEncoder[rightMotor];
	  wait1Msec(10);
	}

	// We've come to the middle of a new tile, update the
	// current position within the maze
	switch(headingToDirection(currentHeading))
	{
	  case 0: x_pos++; break;
	  case 1: y_pos++; break;
	  case 2: x_pos--; break;
	  case 3: y_pos--; break;
	}

  writeDebugStreamLine("x: %d, y: %d", x_pos, y_pos);

  // Update the pointer for the current tile
  currentTile = &maze[x_pos][y_pos];
  writeDebugStreamLine("currentTile: %p", currentTile);

  // If we've reached this point, then we're done.
  if (currentTile == finishTile)
  //if (x_pos == 5 && y_pos == 0)
  {
    writeDebugStreamLine("all done!");
    setMainState(MAIN_STATE_END);
    wait1Msec(100);
    return;
  }

  // Go to the next state
  if (currentTile->scannedWalls == 15)
  {
    // No point in scanning a tile we already know
    writeDebugStreamLine("tile[%d][%d] (%p) already fully known!", x_pos, y_pos, currentTile);
    setMainState(MAIN_STATE_TURN);
  }
  else
    setMainState(MAIN_STATE_SCAN);
}


/**
 * SCAN_STATE_BEGIN
 *
 * Initialise the scan, check the front distance and set the wall
 * of the current tile, if necessary
 */
void doScanStateBegin()
{
  ubyte leftDir =  headingToDirection(normaliseHeading(currentHeading - 90));
  ubyte rightDir = headingToDirection(normaliseHeading(currentHeading + 90));

  writeDebugStreamLine("doScanStateBegin");
	writeDebugStreamLine("front: %d", currentDistance);
	if (currentDistance < 35)
	  setWall(currentTile, currentHeading, true);
	else
	  setWall(currentTile, currentHeading, false);

  // Correct the distance to the wall if necessary
	doCorrectSide(SCAN_SIDE_DISTANCE, SCAN_SIDE_DISTANCE_MARGIN);

	// Skip scanning walls that already known
	if (currentTile->scannedWalls & (1 << leftDir))
	{
    writeDebugStreamLine("doScanStateBegin: left side has already been scanned: %d", leftDir);

		if (currentTile->scannedWalls & (1 << rightDir))
		{
	    writeDebugStreamLine("doScanStateBegin: right side has already been scanned: %d", rightDir);
	    setScanState(SCAN_STATE_END);
	    return;
	  }
	  else
	  {
	    setScanState(SCAN_STATE_TURNING_RIGHT);
	    return;
	  }
	}

	// Go to the next state
	setScanState(SCAN_STATE_TURNING_LEFT);
}


/**
 * SCAN_STATE_TURNING_LEFT
 *
 * Turn the robot to the left
 */
void doScanStateTurnLeft()
{
  writeDebugStreamLine("doScanStateTurnLeft");

  float newHeading = normaliseHeading(requiredHeading - 90);
  updateMotors(SCAN_SPEED, newHeading);

  // Wait for the robot to be done turning
  while (abs(currentRelativeHeading) > 5)
    wait1Msec(10);

  wait1Msec(1000);

  // Go the next state
  setScanState(SCAN_STATE_SCAN_LEFT);
}


/**
 * SCAN_STATE_SCAN_LEFT
 *
 * Check the distance on the left and set the wall of the current
 * tile, if necessary
 */
void doScanStateScanLeft()
{
  writeDebugStreamLine("doScanStateScanLeft");

	if (currentDistance < 25)
	  setWall(currentTile, currentHeading, true);
	else
	  setWall(currentTile, currentHeading, false);

  // Correct the distance to the wall if necessary
  doCorrectSide(SCAN_SIDE_DISTANCE, SCAN_SIDE_DISTANCE_MARGIN);

	if (currentTile->scannedWalls & (1 << headingToDirection(currentHeading + 180)))
	{
    writeDebugStreamLine("doScanStateScanLeft: right side has already been scanned: %d", 1 << headingToDirection(currentHeading + 180));
    setScanState(SCAN_STATE_END);
  }
  else
  {
    setScanState(SCAN_STATE_TURNING_RIGHT);
  }
}


/**
 * SCAN_STATE_TURNING_RIGHT
 *
 * Turn the robot to the right
 */
void doScanStateTurnRight()
{
  writeDebugStreamLine("doScanStateTurnRight");

  float newHeading = 0.0;

  if (oldScanState == SCAN_STATE_BEGIN)
     newHeading = normaliseHeading(requiredHeading +  90);
  else
    newHeading  = normaliseHeading(requiredHeading + 180);

	updateMotors(SCAN_SPEED, newHeading);

  // Wait for the robot to be done turning
  while (abs(currentRelativeHeading) > 5)
    wait1Msec(10);

  wait1Msec(1000);

  setScanState(SCAN_STATE_SCAN_RIGHT);
}


/**
 * SCAN_STATE_SCAN_RIGHT
 *
 * Check the distance on the right and set the wall of the current
 * tile, if necessary
 */
void doScanStateScanRight()
{
  writeDebugStreamLine("doScanStateScanRight: %d", currentDistance);
	if (currentDistance < (SCAN_SIDE_DISTANCE + SCAN_SIDE_DISTANCE_MARGIN))
	  setWall(currentTile, currentHeading, true);
	else
	  setWall(currentTile, currentHeading, false);

  // Correct the distance to the wall if necessary
  doCorrectSide(SCAN_SIDE_DISTANCE, SCAN_SIDE_DISTANCE_MARGIN);

  setScanState(SCAN_STATE_END);
}


/**
 * Function to update the motor speed and heading
 * @param power the power to set the motors to
 * @param heading the heading of the robot
 */
void updateMotors(int power, float heading)
{
  writeDebugStreamLine("updateMotors");
  writeDebugStreamLine("power: %d, heading: %d", power, heading);
  // we want these updated atomically
  hogCPU();
  requiredPower = power;
  requiredHeading = heading;
  releaseCPU();
}


/**
 * Change a heading (0-360 degrees) to a direction (0-3)
 * where 0 = north, 1 = east, 2 = south, 3 is west
 * @param heading the heading of the robot
 * @return the direction (0-3)
 */
int headingToDirection(float heading)
{
  return ((round(heading) + 45) % 360) / 90;
}


/**
 * Turn a (negative) heading to a 0-360 degrees value
 * @param abormalHeading the heading to be normalised
 * @return a more sane heading between 0 and 360 degrees
 */
float normaliseHeading(float abormalHeading)
{
  int tmpHeading = round(abormalHeading * 10.0);
  tmpHeading %= 3600;
  abormalHeading = (float)tmpHeading / 10.0;
  return (abormalHeading < 0) ? abormalHeading + 360.0 : abormalHeading;
}


/**
 * Get the current heading as a value between -179 and 180, relative
 * to the suppplied heading
 * @param relativeTarget
 * @return the relative heading value
 */
float readRelativeHeading(float relativeTarget)
{
  // The black voodoo magic code below is courtsey of Gus from HiTechnic.
  int _tmpHeading = currentHeading * 10 - relativeTarget * 10 + 1800;
  _tmpHeading = (_tmpHeading >= 0 ? _tmpHeading % 3600 : 3600 - (-10 - _tmpHeading)%3600) - 1800;
  return (float)_tmpHeading / 10.0;
}


/**
 * Mark a wall in the specified tile as being present
 * @param tile pointer to the tile we're manipulating
 * @param side the side being marked as having a wall
 * @param heading the heading in which the robot is facing
 * @param setAdjacent whether or not to also set the wall in the adjacent tile (default is true)
 */
void setWall(tMazeTile *tile, float heading, bool hasWall, bool setAdjacent)
{
  writeDebugStreamLine("setWall");

  int index = headingToDirection(heading);

  writeDebugStreamLine("heading: %d, wall", heading, index);
  tile->walls[index] = hasWall;
  tile->scannedWalls |= (1 << index);

  // If we're not setting the adjacent cell's wall, return here.
  if (!setAdjacent)
    return;

  // Setting the walls for tiles that are adjacent to this one.
  if (index == WALL_NORTH && x_pos < 5)
  {
    writeDebugStreamLine("Updating adjacent %d %d, WALL_SOUTH", x_pos + 1, y_pos);
    maze[x_pos + 1][y_pos].walls[WALL_SOUTH] = hasWall;
    maze[x_pos + 1][y_pos].scannedWalls |= (1 << WALL_SOUTH);
  }
  else if (index == WALL_SOUTH && x_pos > 0)
  {
    writeDebugStreamLine("Updating adjacent %d %d, WALL_NORTH", x_pos - 1, y_pos);
    maze[x_pos - 1][y_pos].walls[WALL_NORTH] = hasWall;
    maze[x_pos - 1][y_pos].scannedWalls |= (1 << WALL_NORTH);
  }
  else if (index == WALL_EAST && y_pos < 5)
  {
    writeDebugStreamLine("Updating adjacent %d %d, WALL_WEST", x_pos, y_pos + 1);
    maze[x_pos][y_pos + 1].walls[WALL_WEST] = hasWall;
    maze[x_pos][y_pos + 1].scannedWalls |= (1 << WALL_WEST);
  }
  else if (index == WALL_WEST && y_pos > 0)
  {
	  writeDebugStreamLine("Updating adjacent %d %d, WALL_EAST", x_pos, y_pos - 1);
    maze[x_pos][y_pos - 1].walls[WALL_EAST] = hasWall;
    maze[x_pos][y_pos - 1].scannedWalls |= (1 << WALL_EAST);
  }

}

/**
 * Adjust the distance of the robot to the wall it faces
 * @param desired the distance to the wall we require
 * @param margin the margin beyond which we don't adjust the robot's position
 */
void doCorrectSide(int desired, int margin)
{
  writeDebugStreamLine("correcting side distance");

  if (currentDistance < (desired + margin))
  {
 	  writeDebugStreamLine("Adjusting robot's distance to wall: %d", currentDistance);
		if ((currentDistance - desired) < -1)
		{
		  updateMotors(-SCAN_FIX_DIST_SPEED, requiredHeading);
		}
		else
		{
		  updateMotors(SCAN_FIX_DIST_SPEED, requiredHeading);
		}

		while (abs(currentDistance - desired) > 1)
      wait1Msec(10);

  }

  updateMotors(0, requiredHeading);

  wait1Msec(100);
}


/**
 * Set the next state of the main FSM, also updates the value
 * of the previous state
 * @param newState the value of the next state
 */
void setMainState(tMainState newState)
{
  oldMainState = mainState;
  mainState = newState;
}


/**
 * Set the next state of the scanning FSM, also updates the value
 * of the previous state
 * @param newState the value of the next state
 */
void setScanState(tScanScate newState)
{
  oldScanState = scanState;
  scanState = newState;
}
