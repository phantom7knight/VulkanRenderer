#pragma once

//Library Includes
#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <memory.h>
#include <cstring>
#include <string>
#include <string.h>
#include <optional>
#include <algorithm>
#include <direct.h>
#include <chrono>

// structures
#include <set>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>

// Threading
#include <mutex>
#include <thread>
#include <condition_variable>

#define GLFW_INCLUDE_VULKAN
#include "../../../Dependencies/GLFW/include/GLFW/glfw3.h"

//Above line replaces the line below so no need to worry about including it 
//#include <vulkan/vulkan.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "../../../Dependencies/GLM/glm/glm/glm.hpp"
#include "../../../Dependencies/GLM/glm/glm/vec4.hpp"
#include "../../../Dependencies/GLM/glm/glm/mat4x4.hpp"
#include "../../../Dependencies/GLM/glm/glm/gtc/matrix_transform.hpp"


//File Includes
#include "../Helper/Constants/Constans.hpp"
#include "../ResourceLoading/ResourceLoader.h"