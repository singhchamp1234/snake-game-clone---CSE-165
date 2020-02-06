#include <iostream>
#include "App.h"
# include <cmath>

App* singleton = NULL;

void timer(int id) {
	// This will get called every 500 milliseconds after
	// you call it once

	// If you want to manipulate the app in here
	// do it through the singleton pointer
	
	// this is to redraw
	glutPostRedisplay();
	
	// everytime, we reduce 2 milliseconds the timerSpeed
	singleton->timerSpeed -= 2;
	
	// but... not less than 200 milliseconds
	if (singleton->timerSpeed <= 200)
		singleton->timerSpeed = 200;
	
	// call again the timer with the new elapsed time
	glutTimerFunc(singleton->timerSpeed, timer, id);
}

// constructor
App::App(int argc, char** argv, int width, int height, const char* title) : GlutApp(argc, argv, width, height, title) {
	
	// initializing the singleton
	singleton = this;
	
	// init game
	initGame();
	
	// highscore starts with zero
	highScore = 0;

	// creating 2 bitmaps
	menu = new TexRect("GameHome.png", -1, 1, 2, 2);
	help = new TexRect("help.png", -1, 1, 2, 2);
	//grid = new TexRect("", -1, 1, 2, 2);
	
	// cell size is the width divided by number of cells +1
	cell_size_x = float(w) / (MSIZE+1);
	cell_size_y = float(h) / (MSIZE+1);

	// game over is false
	gameOver = false;
	
	// we can change the points for each apple here
	appleValue = 10;

	// we can change the points for each snake movement here
	posValue = 1;
	
	// first timer call (0 means immediatly)
	timer(0);
}

// initialize map
void App::initGame() {

	update = false;
	
	//copy map_init into map
	for (int i = 0; i < MSIZE; i++)
		strcpy(map[i], map_ini[i]);
	
	// times speed starts with 500 milliseconds
	timerSpeed = 500; //we slowly derease to a refresh rate of 200 MS as seen earlier
	
	// mode menu (0)
	mode = 0;
	
	// we clear all the snake positions
	snake.clear();
	
	// but, we add only one snake position in the middle of the map
	snake.push_back(snakePos((MSIZE - 1) / 2, (MSIZE - 1) / 2)); //create the head as the same size as one single area. any bigger and we are bigger
	//than ourself and we will lose immediately. Changing the amount in which MSIZE is subtracted by will create new, unwanted instances on the board.

	// about the middle of the map; mark the cell as "snake head = 'S'"
	map[(MSIZE - 1) / 2][(MSIZE - 1) / 2] = 'S';
	
	// 4 possible directions, random
	dir = rand() % 4; //starts off with a random direction.

	// counting free positions
	freePositions = 0;
	for (int x = 0; x < MSIZE; x++) {

		for (int y = 0; y < MSIZE; y++) {

			if (map[x][y] == ' ') {	// wall

				freePositions++;

			}
		}

	}
	// creating an apple in random free position
	createApple();
	
	// the user starts with zero points
	score = 0;
	
	// game over is false
	gameOver = false;
}

// drawing a circle as polygon
void App::drawCircle(int lineAmount, float x, float y, float r) {
	// 2pi
	float twicePi = 2.0f * 3.15169f;
	
	// drawing a polygon with the vertices of a circle (lineAmount vertices)
	glBegin(GL_POLYGON);
	for (int i = 0; i <= lineAmount; i++) {

		glVertex3f(x+r*cos(i *  twicePi / lineAmount), y+r*sin(i * twicePi / lineAmount), 0.0f);

	}

	glEnd();

}

// draw score, and high score; algo game over if applies
void App::drawStatus(){

	char string[40];
	// score message
	sprintf(string, "Score = %d", score);

	int len, i; //len = our string length and i is our counter variable

	glColor3f(1.0, 1.0, 1.0);
	
	// normalized start position of the message 
	glRasterPos2f(-0.7, 0.7);
	len = (int)strlen(string);
	
	// drawing the message
	for (i = 0; i<len; i++)	{

		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);

	}

	// high score message
	sprintf(string, "Highest = %d", highScore);
	glRasterPos2f(-0.7, 0.9);
	len = (int)strlen(string);
	for (i = 0; i<len; i++) {

		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);

	}

	// game over message, if applies
	if (gameOver) {

		sprintf(string, "GAME OVER!");
		glRasterPos2f(-0.4, -0.2);
		len = (int)strlen(string);
		for (i = 0; i < len; i++) {

			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);

		}

		sprintf(string, "Click to play again!");
		glRasterPos2f(-0.4, -0.3);
		len = (int)strlen(string);
		for (i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);

		}

	}

}

