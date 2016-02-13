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
#include <signal.h>

#include "UIMessage.h"

#define USERNAME_SPACER_LENGTH 2

#define CHAT_MESSAGES_COLOUR_PAIR 1
#define CURRENT_MESSAGE_COLOUR_PAIR 2


//Private methods
static void CalculateRowsUsedByUIMessage(ClientData* clientData, UIMessage* message)
{
	//+ 2 for ': ' (the colon and space), + 1 row for integer flooring
	message->UIRowsUsed = ((message->username.size() + message->message.size() + 2 ) / clientData->chatData.maxCol) + 1;

	//Update the total rows used
	clientData->chatData.totalMessageRows += message->UIRowsUsed;
}

static void ResetAndRecalculateRowsUsedByAllUIMessages(ClientData* clientData)
{
	//Reset total message rows
	clientData->chatData.totalMessageRows = 0;

	for(uint32_t i = 0; i < clientData->chatData.messageVector.size(); ++i)
	{
		//Recalculate the number of rows used for each message
		CalculateRowsUsedByUIMessage(clientData, clientData->chatData.messageVector[i]);
	}
}



//UI
//---------------------
void UISetup(ClientData* clientdata, MasterMailbox* mailbox)
{
	//Setup ncurses
	initscr(); //init ncurses
	start_color(); //init colours

	//chat messages colour
	init_pair(CHAT_MESSAGES_COLOUR_PAIR, COLOR_GREEN, COLOR_BLACK); //Setup the colours

	init_pair(CURRENT_MESSAGE_COLOUR_PAIR, COLOR_WHITE, COLOR_BLACK);

	//attron(COLOR_PAIR(CHAT_MESSAGES_COLOUR_PAIR)); //Turn the colour on

	scrollok(stdscr, TRUE); //Make the window scrollabled

	keypad(stdscr, true); //We get F!, F2, arrow keys etc
	noecho(); //Don't echo while we do a getch
	timeout(0); //Don't block when getting input

	//Get the max x and y for this screen
	//Is a macro so just pass by value
	getmaxyx(stdscr, clientdata->chatData.maxRow, clientdata->chatData.maxCol);

	move(clientdata->chatData.maxRow - 1, clientdata->chatData.maxCol - 1);

	/*
	std::function<void(int)> functor = std::bind(TerminalResizeHandler, mailbox, std::placeholders::_1);

	//register the handler for a terminal resize signal
	signal(SIGWINCH, functor.target<void(int)>());
	*/
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
	else if(key == KEY_BACKSPACE || key == KEY_DC || key == KEY_DL || key == 127) //This is a 'current message' character
	{
		if(clientdata->chatData.currentMessage.size())
		{
			//Delete the last character in the string
			clientdata->chatData.currentMessage.erase(clientdata->chatData.currentMessage.end() - 1);
			UIDraw(clientdata);
		}
	}
	else if(key == KEY_RESIZE)
	{
		UIResize(clientdata);
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
	CalculateRowsUsedByUIMessage(clientdata, newMessage);


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


	//Change the colour pair to the current message colour
	attron(COLOR_PAIR(CURRENT_MESSAGE_COLOUR_PAIR));

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


	//Change to the chat message colour pair
	attron(COLOR_PAIR(CHAT_MESSAGES_COLOUR_PAIR));

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
			//drawingRowsUsed += 1;

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


void UIResize(ClientData* clientdata)
{
	clear();

	//Get the max x and y for this screen
	//Is a macro so just pass by value
	getmaxyx(stdscr, clientdata->chatData.maxRow, clientdata->chatData.maxCol);

	//Recalculate how many rows are needed to display each message
	ResetAndRecalculateRowsUsedByAllUIMessages(clientdata);

	//Draw the current content
	UIDraw(clientdata);

}





