#pragma once

//Library Includes
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <vector>
#include <cstring>
#include <optional>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <direct.h>

#define GLFW_INCLUDE_VULKAN
#include "../Dependencies/GLFW/include/GLFW/glfw3.h"

//Above line replaces the line below so no need to worry about including it 
//#include <vulkan/vulkan.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "../Dependencies/GLM/glm/glm/glm.hpp"
#include "../Dependencies/GLM/glm/glm/vec4.hpp"
#include "../Dependencies/GLM/glm/glm/mat4x4.hpp"
#include "../Dependencies/GLM/glm/glm/gtc/matrix_transform.hpp"





//File Includes
#include "Constans.hpp"
#include "ResourceLoader.h"
