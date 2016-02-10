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

#define USERNAME_SPACER_LENGTH 2


//Private methods
static void calculateRowsUsedByUIMessage(ClientData* clientData, UIMessage* message)
{
	//+ 2 for ': ' (the colon and space), + 1 row for integer flooring
	message->UIRowsUsed = ((message->username.size() + message->message.size() + 2 ) / clientData->chatData.maxCol) + 1;

	//Update the total rows used
	clientData->chatData.totalMessageRows += message->UIRowsUsed;
}




//UI
//---------------------
void UISetup(ClientData* clientdata)
{
	//Setup ncurses
	initscr(); //init ncurses
	start_color(); //init colours

	init_pair(1, COLOR_CYAN, COLOR_BLACK); //Setup the colours
	attron(COLOR_PAIR(1)); //Turn the colour on

	scrollok(stdscr, TRUE); //Make the window scrollabled

	keypad(stdscr, true); //We get F!, F2, arrow keys etc
	noecho(); //Don't echo while we do a getch
	timeout(0); //Don't block when getting input

	//Get the max x and y for this screen
	//Is a macro so just pass by value
	getmaxyx(stdscr, clientdata->chatData.maxRow, clientdata->chatData.maxCol);

	move(clientdata->chatData.maxRow - 1, clientdata->chatData.maxCol - 1);


	/*
	//TODO: Debug, remove this
	std::stringstream ss;

	for(int i = 0; i < 10; ++i)
	{
		//ss << i <<  "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";

		ss << i << ": This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string This is a string";

		UIMessage* newMessage = new UIMessage(std::string("Vincent"), ss.str());
		UIAddMessage(clientdata, newMessage);

		ss.str(""); //Clear the SS
	}
	*/

	//clientdata->chatData.currentMessage = "$|";
	//clientdata->chatData.currentMessage = "Vincent:y8RkQ2m8PADX5H9qgxKy2twqCHGU1vCO0p3kmktJkpmQ43nkY8DeJ5mV8IgOCS4BHrjFp6EbU0npaO2yUnKu577hfYMcxZDSrDhMF44DbYspQIKrSShJ2BqSEUsEBEoNxDrUodZmwzqUH7CE4gvPuxNfKdWw hello";
			//"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";
}

void UIShutdown()
{
	endwin();
}


