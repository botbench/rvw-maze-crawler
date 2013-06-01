/**
 * CHANGE ME
 * @param link the port number
 * @return true if no error occured, false if it did
 */
int planroute(int dir, tMazeTile *maze, tMazeTile *current)
{
  // the cruisDir variable holds the direction in which we entered
  // the current tile.  Your turns are relative to that.

  // go right
  if (!currentTile->walls[(cruiseDir + 1) % 4])
    return normaliseHeading((cruiseDir * 90) + 90);

  // go straight
  if (!currentTile->walls[(cruiseDir + 0) % 4])
    return cruiseDir * 90;

  // go left
  if (!currentTile->walls[(cruiseDir + 3) % 4])
    return normaliseHeading((cruiseDir * 90) - 90);

  // go in reverse
  if (!currentTile->walls[(cruiseDir + 2) % 4])
    return normaliseHeading((cruiseDir * 90) - 180);

  return 0;
}
