#include <iostream>        //Header File for input/output operations
#include <windows.h>       //Header file for the console control
#include <conio.h>         //Header file for reading user input
#include <string>          //Header file for using storing string
#include <vector>          //Header file for generation and storage of falling numbers
#include <thread>          //Header file for delay
#include <chrono>          //Header file for time calculations
using namespace std;

const int rows = 25;    //fixed number of rows in grid
const int cols = 80;    //fixed number of columns in grid

int robotRow = 18;  // Initial row position of the robot
int robotCol = 30;  // Initial column position of the robot
int score = 0;      // Player score
int timeLimit = 180; // Time limit for the game in seconds (3 minutes)

// Declare startTime outside main function
auto startTime = chrono::steady_clock::now() - chrono::seconds(1); // Start time (1 second offset)

// Function prototypes
void setColor(int color);
void drawGrid(CHAR_INFO buffer[]);
void moveRobot(char key);
void generateNumber(int difficulty);
void updateNumbers();
void drawNumbers(CHAR_INFO buffer[]);
void speed(int difficulty);
void swapBuffers();
void displayResult();

// Data structures for double buffering
CHAR_INFO buffer[rows * cols];
CHAR_INFO nextBuffer[rows * cols];

// Data structure for falling numbers
vector<pair<int, int>> fallingNumbers;  // Stores number positions (row, column)

// Main function
int main() {
    string firstName,lastName;
    int difficulty;  // Start with easy difficulty

    cout << "Enter your name: ";
    cin >> firstName >> lastName;
    cout << endl;

    // Loop until a valid difficulty level is entered
    while (true) {
        cout << "1) Easy\n2) Medium\n3) Hard\n\nSelect Difficulty Level (1-3): ";
        cin >> difficulty;

        // Check if the input is valid
        if (difficulty >= 1 && difficulty <= 3) {
            break;  // Exit the loop if a valid difficulty level is entered
        } else {
            cout << "Invalid input! Please enter a number between 1 and 3." << endl;
        }
    }

    char key;

    // Start the timer after the difficulty level is entered
    startTime = chrono::steady_clock::now();

    do {
        drawGrid(nextBuffer);  // Draw into next buffer

        if (_kbhit()) {
            key = _getch();   //inputs the key to move and pause 
            moveRobot(key);   
        }

        speed(difficulty);
        generateNumber(difficulty);
        updateNumbers();

        swapBuffers();  // Swap the buffers

        // Calculate elapsed time
        auto currentTime = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();

        // Check if time limit is exceeded
        if (elapsedTime >= timeLimit) {
            break;  // End the game loop
        }

    } while (key != 27 && key != 27);

    displayResult(); // Display final result when game ends
    setColor(0);

    return 0;
}

void drawGrid(CHAR_INFO buffer[]) {
    // Clear the buffer
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            buffer[i * cols + j].Char.AsciiChar = ' ';
            buffer[i * cols + j].Attributes = 0;
        }
    }

    // Draw the robot in nextBuffer
    const string robotArt[] = {
        "  \\__/0  ",  // Robot's head and basket
        "     \\|/  ", // Robot's arms
        "     / \\  "  // Robot's legs
    };

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < robotArt[i].size(); ++j) 
		{
            // Update the buffer with the robot part and attributes
            buffer[(robotRow + i) * cols + robotCol + j].Char.AsciiChar = robotArt[i][j];
            buffer[(robotRow + i) * cols + robotCol + j].Attributes = 560;  // Set color for the robot
        }
    }

    // Draw the grid with different colors
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Set background and foreground colors based on position
            if (i == 21 || i == 22 || i == 23) {
                // Pink section for rows 23, 24, and 25
                buffer[i * cols + j].Attributes = 200;
            }
            else if (i == 24) {
                // red color for row 25
                buffer[i * cols + j].Attributes = 8000;
            }
            else {
                // Default section with blue background and black foreground
                buffer[i * cols + j].Attributes = 560;
            }
        }
    }

    // Draw falling numbers in nextBuffer
    drawNumbers(buffer);

    // Display time remaining
    auto currentTime = chrono::steady_clock::now();
    auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
    int timeRemaining = timeLimit - elapsedTime;
    setColor(560);
    COORD timePosition = { 2, 2 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), timePosition);
    cout << "Time Remaining: " << max(0, timeRemaining) << "s";

    // Display score
    COORD scorePosition = { 60, 2 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), scorePosition);
    cout << "Score: " << score << "   ";
}

