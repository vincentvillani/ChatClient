/*
 * UIFunctions.cpp
 *
 *  Created on: 30 Jan 2016
 *      Author: vincent
 */

#include "UIFunctions.h"

#include <ncurses.h>
#include <sstream>
#include <math.h>
#include <algorithm>

#include "UIMessage.h"


//Private methods
static void calculateRowsUsedByUIMessage(ClientData* clientData, UIMessage* message)
{
	//+ 2 for ': ' (the colon and space), + 1 row for integer flooring
	message->UIRowsUsed = ((message->username.size() + message->message.size() + 2 ) / clientData->chatData.maxCol) + 1;
}

/*
static std::string getMessageRowFromRowOffset(ClientData* clientData, int rowOffset, int drawableRows)
{
	ChatData* cd = &clientData->chatData;
	//UIMessage* currentMessageObject = NULL;


	int currentRowTotal = 0;

	for(int i = cd->messageVector.size() - 1; i > 0; --i)
	{
		currentRowTotal += cd->messageVector[i]->UIRowsUsed;

		//This message contains the row we are looking for, get a pointer to the first char in the rowOffset
		if(currentRowTotal >= rowOffset)
		{
			int localRow = currentRowTotal - rowOffset;

			int usernameOffset = cd->messageVector[i]->username.size() + 2;

			if(localRow == 0)
			{

				std::stringstream ss;
				ss << cd->messageVector[i]->username << ": " << cd->messageVector[i]->message.substr(0, cd->maxCol - usernameOffset);

				return ss.str();
			}
			else
			{
				std::stringstream ss;
				ss << cd->messageVector[i]->message.substr(localRow * (cd->maxCol - usernameOffset), cd->maxCol - usernameOffset);

				return ss.str();
			}

		}
	}

	return std::string();

}
*/


//UI
//---------------------
void UISetup(ClientData* clientdata)
{
	//Setup ncurses
	initscr(); //init ncurses
	start_color(); //init colours

	init_pair(1, COLOR_GREEN, COLOR_BLACK); //Setup the colours
	attron(COLOR_PAIR(1)); //Turn the colour on

	scrollok(stdscr, TRUE); //Make the window scrollabled

	keypad(stdscr, true); //We get F!, F2, arrow keys etc
	noecho(); //Don't echo while we do a getch
	timeout(0); //Don't block when getting input

	//Get the max x and y for this screen
	//Is a macro so just pass by value
	getmaxyx(stdscr, clientdata->chatData.maxRow, clientdata->chatData.maxCol);


	//TODO: Debug, remove this
	std::stringstream ss;

	for(int i = 0; i < 50; ++i)
	{
		//ss << i <<  "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";

		ss << i << ": This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a stringThis is a string This is a string This is a string This is a string This is a string" << std::endl;

		UIMessage* newMessage = new UIMessage(std::string("Vincent"), ss.str());
		UIAddMessage(clientdata, newMessage);

		ss.str(""); //Clear the SS
	}

	clientdata->chatData.currentMessage = "Vincent:y8RkQ2m8PADX5H9\nqgxKy2twqCHGU1vCO0p3kmktJkpmQ43nkY8DeJ5mV8IgOCS4BHrjFp6EbU0npaO2yUnKu577hfYMcxZDSrDhMF44DbYspQIKrSShJ2BqSEUsEBEoNxDrUodZmwzqUH7CE4gvPuxNfKdWw hello";
			//"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";
}

void UIShutdown()
{
	endwin();
}


void UIUpdate(ClientData* clientdata)
{

	int key = getch();

	if(key == KEY_UP)
		clientdata->chatData.baseViewableRow -= 1;
	else if(key == KEY_DOWN)
		clientdata->chatData.baseViewableRow += 1;

	UIDraw(clientdata);
}



void UIAddMessage(ClientData* clientdata, UIMessage* newMessage)
{
	clientdata->chatData.messageVector.push_back(newMessage);
	calculateRowsUsedByUIMessage(clientdata, newMessage);


	//Redraw
	//UIDraw(clientdata);
}


void UIDraw(ClientData* clientdata)
{
	//Clear the window
	clear();

	ChatData* cd = &clientdata->chatData;

	int drawingRowsUsed = 0;

	//Draw the users current message first
	//---------------------------------------

	//How many rows do we need to display the whole message? (in screen space)
	int messageRowLength = (cd->currentMessage.size() / cd->maxCol) + 1; //+ 1 to account for the extra characters
	int messageStartRow = cd->maxRow - messageRowLength; //Calculate which row to start drawing the message on

	//We can't display the whole message
	if(messageStartRow < 0)
	{
		//Have to start from the top of the screen
		messageStartRow = 0;

		//We should show the end of the message and cut the top bit off
		int splitStringIndex =  cd->currentMessage.size() - (cd->maxCol * cd->maxRow);
		std::string currentMessageSubstring = cd->currentMessage.substr(splitStringIndex + 1, (cd->maxCol * cd->maxRow) - 1);

		drawingRowsUsed = cd->maxRow;

		move(messageStartRow, 0);
		printw(currentMessageSubstring.c_str());
	}
	else //Everything is fine, just draw the message here
	{
		move(messageStartRow, 0);
		printw(cd->currentMessage.c_str());

		drawingRowsUsed = messageRowLength;
	}

	//---------------------------------------



	//Try and print the current viewable messages, if there is space
	//---------------------------------------




	//Plus one more row for the space between the current message and the latest chat messages
	drawingRowsUsed += 1;


	//We don't have any more space to draw
	if(drawingRowsUsed >= cd->maxRow)
	{
		refresh();
		return;
	}

	/*

	move(cd->maxRow - drawingRowsUsed, 0);

	//int currentRowOffset = cd->baseViewableRow;
	//UIMessage* currentMessage = NULL;


	for(int i = cd->messageVector.size() - 1; i > 0; --i)
	{

		drawingRowsUsed += ((cd->messageVector[i]->message.size() + cd->messageVector[i]->username.size() + 2) / cd->maxCol) + 1;
		move(cd->maxRow - drawingRowsUsed, 0);

		drawingRowsUsed += 1; //Space between messages

		printw("%s: %s", cd->messageVector[i]->username.c_str(), cd->messageVector[i]->message.c_str());

		if(drawingRowsUsed >= cd->maxRow)
			break;
	}




	*/


	refresh(); //Actually draw it to the window




}