// draw 
void App::draw() {

	if (mode == 0) {	// menu

		menu->draw(0);

	}

	else if (mode == 2) {	// help

		help->draw(0);

	}

	else if (mode == 1) {	// if play is true
		// print the head of the snake

		//grid->draw(0);

		snakePos p(*snake.rbegin());

		if (!gameOver && update) { //if the game is not over and update is true. update is initialized as false but becomes true when mode = 1, AKA when the game begins
			// each new piece of the snake is of 's', which we have declared to be a small box with a different color.
			map[p.x][p.y] = 's';
			
			// 4 possible directions
			switch (dir) {

				case 0: p.x--; break; //we move left which is negative on this plain
				case 1: p.x++; break; //we move right which is positive on this plain
				case 2: p.y--; break; //we move down which is negative on this plain
				case 3: p.y++; break; //we move up which is negative on this plain   break may not be necessary

			}

			// checking what we have in the map, at the new snake head position
			if (map[p.x][p.y] == ' ') {  // empty
			
			// we remove the tail
				snakePos tail = *snake.begin();
				map[tail.x][tail.y] = ' ';
				snake.pop_front(); //by poping the front of the snake, we make sure that the snake keeps its appropriate length after each update.
				//without this, our snake will grow endlessly even if we dont eat any apples. 
				// increasing score by 1 for each movement.
				score += posValue;
			}

			else if (map[p.x][p.y] == 'X') {	// apple
				map[p.x][p.y] = 'S'; 
				// reduce one position
				freePositions--;
				// generate a new random apple pos
				createApple();
				score += appleValue;
			}

			else // if we hit a wall or ourselves
				gameOver = true;

				// add one position in the snake (the head)
				snake.push_back(p);
				map[p.x][p.y] = 'S';	// capital letter

			if (score > highScore)	// update high score if applicable
				highScore = score;

		}

		update = true;
		drawStatus();

		// displaying the map
		for (int x = 0; x < MSIZE; x++)
		
			for (int y = 0; y < MSIZE; y++) {

				if (map[x][y] == '*')	{ // if the 2D array is == to *, which we have displayed in App.h, we call rect to print a block given the parameters 

					Rect(
					x0 + x*cell_size_x,	// transform x,y into window position
					y0 + y*cell_size_y,
					cell_size_x,
					cell_size_y,
					0.9f, 0.7f, 0.2f).draw(); //calling rect and defining our x, y, width, height, and color varients, 0.9f, 0.7f, 0.2f

			}

			else if (map[x][y] == 'X')	{ // if the game has marked a location as X, AKA an apple, it will not print a block there, but rather draw the apple.

				glColor3f(1, 0, 0);	// red color
				drawCircle(30,								//total lines
					x0 + x*cell_size_x + cell_size_x * 0.5f,	//x position
					y0 + y*cell_size_y - cell_size_y * 0.5f,	//y position
					(cell_size_x + cell_size_y) / 4.0);			//radius
			}

			else if (map[x][y] == 's') { // snake tail

				Rect(
					x0 + x*cell_size_x,
					y0 + y*cell_size_y,
					cell_size_x,
					cell_size_y,
					0.6f, 0.7f, 0.4f).draw(); //0.9f, 0.3f, 0.4f

			}
			else if (map[x][y] == 'S') { // snake head
				Rect(
					x0 + x*cell_size_x,
					y0 + y*cell_size_y,
					cell_size_x,
					cell_size_y,
					0.75f, 0.5f, 0.6f).draw();	// 1.0f, 0.7f, 0.8f
			}
		}
		
	}
}


// called when mouse click
void App::leftMouseDown(float x, float y) {
	if (mode == 0) { //if at main menu...
		if (x >= -0.94f && y >= 0.52f && x <= -0.35f && y <= 0.92f) {
			// inside play button
			mode = 1;
		}
		else if (x >= -0.31f && y >= 0.52f && x <= 0.29f && y <= 0.93f) {
			// inside help button
			mode = 2;
		}
		else if (x >= 0.33f && y >= 0.52f && x <= 0.92f && y <= 0.93f) {
			// inside exit button
			exit(0);
		}
	}
	else if (gameOver || mode == 2) {	// mode help or game over
		mode = 0;	// back to menu mode again
		initGame();
	}

}

void App::leftMouseUp(float x, float y) {
}


void App::keyDown(unsigned char key, float x, float y) {

    if (key == 27){	// pressed esc

		if (mode == 1 || mode == 2) {	// play or help mode
				mode = 0;	// back to menu mode
				initGame();
		}
		else
			exit(0);
    }
	else if (gameOver || mode==2) {
		mode = 0;	// back to menu mode
		initGame();
	}

}

void App::specialKeyDown(int key, float x, float y) {

	switch (key) {

		case  0x64: dir = 0; break;	// left key
		case  0x66: dir = 1; break;	// right key
		case  0x67: dir = 2; break;	// buttom
		case  0x65: dir = 3; break;	// top

	}

}


App::~App(){
    std::cout << "Exiting..." << std::endl;
}


void App::createApple() {
	// generate at a random free position
	int pos = rand() % freePositions; //using the rand() function, int pos becomes a random number from 0 to (freePositions -1) every time this is called
	
	// take the pos-th free position in the map
	for (int x = 0; x < MSIZE; x++) for (int y = 0; y < MSIZE; y++) {

		if (map[x][y] == ' ') {

			pos--;

			if (pos == 0) {

				map[x][y] = 'X';	// mark it as an apple

				return;

			}
		}
	}
}
