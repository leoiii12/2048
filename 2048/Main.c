#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

const int STARTUP_BLOCK = 1;
int BOARD_LENGTH = 4;
int BOARD_SCALE = 1;
int MODE = 0;
HANDLE consoleHandle;

// Create a temporary board for rotating
int* tempBoard;

// Create a buffer for faster rendering
char BORDER[1024];

// This is faster clear screen function than system("cls")
void clearScreen(void)
{
	DWORD numberOfWrittenChars;
	DWORD size;
	COORD coord = {0};
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;

	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(h, &bufferInfo);
	size = bufferInfo.dwSize.X * bufferInfo.dwSize.Y;
	FillConsoleOutputCharacter(h, TEXT(' '), size, coord, &numberOfWrittenChars);
	GetConsoleScreenBufferInfo(h, &bufferInfo);
	FillConsoleOutputAttribute(h, bufferInfo.wAttributes, size, coord, &numberOfWrittenChars);
	SetConsoleCursorPosition(h, coord);
}


void randomlyInsertNumber(int board[])
{
	int randomIndex;

	// Search and insert
	while (1)
	{
		randomIndex = BOARD_LENGTH * (rand() % BOARD_LENGTH) + rand() % BOARD_LENGTH;
		if (board[randomIndex] == 0)
		{
			board[randomIndex] = ((rand() % 2) + 1) * 2;
			break;
		}
	}
}

void printWelcomeMessages()
{
	puts("This game is made by Leo Choi.\n");
}

void formatBoard(int board[])
{
	int index, numberOfInsertedValue;

	// Set all cells equal to 0
	for (index = 0; index < BOARD_LENGTH * BOARD_LENGTH; index++)
	{
		board[index] = 0;
	}

	// Generate cells
	for (numberOfInsertedValue = 0; numberOfInsertedValue < STARTUP_BLOCK; numberOfInsertedValue ++)
	{
		randomlyInsertNumber(board);
	}
}

void readBoardFromFile(int board[])
{
	FILE* mapRHandle = fopen("map.txt", "r");

	if (mapRHandle == NULL)
	{
		puts("\n[Error] Cannot open \"map.txt\". Exit now.");
		puts("Press any key to continue...");

		getch();
		exit(0);
	}
	else
	{
		int i, temp = -1;
		for (i = 0; i < BOARD_LENGTH * BOARD_LENGTH; i++)
		{
			fscanf(mapRHandle, "%d", &temp);
			if (temp == 2 || temp == 4 || temp == 8 || temp == 16 || temp == 32 || temp == 64 || temp == 128 || temp == 256 || temp == 512 || temp == 1024 || temp == 2048)
				board[i] = temp;
			else
				board[i] = 0;
		}

		fclose(mapRHandle);
	}
}

void initialiseRowBorder()
{
	strcpy(BORDER, "+----+----");
	int i;
	for (i = 2; i < BOARD_LENGTH; i++)
	{
		strcat(BORDER, "+----");
	}
	strcat(BORDER, "+");
}


void rotateBoard(int board[])
{
	int row, column, index;

	// Rotate
	for (row = 0; row < BOARD_LENGTH; row++)
	{
		for (column = 0; column < BOARD_LENGTH; column++)
		{
			tempBoard[row * BOARD_LENGTH + column] = board[column * BOARD_LENGTH + (BOARD_LENGTH - 1 - row)];
		}
	}

	// Copy to real board
	for (index = 0; index < BOARD_LENGTH * BOARD_LENGTH; index++)
	{
		board[index] = tempBoard[index];
	}
}


int slideNumbers(int board[])
{
	int isChanged = 0;
	int row, column;
	int index, nextIndex;
	int rightColumn;

	for (row = 0; row < BOARD_LENGTH; row++)
	{
		for (column = 0; column < BOARD_LENGTH; column++)
		{
			index = row * BOARD_LENGTH + column;

			// Find zero
			if (board[index] == 0)
			{
				// Slide the nearest right cell to fill the zero
				for (rightColumn = column + 1; rightColumn < BOARD_LENGTH; rightColumn++)
				{
					nextIndex = row * BOARD_LENGTH + rightColumn;

					// Find non-zero number
					if (board[nextIndex] > 0)
					{
						board[index] = board[nextIndex];
						board[nextIndex] = 0;

						isChanged = 1;
						break;
					}
				}
			}
		}
	}

	return isChanged;
}

int mergeNumbers(int board[])
{
	int isChanged = 0;
	int row, column;
	int index, nextIndex;

	for (row = 0; row < BOARD_LENGTH; row++)
	{
		for (column = 0; column < BOARD_LENGTH; column++)
		{
			// Merge the right one number if they are the same
			index = row * BOARD_LENGTH + column;
			nextIndex = row * BOARD_LENGTH + (column + 1);

			if (column + 1 != BOARD_LENGTH)
			{
				// Merge any non zero by equal number
				if (board[index] == board[nextIndex] && board[index] != 0)
				{
					board[index] = board[index] * 2;
					board[nextIndex] = 0;

					isChanged = 1;
				}
			}
		}
	}

	return isChanged;
}