void moveRobot(char key)
{
    const int robotSpeed = 5;  // Adjust speed as needed

    // Update the robot's position based on the direction
    switch (key)
    {
    case 'A':
    case 'a':
    case 75:
        if (robotCol >= 5)
        {
            robotCol -= robotSpeed;
        }
        break;
    case 'D':
    case 'd':
    case 77:
        if (robotCol < cols - 7 - robotSpeed)
        {
            robotCol += robotSpeed;
        }
        break;
    case 'P':
    case 'p':
        if (key == 'P' || key == 'p')
        {
            _getch(); // Wait for any key press to continue
            // Redraw the grid after pause
        }
        break;


    }
}

// Function to generate a new number at a random column
void generateNumber(int difficulty)
{
    int maxFallingNumbers;

    switch (difficulty)
    {
    case 1:
        maxFallingNumbers = 5;  // Maximum number of falling numbers for easy difficulty
        break;
    case 2:
        maxFallingNumbers = 8;  // Maximum number of falling numbers for medium difficulty
        break;
    case 3:
        maxFallingNumbers = 12;  // Maximum number of falling numbers for hard difficulty
        break;
    }

    // Check if time limit is exceeded
    auto currentTime = chrono::steady_clock::now();
    auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
    if (elapsedTime >= timeLimit) {
        return;  // Stop generating numbers if time limit is exceeded
    }

    if (fallingNumbers.size() < maxFallingNumbers) {
        int col = rand() % (cols - 8) + 1;  // Avoid robot area
        fallingNumbers.push_back({ 0, col });  // Start at top row
    }
}

// Function to update positions of falling numbers and remove those reaching bottom
void updateNumbers() {
    // Check if time limit is exceeded
    auto currentTime = chrono::steady_clock::now();
    auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
    if (elapsedTime >= timeLimit) {
        return;  // Stop updating numbers if time limit is exceeded
    }

    for (int i = 0; i < fallingNumbers.size(); i++) {
        int& row = fallingNumbers[i].first;  // Get reference to row for direct modification
        row++;  // Update row position (downward movement)

        // Remove numbers reaching the 21st row
        if (row >= 21) {
            fallingNumbers.erase(fallingNumbers.begin() + i);
            i--;  // Adjust loop counter to avoid skipping elements after removal
        }

        // Check if the robot catches the number
        if (row == robotRow && fallingNumbers[i].second >= robotCol && fallingNumbers[i].second <= robotCol + 6) {
            // Extract the number from the falling number
            int num = fallingNumbers[i].second % 10;  // Get the ones digit

            // Check if the number is zero
            if (num == 0) {
                // Reset the score to zero
                score = 0;
            }
            else {
                // Update score
                score += num;
            }

            // Remove the caught number
            fallingNumbers.erase(fallingNumbers.begin() + i);
            i--;  // Adjust loop counter after removal
        }
    }
}

// Function to draw numbers at their current positions
void drawNumbers(CHAR_INFO buffer[]) {
    setColor(560);  // Blue background for numbers
    for (auto number : fallingNumbers) {
        buffer[number.first * cols + number.second].Char.AsciiChar = (char)(number.second % 10 + '0');  // Use stored number value, modulus by 10 to ensure single digits
        buffer[number.first * cols + number.second].Attributes = 560;
    }
}

        // Function to control the speed of falling numbers based on difficulty
void speed(int difficulty) {
    
    switch (difficulty) 
	{                    // Adjust speed based on difficulty level
    case 1:
        // Easy difficulty: slower speed
        this_thread::sleep_for(chrono::milliseconds(80));
        break;
    case 2:
        // Medium difficulty: medium speed
        this_thread::sleep_for(chrono::milliseconds(50));
        break;
    case 3:
        // Hard difficulty: faster speed
        this_thread::sleep_for(chrono::milliseconds(30));
        break;
    }
}

            // Function to display the final result after the game ends
void displayResult()
{
    setColor(7);
    system("cls");
    setColor(3);
    cout << "\n\n\n\n\n\n\n\n\t\tGame Over! Your final score: " << score << endl;
    cout << "\t\t   Thank you for playing!    " << endl;
}

            // Function to set console text color
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

            // Function to swap the current buffer with the next buffer
void swapBuffers() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD bufferSize = { cols, rows };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, cols - 1, rows - 1 };
    WriteConsoleOutput(hConsole, nextBuffer, bufferSize, bufferCoord, &writeRegion);

                // Swap the buffers
    swap(buffer, nextBuffer);
}
