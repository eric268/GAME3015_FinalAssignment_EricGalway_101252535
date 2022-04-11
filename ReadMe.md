 \mainpage Main Page 
 
  \section DirectX12 DirectX12 Engine Framework GAME3015 Final Assignment W2022
   

   This project includes the framework needed to build games using DirectX12. It contains a game and world scene. These scenes are encapsulated with a scene stack which contains\n
   a title screen, main menu screen, game screen and pause screen.The game scene initializes the world and handles the DirectX12 required initializations and updates. The\n
   world scene updates and draws the scene graph. All game objects that need to be drawn and updated correctly must be attached as a child of the scene node or have a parent\n
   that is a child of the scene node. A player class has been added to allow for the handeling of player inputs. Currently some keyboard inputs have been implemented to allow for\n 
   control of a plane. More inputs can be added in the Player.cpp file.\n
	   

  @note
The UI controls are as follows:\n @note **Up Arrow:** Move selection arrow up in main menu\n
	**Down Arrow:** Move selection arrow down in main menu\n
	**Return:** Select choice in main menu\n
  @note 
The game controls are as follows:\n @note **Left Arrow:** Move Left\n 
	**Right Arrow:** Move Right\n 
	**Up Arrow:** Move Up\n 
	**Down Arrow:** Move Down\n
	**P:** Pause Game\n
	**Backspace:** Return to main menu when paused\n
	**Return:** Resume game when paused\n

  @warning Not correctly attaching game objects to the scene node will result in them not being correctly drawn and updated.
    
 
  @author **Eric Galway**

 
 