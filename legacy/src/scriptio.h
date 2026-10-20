/*
Name: scriptio.h
Created: Feb 16 2002

Description:
	This header is used to read scripts and set up there parameters into the program.

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fstream.h>
#include <iostream.h>

int loadmapScript(char * mapName);
char * StringCopy(char * str1);
int loadHeroMatrix(char * filename);
bool processCommand(char * input);
void bufferMapAnimation(char * input, int counter);
void clearAnimationBuffer();

char * StringCopy(char * str1){
   char * str;

   if ((str = (char *) malloc(InputStringMax)) == NULL){
       printf("Not enough memory to allocate buffer\n");
       exit(1);
   }
   strcpy(str, str1);

   return str;
}

void clearAnimationBuffer(){
	for (int i=0; i < ANIMATIONSCRIPT_MAX; i++)
       if (mapAnimationBuffer[i] != NULL) mapAnimationBuffer[i]=NULL;
}

char * firstpart = ">> Running ";
bool getAnimationScript(char * scriptName){
   char ch_rg_temp[50];
   char  * input;
   ifstream InFile;
   int counter = 0;
   char * t_temp = StringCopy(firstpart);

   InFile.open(scriptName);

   clearAnimationBuffer();
   CURRENT_GAMESTATE = STATE_ANIMATION;
   //sprintf(t_name,">> Running Script %s",scriptName);
   if (scriptName != NULL){
      strcat(t_temp,scriptName);
      addMessagetoConsole(t_temp);
   }

   InFile >> ch_rg_temp;
   input = (char *) ch_rg_temp;

   while (!InFile.eof()){
	  mapAnimationBuffer[counter++]=StringCopy(input);

	  InFile >> ch_rg_temp;
	  input = (char *) ch_rg_temp;
   }

   InFile.close();
   return true;
}

int loadmapScript(char * mapName){
//	scripts[0].scriptName="scripts\\actions\\action1.spt";

//   FILE *out;
   char ch_rg_temp[50];
   char  * input;
   ifstream InFile;
   int counter = 0;

   InFile.open(mapName);
/*
   if ((out = fopen("temp.txt", "w"))== NULL){
      fprintf(stderr, "Cannot open input file.\n");
      return 1;
   }
*/
   while (!InFile.eof()){
	  InFile >> ch_rg_temp;
	  input = (char *) ch_rg_temp;
	  
	  // If there is an animation script in a map then buffer it else setup map parmeters
	  if(strcmp(input,"StartMapAnimation[")==0){
         // Tell the renderer that the animation is playing
		 CURRENT_GAMESTATE = STATE_ANIMATION;
         InFile >> ch_rg_temp;
         input = (char *) ch_rg_temp;
		 
		 // buffer the animation in the mapAnimationBuffer and quit when you reach the "]"
         while (strcmp(input,"]")!=0){
			if (mapAnimationBuffer[counter] != NULL)
				free (mapAnimationBuffer[counter]);

			mapAnimationBuffer[counter++]=StringCopy(input);

            InFile >> ch_rg_temp;
            input = (char *) ch_rg_temp;

            // exit the file even if the user forgot to put a "]"
			if(InFile.eof())
		       break;
		 }
         //sprintf(strMsgDebug,"%s",mapAnimationBuffer[counter-1]);
	  }
	  else{
         // process a non animation script command
	     processCommand(input);
	  }
   }

   InFile.close();
//   fclose(out);

   return 0;
}

