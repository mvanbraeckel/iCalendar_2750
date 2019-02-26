#include <stdio.h>

/* define enumerated type */
typedef enum Direction { NORTH, EAST, SOUTH, WEST } Direction;
/* NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 */

/* function prototypes */
Direction turnLeft(Direction dir);
Direction turnRight(Direction dir);
Direction turnAround(Direction dir);

void printDirection(Direction dir);


int main(void)
{
	Direction dir = NORTH;

	printDirection(dir);

	dir = turnLeft(dir);
	printDirection(dir);

	dir = turnAround(dir);
	printDirection(dir);

	dir = turnRight(dir);
	printDirection(dir);

	return 0;
}


Direction turnRight(Direction dir)
{
	printf("Turning right\n");
	return (dir + 1) % 4;
}


Direction turnLeft(Direction dir)
{
	printf("Turning left\n");
	return (dir + 3) % 4;
}


Direction turnAround(Direction dir)
{
	printf("Turning around\n");
	return (dir + 2) % 4;
}


void printDirection(Direction dir)
{
	printf("You are going ");

	switch (dir) {
	case NORTH:
		printf("North");
		break;
	case EAST:
		printf("East");
		break;
	case SOUTH:
		printf("South");
		break;
	case WEST:
		printf("West");
		break;
	default:
		/* should not happen */
		printf("Unknown Direction");
		break;
	}

	printf("\n");
}
