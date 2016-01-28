/*
 * ClientThreadFunctions.cpp
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */


#include "ClientThreadFunctions.h"

#include "Network.h"
#include "MasterMailbox.h"

#include <string.h>
#include <chrono>
#include <functional>

#include <ncurses.h>


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


	/*
	printw("I am a hacker"); //write to a buffer
	refresh(); //render it to the screen
	getch(); //wait for use input
	endwin(); //End curses mode
	*/


	while(true)
	{
		std::unique_lock<std::mutex> workQueueLock(clientData->mutex);

		bool workToDo = clientData->conditionVariable.wait_for(workQueueLock, std::chrono::milliseconds(10), [&]{return clientData->workQueue.size();});

		if(workToDo)
		{
			workQueueLock.unlock();
			DoAllWork(clientData, mailbox);
		}

	}


}



void ClientHandleUsernameChanged(ClientData* clientData)
{
	printf("Client thread: Username sent to server!\n");
}
