#pragma once

//Library Includes
#include <iostream>
#include <stdlib.h>
#include <memory.h>


#define GLFW_INCLUDE_VULKAN
#include "../Dependencies/GLFW/include/GLFW/glfw3.h"

//Above line replaces the line below so no need to worry about including it 
//#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../Dependencies/GLM/glm/glm/vec4.hpp"
#include "../Dependencies/GLM/glm/glm/mat4x4.hpp"



//File Includes
#include "Constans.hpp"