bool processCommand(char * input){
   char * pstrParse;
   pstrParse = NULL;
   pstrParse = strtok(input, "=");
   bool f_isValidCommand = false;
   
   if (pstrParse){
      if(strcmp(input,"STARTSERVER")==0){
         setupServerSocket(4000);
		 f_isValidCommand = true;
      }
      if(strcmp(input,"STARTCLIENT")==0){
         setupClientSocket("192.168.1.100");
		 f_isValidCommand = true;
      }
      if(strcmp(input,"LISTEN")==0){
         StartServer();
		 f_isValidCommand = true;
      }
      if(strcmp(input,"COLORKEY")==0){
         pstrParse = strtok(NULL, "=");

         if (pstrParse)
            scripts[colorkey].key = atoi(pstrParse);
		 f_isValidCommand = true;
      }
      if(strcmp(input,"SCRIPTNAME")==0){
         pstrParse = strtok(NULL, "=");

         if (pstrParse)
            scripts[colorkey++].scriptName = StringCopy(pstrParse);
		 f_isValidCommand = true;
      }
      // Get the X volocity of the camera speed for layer 1
      if(strcmp(input,"LAYER[0].SPEED")==0){
         pstrParse = strtok(NULL, "=");

         if (pstrParse)
            layers[0].speed = atoi(pstrParse);
		 f_isValidCommand = true;
      }
      if(strcmp(input,"LAYER[1].SPEED")==0){
         pstrParse = strtok(NULL, "=");
         if (pstrParse)
            MAP_LAYER_SPEEDX = atoi(pstrParse);
		 f_isValidCommand = true;
      }
      // Get the X volocity of the camera speed for layer 2
      if(strcmp(input,"LAYER[2].SPEED")==0){
	     pstrParse = strtok(NULL, "=");
	     if (pstrParse)
	        MAP_LAYER_SPEEDX = atoi(pstrParse);
		 f_isValidCommand = true;
	  }
      // Get the Y volocity of the camera speed for layer 2
      if(strcmp(input,"LAYER[2].YSPEED")==0){
	     pstrParse = strtok(NULL, "=");
	     if (pstrParse)
	        MAP_LAYER_SPEEDY = atoi(pstrParse);
		 f_isValidCommand = true;
	  }

      // Get the file name of Layer mask
      if(strcmp(input,"LAYER[MASK].NAME")==0){
	     pstrParse = strtok(NULL, "=");
         if (pstrParse){
	        MAP_LAYER_MASK_FILE = StringCopy(pstrParse);
		 } 
		 f_isValidCommand = true;
	  }
      // Get the file name of Layer 1
      if(strcmp(input,"LAYER[1].NAME")==0){
	     pstrParse = strtok(NULL, "=");
   	     if (pstrParse)
	        layers[0].name = StringCopy(pstrParse);
		 f_isValidCommand = true;
	  }
      // Get the file name of Layer 2
      if(strcmp(input,"LAYER[2].NAME")==0){
	     pstrParse = strtok(NULL, "=");
  	     if (pstrParse)
	        layers[1].name = StringCopy(pstrParse);
		 f_isValidCommand = true;
	  }

	  // Get the file name of Layer 2
      if(strcmp(input,"LAYER[3].NAME")==0){
	     pstrParse = strtok(NULL, "=");
  	     if (pstrParse)
	        layers[2].name = StringCopy(pstrParse);
		 f_isValidCommand = true;
      }

      // Get the width of a tile in the heros graphics matrix
 	  if(strcmp(input,"HERO.WIDTH")==0){
	     pstrParse = strtok(NULL, "=");
   	     if (pstrParse)
	         HERO_WIDTH = atoi(pstrParse);
		 f_isValidCommand = true;
	  }
      // Get the hieght of a tile in the heros graphics matrix
      if(strcmp(input,"HERO.HEIGHT")==0){
	     pstrParse = strtok(NULL, "=");
	     if (pstrParse)
	        HERO_HEIGHT = atoi(pstrParse);
		 f_isValidCommand = true;
	  }

      // Get the Grid of Tiles that the graphics reside in for the hero
      if(strcmp(input,"HERO.TILES")==0){
	     pstrParse = strtok(NULL, "=");
		 if (pstrParse)
            heroStruct.filename = StringCopy(pstrParse);
		 f_isValidCommand = true;
	  }

      // Get the starting x position of the hero
      if(strcmp(input,"HERO.LOCX")==0){
         pstrParse = strtok(NULL, "=");
         if (pstrParse)
            heroStruct.map.x = atoi(pstrParse);
		 f_isValidCommand = true;
      }
      // Get the starting y position of the hero
	  if(strcmp(input,"HERO.LOCY")==0){
	     pstrParse = strtok(NULL, "=");
	     if (pstrParse)
	        heroStruct.map.y = atoi(pstrParse);
		 f_isValidCommand = true;
	  }

      // Get the number of frames that the hero has for walking
      if(strcmp(input,"HERO.WALKFRAMES")==0){
	     pstrParse = strtok(NULL, "=");
	     if (pstrParse)
	        HERO_WALKFRAMEMAX = atoi(pstrParse);
		 f_isValidCommand = true;
	  }

      // Get the x position of the Camera
      if(strcmp(input,"MAP.CAMERAX")==0){
	     pstrParse = strtok(NULL, "=");
	     if (pstrParse)
	        MAP_CAMERAX = atoi(pstrParse);
		 f_isValidCommand = true;
	  }
      // Get the y position of the Camera
      if(strcmp(input,"MAP.CAMERAY")==0){
	     pstrParse = strtok(NULL, "=");
	     if (pstrParse)
	        MAP_CAMERAY = atoi(pstrParse);
		 f_isValidCommand = true;
	  }

      if(strcmp(input,"SET MAPINFO")==0){
         DEBUGING_MODE = DEBUGING_MODE_MAPINFO;
		 f_isValidCommand = true;
	  }
      if(strcmp(input,"SET HEROINFO")==0){
         DEBUGING_MODE = DEBUGING_MODE_HEROINFO;
		 f_isValidCommand = true;
	  }
      if(strcmp(input,"SET NOINFO")==0){
         DEBUGING_MODE = DEBUGING_MODE_NONE;
		 f_isValidCommand = true;
	  }
	  // ***************** Weather **********************

      char * str_weatherState;
      if(strcmp(input,"WEATHER")==0){
	     pstrParse = strtok(NULL, "=");
  	     if (pstrParse)
	        str_weatherState = StringCopy(pstrParse);
		    if (strcmp(str_weatherState,"RAIN")==0){
  		       changeWeatherState(WEATHER_RAIN);
			}
            if (strcmp(str_weatherState,"SNOW")==0){
  		       changeWeatherState(WEATHER_SNOW);
			}
            if (strcmp(str_weatherState,"PATICLE")==0){
  		       changeWeatherState(WEATHER_PARTICLE);
			}
            if (strcmp(str_weatherState,"FERRIES")==0){
  		       changeWeatherState(WEATHER_FERRY);
			}
            if (strcmp(str_weatherState,"NONE")==0){
  		       CURRENT_WEATHERSTATE = NO_WEATHER;
			}

		 f_isValidCommand = true;
      }
   }
   return f_isValidCommand;
}
int loadHeroMatrix(char * filename){
   //Load_Bitmap_File(&Img_Hero, filename);
   return 1;
}
