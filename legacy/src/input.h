/*
Name: input.h
Created: Jan 5 2002

Description:

	This header controls all the input devices
		* Keyboard
		* game port
		* mouse

    Depending on the current state of the game, the key commands change. This modual is the 
	controller for the different key commands and there states.  Same goes for the game port and
	mouse.
	
	Note: Escape is always used for quit and is handled at a higher level
*/

void KeyBoardController();
void OverRideKeyBoardController();
void nextHeroFrame();
void InputModePlaying();
void InputModeConsole();
void InputModeOverLay();
void GamePortController();
void InputModePlaying_JoyPad();
void InputModeOverLay_JoyPad();

BOOL ObjectCollisionCheck(int direction);
char getKeyPressed_DI();
char getKeyPressed ();

// check for input across all devices
void InputController(){
   KeyBoardController();
   if(f_hasGamePort)
   GamePortController();
   //MouseController();
}

// check for input from the keyboard and execute the correct command according to the current state
void KeyBoardController(){
	switch(CURRENT_GAMESTATE){
  	    case STATE_PLAYING:
		   InputModePlaying();
		break;
  	    case STATE_MESSAGE:
		   InputModeMessage();
		break;
  	    case STATE_CONSOLE:
		   InputModeConsole();
		break;
		case STATE_OVERLAYMENU:
 	       InputModeOverLay();
		break;
    	case STATE_ANIMATION:
			DoNextFrame();
		break;
	}
}

void GamePortController(){
	switch(CURRENT_GAMESTATE){
  	    case STATE_PLAYING:
		   InputModePlaying_JoyPad();
		break;
  	    case STATE_MESSAGE:
		   InputModeMessage_JoyPad();
		break;
		case STATE_OVERLAYMENU:
 	       InputModeOverLay_JoyPad();
		break;
    	case STATE_ANIMATION:
		break;
	}
}

void OverRideKeyBoardController(){
   if (KEY_DOWN(VK_ESCAPE))
      PostMessage(main_window_handle, WM_DESTROY,0,0);
}

