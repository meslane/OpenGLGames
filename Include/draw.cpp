#include "draw.h"

#include <stdio.h>
#include <string.h>

void drawSegment(char* game, unsigned char width, unsigned char height, unsigned char startX, unsigned char startY, char dir) { //0 = right, 1 = up
	switch (dir) {
		case 0:
			for (unsigned int i = 0; i < 15; i++) { //15 bytes for RGB
				game[(width * startY * 3) + (startX * 3) + i] = 128;
			}
			break;
		case 1:
			for (unsigned int i = 0; i < 5; i++) {
				for (unsigned int j = 0; j < 3; j++) {
					game[(width * (startY + i) * 3) + (startX * 3) + j] = 128;
				}
			}
			break;
		default:
			break;
	}
}

/*   0
 * 2   1
 *   3
 * 5   4
 *   6
*/
void drawDigit(char* game, unsigned char digit, unsigned char width, unsigned char height, unsigned char x, unsigned char y) {
	for (unsigned int i = x; i <= (x + 4); i++) { //clear space
		for (unsigned int j = (y - 8); j <= y; j++) {
			for (unsigned int k = 0; k < 3; k++) {
				game[(width * j * 3) + (i * 3) + k] = 0;
			}
		}
	}

	switch (digit) { //0
	case 1:
	case 4:
		break;
	default:
		drawSegment(game, width, height, x, y, 0);
		break;
	}
	switch (digit) { //1
	case 5:
	case 6:
		break;
	default:
		drawSegment(game, width, height, x + 4, y - 4, 1);
		break;
	}
	switch (digit) { //2
	case 1:
	case 2:
	case 3:
	case 7:
		break;
	default:
		drawSegment(game, width, height, x, y - 4, 1);
		break;
	}
	switch (digit) { //3
	case 1:
	case 7:
	case 0:
		break;
	default:
		drawSegment(game, width, height, x, y - 4, 0);
		break;
	}
	switch (digit) { //4
	case 2:
		break;
	default:
		drawSegment(game, width, height, x + 4, y - 8, 1);
		break;
	}
	switch (digit) { //5
	case 2:
	case 6:
	case 8:
	case 0:
		drawSegment(game, width, height, x, y - 8, 1);
		break;
	default:
		break;
	}
	switch (digit) { //6
	case 1:
	case 4:
	case 7:
		break;
	default:
		drawSegment(game, width, height, x, y - 8, 0);
		break;
	}
}