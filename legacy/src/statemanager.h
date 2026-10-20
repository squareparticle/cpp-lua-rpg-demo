/*
Name: statemanager.h
Created: May 1 2002

Description:
	Setup the states of the program.
*/

const int STATE_BUSY            = -1; // There should be a state for when the computer is busy.  
                                      // During this time no input commands should be processed
                                      // Example: if the space key changes the state into message
                                      // mode and while in message mode the space key changes back
                                      // to playing mode.  Bad things will happen :(
const int STATE_NULL       = -2;  
int BUFFER_STATE = STATE_NULL;

const int STATE_READY           = 0;  // Shows that the game is not in a busy state
int PROCESSING_GAMESTATE =  STATE_READY; 
int PROCESSING_TTL_MAX          = 4; // Time given for changing states. 4 * 0.05 = 0.20 seconds
int UNLIMITED                   = -999; // The time to live = unlimited
int PROCESSING_TTL              = PROCESSING_TTL_MAX;

const int STATE_STARTMENU       = 1;  // Display the start screen
const int STATE_PLAYING         = 2;  // Actual playing mode (walking around the world)
const int STATE_MESSAGE         = 3;  // When a text message is spoken on the screen
const int STATE_SAVING          = 4;  // Save screen
const int STATE_LOADING         = 5;  // Load screen
const int STATE_CONSOLE         = 6;  // Drop down console for debugging
const int STATE_DROPDOWMMENU    = 7;  
const int STATE_OVERLAYMENU     = 8;  // Options menu that appears over the play area
const int STATE_TRANSITION      = 9;  // fading from one screen to the next
const int STATE_ANIMATION       = 10;  

int TRANSITION_STATE = 0;

int CURRENT_GAMESTATE = STATE_PLAYING;

const int STATE_DEBUGING_ON       = 1; // Display the start screen
const int STATE_DEBUGING_OFF      = 0; // Display the start screen
int DEBUGING_GAMESTATE = STATE_DEBUGING_OFF;

const int STATE_MESSAGEWINDOW_OPENING  = 0; // Animate the window opening
const int STATE_MESSAGEWINDOW_ON       = 1; // Display message window
const int STATE_MESSAGEWINDOW_CLOSING  = 2; // Animate the window closing
int MESSAGEWINDOW_GAMESTATE = STATE_MESSAGEWINDOW_OPENING;
int MESSAGEWINDOW_HIEGHT = 0;

const int DEBUGING_MODE_NONE = -1;
const int DEBUGING_MODE_MAPINFO = 0;
const int DEBUGING_MODE_HEROINFO = 1;
int DEBUGING_MODE = DEBUGING_MODE_NONE;

const int NO_WEATHER = -1;
const int WEATHER_RAIN = 0;
const int WEATHER_SNOW = 1;
const int WEATHER_PARTICLE = 2;
const int WEATHER_FERRY = 3;

int CURRENT_WEATHERSTATE = NO_WEATHER;

void changeState(int newState);
void bufferState(int oldState);
void changeWeatherState(int newState);
void setBusyState(int PROCESSING_TTL_MAC);

void bufferState(int oldState){
   BUFFER_STATE = oldState;
}

void changeState(int newState){
   PROCESSING_GAMESTATE = STATE_BUSY;

   if (BUFFER_STATE == STATE_NULL){
      CURRENT_GAMESTATE = newState;
   }
   else{
      CURRENT_GAMESTATE = BUFFER_STATE;
	  BUFFER_STATE = STATE_NULL;
   }
}

void changeState(int newState, int bufferedState){
   PROCESSING_GAMESTATE = STATE_BUSY;

   CURRENT_GAMESTATE = newState;
   BUFFER_STATE = bufferedState;
}

void changeWeatherState(int newState){
   CURRENT_WEATHERSTATE = newState;
   switch(CURRENT_WEATHERSTATE){
  	    case NO_WEATHER:
		break;
  	    case WEATHER_RAIN:
           init_RainSprites();
		break;
  	    case WEATHER_SNOW:
           init_SnowSprites();
		break;
  	    case WEATHER_PARTICLE:
           init_Particles();
		break;
		case WEATHER_FERRY:
           init_Ferries();
		break;
   }
}


void setBusyState(int PROCESSING_TTL_MAC){
   PROCESSING_GAMESTATE = STATE_BUSY;
   PROCESSING_TTL = PROCESSING_TTL_MAC;
}