void UIUpdate(ClientData* clientdata, MasterMailbox* mailbox)
{

	int key = getch();

	if(key == -1)
		return;
	else if(key == KEY_UP)
	{
		clientdata->chatData.baseViewableRow += 1;

		if(clientdata->chatData.baseViewableRow >= clientdata->chatData.totalMessageRows)
			clientdata->chatData.baseViewableRow = clientdata->chatData.totalMessageRows - 1;

		UIDraw(clientdata);
	}
	else if(key == KEY_DOWN)
	{
		if(clientdata->chatData.baseViewableRow == 0)
			return;

		clientdata->chatData.baseViewableRow -= 1;
		UIDraw(clientdata);

	}
	else if(key == KEY_ENTER || key == '\n')
	{
		//Add the current message to the local queue
		UIMessage* messageData = new UIMessage(clientdata->username, clientdata->chatData.currentMessage);
		UIAddMessage(clientdata, messageData);

		//Notify the network thread that we want to send a message
		mailbox->ClientThreadSendChatMessage(clientdata->chatData.currentMessage);
		clientdata->chatData.currentMessage.clear(); //Clear the string

		//Redraw
		UIDraw(clientdata);
	}
	else if(key == KEY_BACKSPACE) //|| key == KEY_DL) //This is a 'current message' character
	{
		if(clientdata->chatData.currentMessage.size())
		{
			//Delete the last character in the string
			clientdata->chatData.currentMessage.erase(clientdata->chatData.currentMessage.end() - 1);
			UIDraw(clientdata);
		}
	}
	else
	{
		clientdata->chatData.currentMessage += key;
		UIDraw(clientdata);
	}



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
	//const std::string currentMessagePrefix = "$|";

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
	drawingRowsUsed += 2;


	//We don't have any more space to draw, or there are no messages to print
	if(drawingRowsUsed >= cd->maxRow)
	{
		//Move the curser to the char past the last one in the current string message
		move(cd->maxRow - 1, (cd->currentMessage.size() % cd->maxCol));
		refresh();
		return;
	}



	move(cd->maxRow - drawingRowsUsed, 0);

	UIMessage* currentMessage = NULL;
	int currentMessageIndex = cd->messageVector.size() - 1;
	uint32_t totalRowCount = 0;
	int localRow = -1; //Row number relative to the upper most row of the current message

	//Find the first viewable message and which of it's rows is the base row
	for(; currentMessageIndex >= 0; --currentMessageIndex)
	{
		UIMessage* tempMessage = cd->messageVector[currentMessageIndex];

		totalRowCount += tempMessage->UIRowsUsed;

		//This is the current message
		if(totalRowCount > cd->baseViewableRow)
		{
			//Set currentMessage and draw the baseviewablerow
			currentMessage = tempMessage;

			//- 1 because the row counts start at zero
			localRow = (totalRowCount - 1) - cd->baseViewableRow;


			int rowCharLength = cd->maxCol - currentMessage->username.size() - USERNAME_SPACER_LENGTH;

			if(localRow == 0)
			{
				//Username length + USERNAME_SPACER_LENGTH
				printw("%s| %s", currentMessage->username.c_str(), currentMessage->message.substr(0, std::min<int>(rowCharLength, currentMessage->message.size() )).c_str());
			}
			else
			{
				int offsetLength = localRow * rowCharLength;
				move(cd->maxRow - drawingRowsUsed, cd->maxCol - rowCharLength);
				printw("%s", currentMessage->message.substr(offsetLength, std::min<int>(rowCharLength, currentMessage->message.size() - offsetLength)).c_str());

			}

			drawingRowsUsed += 1;

			//We are done here
			break;
		}
	}

	//We can't find the most recent message
	if(currentMessage == NULL)
	{
		//fprintf(stdout, "We can't find the current message!\n");

		//Move the curser to the char past the last one in the current string message
		move(cd->maxRow - 1, (cd->currentMessage.size() % cd->maxCol));
		refresh();
		return;
	}



	//Draw the remaining rows
	while(drawingRowsUsed < cd->maxRow)
	{
		localRow -= 1;



		//We have run out of rows in the current message
		if(localRow < 0)
		{
			currentMessageIndex -= 1;

			//For the space between different messages
			drawingRowsUsed += 1;

			//We have run out of messages
			if(currentMessageIndex < 0)
				break;

			currentMessage = cd->messageVector[currentMessageIndex]; //Get the next message
			localRow = currentMessage->UIRowsUsed - 1; //local rows are zero indexed
		}


		int rowCharLength = cd->maxCol - currentMessage->username.size() - USERNAME_SPACER_LENGTH;

		if(localRow == 0) //We need to print the username along with the start of the message
		{
			move(cd->maxRow - drawingRowsUsed, 0);
			printw("%s| %s", currentMessage->username.c_str(), currentMessage->message.substr(0, std::min<int>(rowCharLength, currentMessage->message.size() )).c_str());
		}
		else //Just print the current row
		{
			int offsetLength = localRow * rowCharLength;
			move(cd->maxRow - drawingRowsUsed, cd->maxCol - rowCharLength);
			printw("%s", currentMessage->message.substr(offsetLength, std::min<int>(rowCharLength, currentMessage->message.size() - offsetLength)).c_str());
		}

		drawingRowsUsed += 1;

	}

	//Move the curser to the char past the last one in the current string message
	move(cd->maxRow - 1, (cd->currentMessage.size() % cd->maxCol));


	refresh(); //Actually draw it to the window

}