void InputModePlaying_JoyPad(){
   // get joystick data
   lpdijoy2->Poll(); // this is needed for joysticks only
   lpdijoy2->GetDeviceState(sizeof(DIJOYSTATE), (LPVOID)&joy_state);

   if (joy_state.lY > 10){
	  if(ObjectCollisionCheck(Jcompas.SOUTH)){
         if(((MAP_CAMERAY + SCREEN_HEIGHT) < (MAP_HEIGHT-MAP_LAYER_SPEEDY)) && ((heroStruct.map.y-MAP_CAMERAY) > ((SCREEN_HEIGHT/2)-(heroStruct.height/2))))
            MAP_CAMERAY = MAP_CAMERAY + MAP_LAYER_SPEEDY;
	      heroStruct.map.y = heroStruct.map.y + heroStruct.speed;
	  }
  	  heroStruct.direction = Jcompas.SOUTH;
	  nextHeroFrame();
   }

   if (joy_state.lY < -10){
	  if(ObjectCollisionCheck(Jcompas.NORTH)){
         if((MAP_CAMERAY >= MAP_LAYER_SPEEDY) && (heroStruct.map.y < (MAP_HEIGHT - (SCREEN_HEIGHT/2)-(heroStruct.height/2))))
            MAP_CAMERAY = MAP_CAMERAY - MAP_LAYER_SPEEDY;
	     heroStruct.map.y = heroStruct.map.y - heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.NORTH;
	  nextHeroFrame();
   }

   if (joy_state.lX < -10){
	  if(ObjectCollisionCheck(Jcompas.WEST)){
         if((MAP_CAMERAX >= MAP_LAYER_SPEEDX) && (heroStruct.map.x < (MAP_WIDTH - (SCREEN_WIDTH/2)-(heroStruct.width/2))))
            MAP_CAMERAX = MAP_CAMERAX - MAP_LAYER_SPEEDX;
	     heroStruct.map.x = heroStruct.map.x - heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.WEST;
	  nextHeroFrame();
   }

   if (joy_state.lX > 10){
	  if(ObjectCollisionCheck(Jcompas.EAST)){
         if(((MAP_CAMERAX + SCREEN_WIDTH) < (MAP_WIDTH-MAP_LAYER_SPEEDX)) && ((heroStruct.map.x-MAP_CAMERAX) > ((SCREEN_WIDTH/2)-(heroStruct.width/2))))
            MAP_CAMERAX = MAP_CAMERAX + MAP_LAYER_SPEEDX;
	     heroStruct.map.x = heroStruct.map.x + heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.EAST;
	  nextHeroFrame();
   }
   if (joy_state.rgbButtons[0] > 100){
	   changeState(STATE_MESSAGE);
       f_needScreenShot = true;
   }
   if (joy_state.rgbButtons[1] > 100){
	   newspeed ++;
   }
   if (joy_state.rgbButtons[2] > 100){
	   newspeed --;
   }
}


void InputModePlaying(){
   if (KEY_DOWN(VK_DOWN)){
	  if(ObjectCollisionCheck(Jcompas.SOUTH)){
         if(((MAP_CAMERAY + SCREEN_HEIGHT) < (MAP_HEIGHT-MAP_LAYER_SPEEDY)) && ((heroStruct.map.y-MAP_CAMERAY) > ((SCREEN_HEIGHT/2)-(heroStruct.height/2))))
            MAP_CAMERAY = MAP_CAMERAY + MAP_LAYER_SPEEDY;
	      heroStruct.map.y = heroStruct.map.y + heroStruct.speed;
	  }
  	  heroStruct.direction = Jcompas.SOUTH;
	  nextHeroFrame();
   }

   if (KEY_DOWN(VK_UP)){
	  if(ObjectCollisionCheck(Jcompas.NORTH)){
         if((MAP_CAMERAY >= MAP_LAYER_SPEEDY) && (heroStruct.map.y < (MAP_HEIGHT - (SCREEN_HEIGHT/2)-(heroStruct.height/2))))
            MAP_CAMERAY = MAP_CAMERAY - MAP_LAYER_SPEEDY;
	     heroStruct.map.y = heroStruct.map.y - heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.NORTH;
	  nextHeroFrame();
   }

   if (KEY_DOWN(VK_LEFT)){
	  if(ObjectCollisionCheck(Jcompas.WEST)){
         if((MAP_CAMERAX >= MAP_LAYER_SPEEDX) && (heroStruct.map.x < (MAP_WIDTH - (SCREEN_WIDTH/2)-(heroStruct.width/2))))
            MAP_CAMERAX = MAP_CAMERAX - MAP_LAYER_SPEEDX;
	     heroStruct.map.x = heroStruct.map.x - heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.WEST;
	  nextHeroFrame();
   }

   if (KEY_DOWN(VK_RIGHT)){
	  if(ObjectCollisionCheck(Jcompas.EAST)){
         if(((MAP_CAMERAX + SCREEN_WIDTH) < (MAP_WIDTH-MAP_LAYER_SPEEDX)) && ((heroStruct.map.x-MAP_CAMERAX) > ((SCREEN_WIDTH/2)-(heroStruct.width/2))))
            MAP_CAMERAX = MAP_CAMERAX + MAP_LAYER_SPEEDX;
	     heroStruct.map.x = heroStruct.map.x + heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.EAST;
	  nextHeroFrame();
   }
   if (KEY_DOWN(VK_SPACE)){
	   changeState(STATE_MESSAGE);
       f_needScreenShot = true;
   }
   if (KEY_DOWN(VK_F12) && (DEBUGING_GAMESTATE == STATE_DEBUGING_ON)){
	      changeState(STATE_CONSOLE);
       f_needScreenShot = true;
   }
   if (KEY_DOWN(VK_F1)){
	   changeState(STATE_OVERLAYMENU);
       f_needScreenShot = true;
   }
   if (KEY_DOWN(VK_F2)){
	   for(int tt =0; tt< 20000; tt++)
      	   DatagramClient("192.168.1.100", 80,"hello world");
   }
}

void InputModePlayingOverRide(int Direction){
   //sprintf(strMsgDebug,"%d",Direction);

   if (Direction == Jcompas.SOUTH){
	  if(ObjectCollisionCheck(Jcompas.SOUTH)){
  //       if(((MAP_CAMERAY + SCREEN_HEIGHT) < (MAP_HEIGHT-MAP_LAYER_SPEEDY)) && ((heroStruct.map.y-MAP_CAMERAY) > ((SCREEN_HEIGHT/2)-(heroStruct.height/2))))
  //          MAP_CAMERAY = MAP_CAMERAY + MAP_LAYER_SPEEDY;
	      heroStruct.map.y = heroStruct.map.y + heroStruct.speed;
	  }
  	  heroStruct.direction = Jcompas.SOUTH;
	  nextHeroFrame();
   }

   if (Direction == Jcompas.NORTH){
	  if(ObjectCollisionCheck(Jcompas.NORTH)){
//         if((MAP_CAMERAY >= MAP_LAYER_SPEEDY) && (heroStruct.map.y < (MAP_HEIGHT - (SCREEN_HEIGHT/2)-(heroStruct.height/2))))
//            MAP_CAMERAY = MAP_CAMERAY - MAP_LAYER_SPEEDY;
	     heroStruct.map.y = heroStruct.map.y - heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.NORTH;
	  nextHeroFrame();
   }

   if (Direction == Jcompas.WEST){
	  if(ObjectCollisionCheck(Jcompas.WEST)){
//         if((MAP_CAMERAX >= MAP_LAYER_SPEEDX) && (heroStruct.map.x < (MAP_WIDTH - (SCREEN_WIDTH/2)-(heroStruct.width/2))))
//            MAP_CAMERAX = MAP_CAMERAX - MAP_LAYER_SPEEDX;
	     heroStruct.map.x = heroStruct.map.x - heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.WEST;
	  nextHeroFrame();
   }

   if (Direction == Jcompas.EAST){
	  if(ObjectCollisionCheck(Jcompas.EAST)){
//         if(((MAP_CAMERAX + SCREEN_WIDTH) < (MAP_WIDTH-MAP_LAYER_SPEEDX)) && ((heroStruct.map.x-MAP_CAMERAX) > ((SCREEN_WIDTH/2)-(heroStruct.width/2))))
//            MAP_CAMERAX = MAP_CAMERAX + MAP_LAYER_SPEEDX;
	     heroStruct.map.x = heroStruct.map.x + heroStruct.speed;
	  }
      heroStruct.direction = Jcompas.EAST;
	  nextHeroFrame();
   }
}

// ***************** Helper methods for CURRENT_GAMESTATE = STATE_PLAYING ************************
BOOL ObjectCollisionCheck(int direction){
    BOOL f_isnotBlocked = true;
	int Calc = 0;

	switch(direction){
	   case SOUTH:
          Calc = (heroStruct.map.x+(heroStruct.width/2))+((heroStruct.map.y+heroStruct.height)* Img_BackMask.bitmapinfoheader.biWidth);
	      if (Img_BackMask.buffer[Calc] == 0)
			  f_isnotBlocked = false;
       break;
	   case NORTH:
          Calc = (heroStruct.map.x+(heroStruct.width/2))+((heroStruct.map.y+20)* Img_BackMask.bitmapinfoheader.biWidth);
	      if (Img_BackMask.buffer[Calc] == 0)
			  f_isnotBlocked = false;
       break;
	   case EAST:
          Calc = (heroStruct.map.x + heroStruct.width-15)+((heroStruct.map.y + (heroStruct.height/2)) * Img_BackMask.bitmapinfoheader.biWidth);
	      if (Img_BackMask.buffer[Calc] == 0)
			  f_isnotBlocked = false;
	   break;
	   case WEST:
          Calc = (heroStruct.map.x+15)+((heroStruct.map.y+(heroStruct.height/2))* Img_BackMask.bitmapinfoheader.biWidth);
	      if (Img_BackMask.buffer[Calc] == 0)
			  f_isnotBlocked = false;
	      if (Img_BackMask.buffer[Calc] == scripts[0].key)
//			  f_isnotBlocked = false;
  		     getAnimationScript(scripts[0].scriptName);
	   break;

	}
    
	return f_isnotBlocked;
}

void nextHeroFrame(){
   
   if(heroStruct.currentFrame < 7) heroStruct.currentFrame ++; else heroStruct.currentFrame = 0;
   
   heroStruct.frame.x = heroStruct.frameSequence[heroStruct.currentFrame] * heroStruct.width;
   heroStruct.frame.y = heroStruct.direction * heroStruct.height;
}

void InputModeOverLay_JoyPad(){
   lpdijoy2->Poll(); // this is needed for joysticks only
   lpdijoy2->GetDeviceState(sizeof(DIJOYSTATE), (LPVOID)&joy_state);
   if (joy_state.rgbButtons[1] > 100){
      changeState(STATE_PLAYING);
   }
   if (joy_state.lY > 10){
	   if(Sword_loc != 5){
	     Sword_loc ++;
         PROCESSING_GAMESTATE = STATE_BUSY;
	   }
   }
   if (joy_state.lY < -10){
	   if(Sword_loc != 0){
	     Sword_loc --;
         PROCESSING_GAMESTATE = STATE_BUSY;
	   }
   }
}

void InputModeOverLay(){
   if(KEY_DOWN(VK_F1)){
      changeState(STATE_PLAYING);
   }
   if(KEY_DOWN(VK_DOWN)){
	   if(Sword_loc != 5){
	     Sword_loc ++;
         PROCESSING_GAMESTATE = STATE_BUSY;
	   }
   }
   if(KEY_DOWN(VK_UP)){
	   if(Sword_loc != 0){
	     Sword_loc --;
         PROCESSING_GAMESTATE = STATE_BUSY;
	   }
   }
}

// ***************************** Code for console input **************************************
void InputModeConsole(){
   char ch;
   int delaytime = 1;

   if(KEY_DOWN(VK_F12)){           // F12 gets you in and out of the console menu
      changeState(STATE_PLAYING);
   }
   else{
      ch = getKeyPressed_DI(); // attempt to get the letter that the user typed

      if (ch != NULL){
		 if(ch == '\n'){   // When the user hits backspace, delete the last letter typed
			 if (ptr_ConsolePrompt != 0){ // If there are no letters to delete, then don't continue
		        msg_Console[--ptr_ConsolePrompt] = NULL;
			    setBusyState(delaytime);
			 }
		 }
		 else if(ch == '\r'){  // When the user hits enter
            ptr_ConsolePrompt = 0;
  	        stringToUpper(msg_Console);
			scrollConsole();

			if (processCommand(msg_Console)){
     		   scrollConsole();
			   rg_ConsoleLine[ConsoleLine_MAX-1] = StringCopy("Command excepted");
			}

			msg_Console[ptr_ConsolePrompt] = NULL;
   	        setBusyState(delaytime);
		 }
		 else{ // if the user typed a normal character then add it to the screen
	        msg_Console[ptr_ConsolePrompt++] = ch;
            msg_Console[ptr_ConsolePrompt] = NULL;
            setBusyState(delaytime); // set a delay for typing.  Without a delay it gets really 
			                         // hard to type.Example: Typing the letter j would come 
			                         // out jjj :(
		 }
	  }
   }
}

// Retreave keyboard input using Direct input
char getKeyPressed_DI(){
   char keypressed = NULL;
   lpdikey->GetDeviceState(256, (LPVOID)keyboard_state);

   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_0] ? '0' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_1] ? '1' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_2] ? '2' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_3] ? '3' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_4] ? '4' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_5] ? '5' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_6] ? '6' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_7] ? '7' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_8] ? '8' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_9] ? '9' : NULL;	   

   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_A] ? 'a' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_B] ? 'b' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_C] ? 'c' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_D] ? 'd' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_E] ? 'e' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_F] ? 'f' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_G] ? 'g' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_H] ? 'h' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_I] ? 'i' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_J] ? 'j' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_K] ? 'k' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_L] ? 'l' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_M] ? 'm' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_N] ? 'n' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_O] ? 'o' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_P] ? 'p' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_Q] ? 'q' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_R] ? 'r' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_S] ? 's' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_T] ? 't' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_U] ? 'u' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_V] ? 'v' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_W] ? 'w' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_X] ? 'x' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_Y] ? 'y' : NULL;	   
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_Z] ? 'z' : NULL;	   

   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_SPACE] ? ' ' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_RETURN] ? '\r' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_BACK] ? '\n' : NULL;

   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_EQUALS] ? '=' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_LBRACKET] ? '[' : NULL;
   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_RBRACKET] ? ']' : NULL;

   if(keypressed == NULL)
      keypressed = keyboard_state[DIK_PERIOD ] ? '.' : NULL;

   return keypressed;
}

