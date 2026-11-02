#pragma once

#include <vector>
#include <memory>
using std::shared_ptr;
using std::make_shared;
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "RedCppLib/RedCppLib.hpp"
using Red::vec2f;
using Red::vec3f;
using Red::vec3i;

#include "datatypes.hpp"
#include "UASimConstants.hpp"

// 1. Obiekty kolizyjne
#include "CollisionMaterials/CollisionMaterial.hpp"
#include "CollisionMaterials/CollisionNullMaterial.hpp"
#include "CollisionObjects/CollisionObject.hpp"
#include "CollisionObjects/CollisionBox.hpp"
#include "CollisionObjects/CollisionHeightmap.hpp"

// 2. Geometria 
#include "geometry.hpp"

// 3. Wave 
#include "wave.hpp"

// 4. Mikrofon
#include "microphone.hpp"

// 5. WaterTank
#include "WaterTank.hpp"

// 6. Fizyka
#include "physics.hpp"