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