int computeBoard(int board[])
{
	int isChanged = slideNumbers(board) + mergeNumbers(board);
	if (isChanged > 0)
		slideNumbers(board);

	return isChanged;
}


enum BoardStatus
{
	FREE,
	FULL,
	ACHIEVED
};

enum BoardStatus getBoardStatus(int board[])
{
	int index;

	// Basic check
	for (index = 0; index < BOARD_LENGTH * BOARD_LENGTH; index++)
	{
		if (board[index] == 2048)
			return ACHIEVED;
	}
	for (index = 0; index < BOARD_LENGTH * BOARD_LENGTH; index++)
	{
		if (board[index] == 0)
			return FREE;
	}

	int row, column;

	// Comprehensive check ( Check if any row can be merged )
	for (row = 0; row < BOARD_LENGTH; row++)
	{
		for (column = 0; column < BOARD_LENGTH; column++)
		{
			if (column + 1 != BOARD_LENGTH)
			{
				if (board[row * BOARD_LENGTH + column] == board[row * BOARD_LENGTH + column + 1])
					return FREE;
			}
		}
	}

	// Comprehensive check in different angle
	rotateBoard(board);
	for (row = 0; row < BOARD_LENGTH; row++)
	{
		for (column = 0; column < BOARD_LENGTH; column++)
		{
			if (column + 1 != BOARD_LENGTH)
			{
				if (board[row * BOARD_LENGTH + column] == board[row * BOARD_LENGTH + column + 1])
				{
					// Rotate back
					rotateBoard(board);
					rotateBoard(board);
					rotateBoard(board);
					return FREE;
				}
			}
		}
	}

	// Do not need to rotate back, because the game is ended
	return FULL;
}


int getNumberOfZero(int board[])
{
	int index, numberOfZeroCells = 0;

	// Get the number of free cells
	for (index = 0; index < BOARD_LENGTH * BOARD_LENGTH; index++)
	{
		if (board[index] == 0)
		{
			numberOfZeroCells = numberOfZeroCells + 1;
		}
	}

	return numberOfZeroCells;
}

void randomlyInsertNumbers(int board[])
{
	int numberOfPreparedInsert, numberOfInserted, numberOfZeroCells = getNumberOfZero(board);

	// Get the smaller number to prevent dead loop
	if (numberOfZeroCells < BOARD_SCALE)
		numberOfPreparedInsert = numberOfZeroCells;
	else
		numberOfPreparedInsert = BOARD_SCALE;

	// Insert one number randomly
	for (numberOfInserted = 0; numberOfInserted < numberOfPreparedInsert; numberOfInserted++)
	{
		randomlyInsertNumber(board);
	}
}


int getNumberColorCode(int number)
{
	switch(number)
	{
	case 2:
		return 31;
	case 4:
		return 159;
	case 8:
		return 63;
	case 16:
		return 79;
	case 32:
		return 95;
	case 64:
		return 111;
	case 128:
		return 143;
	case 256:
		return 127;
	case 512:
		return 47;
	case 1024:
		return 191;
	case 2048:
		return 175;
	}
}

void printBoard(int board[])
{
	clearScreen();

	puts(BORDER);

	int row, column, index;
	for (row = 0; row < BOARD_LENGTH; row++)
	{
		for (column = 0; column < BOARD_LENGTH; column++)
		{
			index = row * BOARD_LENGTH + column;
			if (board[index] == 0)
				printf("|    ");
			else
			{
				printf("|");
				SetConsoleTextAttribute(consoleHandle, getNumberColorCode(board[index]));
				printf("%4d", board[index]);
				SetConsoleTextAttribute(consoleHandle, 15);
			}
		}
		printf("|\n");
		puts(BORDER);
	}
}


void requestNumberInRange(int* number, int minimum, int maximum, char format[])
{
	while (1)
	{
		printf(format, minimum, maximum);
		scanf("%d", number);

		if (*number >= minimum && *number <= maximum)
			break;
		else
			puts("[Error] Please enter a valid number.");
	}
}

void requestForSeed()
{
	int seed = 0;
	requestNumberInRange(&seed, 0, 32767, "Random Seed - (%d ~ %d): ");
	srand(seed);
}

void requestForMode()
{
	while (1)
	{
		printf("Mode - Normal Mode (0) | Debug Mode (1): ");
		scanf("%d", &MODE);

		if (MODE == 0 || MODE == 1)
			break;
		else
			puts("[Error] Please enter a valid number.");
	}
}

