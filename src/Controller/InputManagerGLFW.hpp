
#pragma once

#include <GLFW/glfw3.h>

#include "EventManager.hpp"
#include "Model/Components/Camera.hpp"
/**
 * @class InputManagerGLFW
 * @brief Manages the GLFW input
 */
class InputManagerGLFW
{
  public:
    /**
     * Returns this instance of InputManagerGLFW
     * @return InputManagerGLFW&
     */
	  static InputManagerGLFW& getInstance();
      ///Deleted copy constructor
	  InputManagerGLFW(InputManagerGLFW const&) = delete;
      ///Deleted = operator
	  void operator=(InputManagerGLFW const&) = delete;
      /**
       * @brief Processes Keyboard Input
       * @param GLFWwindow* - window
       */
	  void processKeyboard(GLFWwindow * window);
      /**
       * @brief Processes Mouse Input
       * @param GLFWwindow* - window
       */
	  void processMouse(GLFWwindow* window);

		/**
		* @brief Processes gamepad button Input	
		*/
	  void processGamepadButtons();

		/**
		* @brief Processes gamepad axis Input
		*/
	  void processGamepadAxis();

  private:
      /**
       * @brief Privatised constructor for Singleton Pattern
       */
	  InputManagerGLFW() : mouseInit(0) {}
      ///Mouse Init
	  bool mouseInit;
      ///Last Mouse X and Y
	  double lastX, lastY;
};