/*
 * ClientThreadFunctions.cpp
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */


#include "ClientThreadFunctions.h"

#include "Network.h"
#include "MasterMailbox.h"
#include "UIFunctions.h"

#include <string.h>
#include <chrono>
#include <functional>


#include <string.h>


static void DoAllWork(ClientData* clientData, MasterMailbox* mailbox)
{
	std::unique_lock<std::mutex> workQueueLock(clientData->mutex);

	while(clientData->workQueue.size())
	{
		std::function<void()> workItem = clientData->workQueue.front();
		clientData->workQueue.pop();

		//release the lock
		workQueueLock.unlock();

		//Do the work
		workItem();

		//Lock before checking the condition again
		workQueueLock.lock();
	}
}


void ClientThreadMain(ClientData* clientData, MasterMailbox* mailbox)
{

	UISetup(clientData);

	UIDraw(clientData);

	//while(1);


	while(true)
	{
		std::unique_lock<std::mutex> workQueueLock(clientData->mutex);

		bool workToDo = clientData->conditionVariable.wait_for(workQueueLock, std::chrono::milliseconds(10), [&]{return clientData->workQueue.size();});

		if(workToDo)
		{
			workQueueLock.unlock();
			DoAllWork(clientData, mailbox);
		}

		//Check for user input
		UIUpdate(clientData, mailbox);
	}



	UIShutdown();

}



void ClientHandleUsernameChanged(ClientData* clientData)
{
	printf("Client thread: Username sent to server!\n");
}


/*
	initscr(); //init ncurses
	start_color(); //init colours

	//init_color(COLOR_RED, 934, 602, 535);
	init_pair(1, COLOR_GREEN, COLOR_BLACK);



	attron(COLOR_PAIR(1));

	getch()

	//if(can_change_color())
	printw("Wow this is a colour pair, wowowowow");

	refresh();
	getch();
	endwin(); //end ncurses

	*/

	/*
	char* message = "Enter a string: ";
	char str[80];
	int row, col;

	initscr();
	raw();
	getmaxyx(stdscr, row, col);
	attron(A_BOLD);

	mvprintw(row / 2, (col - strlen(message)) / 2, "%s", message); //print the message at the center of the screen

	getstr(str);
	mvprintw(LINES - 2, 0, "You entered: %s", str);
	getch();
	endwin();
	*/

	/*
	int ch;
	int rowNum, columnNum;

	initscr(); //Start curses mode
	getmaxyx(stdscr, rowNum, columnNum); //This is a macro to get the number of rows and columns in a window, pass it ints by value because its a macro
	raw(); //Line buffering disabled
	keypad(stdscr, true); //We get F!, F2, arrow keys etc
	noecho(); //Don't echo while we do a getch

	printw("Number of rows and columns is: (%d,%d)\n", rowNum, columnNum);

	ch = getch(); //If raw() hadn't been called we have to press enter before it gets to the program



	refresh(); //render everything in stdscr buffer to the screen
	getch(); //wait for user input
	endwin(); //exit curses mode
	 */

	/*
	printw("I am a hacker"); //write to a buffer
	refresh(); //render it to the screen
	getch(); //wait for use input
	endwin(); //End curses mode
	*/