void requestForLength()
{
	requestNumberInRange(&BOARD_LENGTH, 2, 30, "Length of board - (%d ~ %d) : ");

	BOARD_SCALE = (BOARD_LENGTH * BOARD_LENGTH) / 16;
	if (BOARD_SCALE == 0)
		BOARD_SCALE = 1;

	initialiseRowBorder();
}


void requestForDebugInsertNumber(int board[])
{
	if (getNumberOfZero(board) == 0)
	{
		puts("[Error] No empty cell. You should move first.");
		return;
	}

	int row = 0, column = 0, value = 0;

	while (1)
	{
		requestNumberInRange(&row, 0, BOARD_LENGTH - 1, "\nRow    - (%d ~ %d): ");
		requestNumberInRange(&column, 0, BOARD_LENGTH - 1, "Column - (%d ~ %d): ");

		while (1)
		{
			printf("Value  - (2 / 4): ");
			scanf("%d", &value);

			if (value == 2 || value == 4)
				break;
			else
				puts("[Error] Please enter a valid number.");
		}

		if (board[row * BOARD_LENGTH + column] == 0)
		{
			board[row * BOARD_LENGTH + column] = value;
			break;
		}
		else
		{
			puts("[Error] The cell is not empty.");
		}
	}
}

enum GameStatus
{
	INITIALISING,
	PLAYING,
	LOSE,
	WIN,
	END
};

int main(void)
{
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, 15);

	printWelcomeMessages();
	requestForLength();

	// Initialise boards
	int* board;
	board = (int*)malloc(sizeof(int) * BOARD_LENGTH * BOARD_LENGTH);

	// int* tempBoard is global variable
	tempBoard = (int*)malloc(sizeof(int) * BOARD_LENGTH * BOARD_LENGTH);

	enum GameStatus gameStatus = INITIALISING;

	while (1)
	{
		if (gameStatus == INITIALISING)
		{
			requestForMode();

			if (MODE == 0)
			{
				requestForSeed();
				formatBoard(board);
			}
			else
			{
				readBoardFromFile(board);
			}

			printBoard(board);

			if (MODE == 1)
			{
				requestForDebugInsertNumber(board);
				printBoard(board);
			}

			gameStatus = PLAYING;
		}

		if (gameStatus == PLAYING)
		{
			int isCorrectInput = 1;
			int isChanged = 0;
			switch (getch())
			{
			case 0x48:
			case 0x33:
				rotateBoard(board);
				isChanged = computeBoard(board);
				rotateBoard(board);
				rotateBoard(board);
				rotateBoard(board);
				break;
			case 0x4D:
			case 0x32:
				rotateBoard(board);
				rotateBoard(board);
				isChanged = computeBoard(board);
				rotateBoard(board);
				rotateBoard(board);
				break;
			case 0x50:
			case 0x34:
				rotateBoard(board);
				rotateBoard(board);
				rotateBoard(board);
				isChanged = computeBoard(board);
				rotateBoard(board);
				break;
			case 0x4B:
			case 0x31:
				isChanged = computeBoard(board);
				break;
			case 0x35:
				gameStatus = END;
				isCorrectInput = 0;
				break;
			case 0x36:
				gameStatus = INITIALISING;
				isCorrectInput = 0;
				break;
			case 224:
				isCorrectInput = 0;
				break;
			default:
				puts("[Error] Please press the arrow key or enter LEFT(1) | RIGHT(2) | UP(3) | DOWN(4) | QUIT(5) | RESTART(6).");
				isCorrectInput = 0;
				break;
			}

			if (isCorrectInput == 1)
			{
				enum BoardStatus boardStatus = getBoardStatus(board);
				switch (boardStatus)
				{
				case FREE:
					if (isChanged > 0)
					{
						printBoard(board);

						if (MODE == 0)
						{
							randomlyInsertNumbers(board);
						}
						else
						{
							requestForDebugInsertNumber(board);
						}

						printBoard(board);
					}
					else
					{
						puts("[Error] Invalid direction.");
					}
					break;
				case FULL:
					gameStatus = LOSE;
					break;
				case ACHIEVED:
					printBoard(board);
					gameStatus = WIN;
					break;
				}
			}
		}

		if (gameStatus == LOSE)
		{
			puts("\nGame Over !");
			printf("Retry - No (0) | Yes (1): ");
		}

		if (gameStatus == WIN)
		{
			puts("\nGame Over !");
			printf("Replay - No (0) | Yes (1): ");
		}

		if (gameStatus == LOSE || gameStatus == WIN)
		{
			int answer = 0;
			scanf("%d", &answer);

			if (answer == 0)
				gameStatus = END;

			if (answer == 1)
				gameStatus = INITIALISING;
		}

		if (gameStatus == END)
		{
			puts("\nBye :(");

			puts("Press any key to continue...");
			getch();
			exit(0);
		}
	}

	return 0;
}
