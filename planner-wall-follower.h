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

/**
 * A simple wall following route planner
 * @param currentDirection the current direction
 * @param cruiseDirection the direction from which we entered the current tile
 * @param maze pointer to the maze
 * @param current pointer to the current maze tile
 * @return the new heading (0-3)
 */
int planRoute(int currentDirection, int cruiseDirection, tMazeTile *maze, tMazeTile *current)
{
  // go right
  if (!current->walls[(cruiseDirection + 1) % 4])
    return normaliseHeading((cruiseDirection * 90) + 90);

  // go straight
  if (!current->walls[(cruiseDirection + 0) % 4])
    return cruiseDirection * 90;

  // go left
  if (!current->walls[(cruiseDirection + 3) % 4])
    return normaliseHeading((cruiseDirection * 90) - 90);

  // go in reverse
  if (!current->walls[(cruiseDirection + 2) % 4])
    return normaliseHeading((cruiseDirection * 90) - 180);

  return 0;
}