// Retreave keyboard input using Windows 
char getKeyPressed (){
	char keypressed = NULL;

	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x30) ? '0' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x31) ? '1' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x32) ? '2' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x33) ? '3' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x34) ? '4' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x35) ? '5' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x36) ? '6' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x37) ? '7' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x38) ? '8' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x39) ? '9' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0xBE) ? '.' : NULL;

	if(keypressed == NULL)
   	   keypressed = KEY_DOWN(0x41) ? 'a' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x42) ? 'b' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x43) ? 'c' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x44) ? 'd' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x45) ? 'e' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x46) ? 'f' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x47) ? 'g' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x48) ? 'h' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x49) ? 'i' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x4A) ? 'j' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x4B) ? 'k' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x4C) ? 'l' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x4D) ? 'm' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x4E) ? 'n' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x4F) ? 'o' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x50) ? 'p' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x51) ? 'q' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x52) ? 'r' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x53) ? 's' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x54) ? 't' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x55) ? 'u' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x56) ? 'v' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x57) ? 'w' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x58) ? 'x' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x59) ? 'y' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0x5A) ? 'z' : NULL;

	if(keypressed == NULL)
	   keypressed = KEY_DOWN(VK_SPACE) ? ' ' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(VK_RETURN) ? '\r' : NULL;
	if(keypressed == NULL)
	   keypressed = KEY_DOWN(VK_BACK) ? '\n' : NULL;

	if(keypressed == NULL)
	   keypressed = KEY_DOWN(0xBB) ? '=' : NULL;
	return keypressed;
}