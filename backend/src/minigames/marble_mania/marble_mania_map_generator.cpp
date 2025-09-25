#include "marble_mania_map_generator.h"
#include <algorithm>
#include <cmath>

MarbleManiaMapGenerator::MarbleManiaMapGenerator(float worldMinX, float worldMaxX, 
                                               float worldMinY, float worldMaxY, 
                                               float marbleRadius, unsigned int seed)
    : worldMinX_(worldMinX), worldMaxX_(worldMaxX), worldMinY_(worldMinY), worldMaxY_(worldMaxY),
      marbleRadius_(marbleRadius), obstacleCounter_(0)
{
    worldWidth_ = worldMaxX_ - worldMinX_;
    worldHeight_ = worldMaxY_ - worldMinY_;
    
    // Set default parameters
    minObstacleSpacing_ = marbleRadius_ * 3.0f;  // Ensure marbles can pass between obstacles
    obstacleDensity_ = 0.015f;  // Very low density - obstacles per square unit
    platformInterval_ = 800.0f;   // Platform every 800 units down
    minTilt_ = 0.1f;             // Minimum 0.1 radians (~6 degrees)
    maxTilt_ = 0.4f;             // Maximum 0.4 radians (~23 degrees)
    // above values are overwritten inside the map.cpp create generator function.
    
    // Initialize random number generation
    if (seed == 0) {
        std::random_device rd;
        seed = rd();
    }
    rng_.seed(seed);
    
    rotationDist_ = std::uniform_real_distribution<float>(-maxTilt_, maxTilt_);
    positionXDist_ = std::uniform_real_distribution<float>(worldMinX_, worldMaxX_);
    unitDist_ = std::uniform_real_distribution<float>(0.0f, 1.0f);
}

std::vector<ObstacleInfo> MarbleManiaMapGenerator::GenerateObstacles() {
    generatedObstacles_.clear();
    obstacleCounter_ = 0;
    
    // Generate platforms first (they have priority positioning)
    auto platforms = generatePlatforms_();
    generatedObstacles_.insert(generatedObstacles_.end(), platforms.begin(), platforms.end());
    
    // Generate random obstacles
    auto randomObstacles = generateRandomObstacles_();
    generatedObstacles_.insert(generatedObstacles_.end(), randomObstacles.begin(), randomObstacles.end());
    
    return generatedObstacles_;
}

std::vector<ObstacleInfo> MarbleManiaMapGenerator::generateRandomObstacles_() {
    std::vector<ObstacleInfo> obstacles;
    
    // Calculate total area and number of obstacles to generate
    float totalArea = worldWidth_ * worldHeight_;
    int targetObstacleCount = static_cast<int>(totalArea * obstacleDensity_);
    
    // Generate obstacles at random Y positions (avoiding platform zones and drop zone)
    std::vector<float> yPositions;
    float dropZoneBuffer = 300.0f;  // Buffer below drop zone before obstacles start
    
    for (int i = 0; i < targetObstacleCount; ++i) {
        float y = worldMinY_ + unitDist_(rng_) * worldHeight_;
        
        // Skip if in drop zone + buffer area
        if (y < worldMinY_ + dropZoneBuffer) {
            continue;
        }

        // don't create in the last 50, otherwise it will be in finish line
        if (y > worldMaxY_ - 50.0f) {
            continue;
        }

        // Skip if too close to a platform Y position
        bool tooCloseToPlatform = false;
        for (float platformY = worldMinY_ + platformInterval_; platformY < worldMaxY_; platformY += platformInterval_) {
            if (std::abs(y - platformY) < 80.0f) {  // 80 unit buffer around platforms
                tooCloseToPlatform = true;
                break;
            }
        }
        
        if (!tooCloseToPlatform) {
            yPositions.push_back(y);
        }
    }
    
    // Create obstacles at valid positions
    for (float y : yPositions) {
        ObstacleInfo obstacle = createRandomObstacle_(y);
        if (obstacle.position.x != 0.0f || obstacle.position.y != 0.0f) {  // Valid position found
            // Check for overlap with existing obstacles
            bool overlaps = false;
            for (const auto& existing : obstacles) {
                float dx = obstacle.position.x - existing.position.x;
                float dy = obstacle.position.y - existing.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                float combinedSize = (obstacle.width + existing.width) * 0.5f + minObstacleSpacing_;
                
                if (distance < combinedSize) {
                    overlaps = true;
                    break;
                }
            }
            
            // Only add if no overlap detected
            if (!overlaps) {
                obstacles.push_back(obstacle);
            }
        }
    }
    
    return obstacles;
}

ObstacleInfo MarbleManiaMapGenerator::createRandomObstacle_(float y) {
    ObstacleInfo obstacle;
    
    // Randomly choose obstacle type
    float typeRand = unitDist_(rng_);
    if (typeRand < 0.4f) {
        obstacle.type = ObstacleType::Circle;
        obstacle.width = obstacle.height = 30.0f + unitDist_(rng_) * 60.0f;  // Radius 30-90
    } else if (typeRand < 0.7f) {
        obstacle.type = ObstacleType::Rectangle;
        obstacle.width = 40.0f + unitDist_(rng_) * 100.0f;   // Width 40-140
        obstacle.height = 20.0f + unitDist_(rng_) * 70.0f;  // Height 30-130
    } else {
        obstacle.type = ObstacleType::Triangle;
        obstacle.width = 60.0f + unitDist_(rng_) * 80.0f;   // Width 60-120
        obstacle.height = 60.0f + unitDist_(rng_) * 80.0f;  // Height 60-110
    }
    
    // Find valid position
    obstacle.position = findValidPosition_(y, obstacle.width, obstacle.height);
    
    // Ensure obstacle has tilt (never perfectly horizontal)
    obstacle.rotation = getRandomRotation_();
    
    // Generate unique ID
    obstacle.id = generateObstacleId_("obs");
    
    return obstacle;
}

Vector2D MarbleManiaMapGenerator::findValidPosition_(float y, float width, float height, int maxAttempts) {
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        float x = positionXDist_(rng_);
        Vector2D pos(x, y);
        
        if (isPositionValid_(pos, width, height)) {
            return pos;
        }
    }
    
    // Return invalid position if no valid spot found
    return Vector2D(0.0f, 0.0f);
}

bool MarbleManiaMapGenerator::isPositionValid_(const Vector2D& pos, float width, float height) {
    // Check world bounds (with margin sufficient for marble passage)
    // Use same spacing as between obstacles to ensure marbles can pass between obstacle and wall
    float wallMargin = minObstacleSpacing_ + marbleRadius_;
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    if (pos.x - halfWidth < worldMinX_ + wallMargin || 
        pos.x + halfWidth > worldMaxX_ - wallMargin) {
        return false;
    }
    
    // Check distance from existing obstacles
    for (const auto& existing : generatedObstacles_) {
        float dx = pos.x - existing.position.x;
        float dy = pos.y - existing.position.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // Calculate minimum required distance
        float existingSize = std::max(existing.width, existing.height);
        float currentSize = std::max(width, height);
        float minDistance = (existingSize + currentSize) * 0.5f + minObstacleSpacing_;
        
        if (distance < minDistance) {
            return false;
        }
    }
    
    return true;
}

std::vector<ObstacleInfo> MarbleManiaMapGenerator::generatePlatforms_() {
    std::vector<ObstacleInfo> platforms;
    
    // Generate platforms every ~800 units down
    for (float y = worldMinY_ + platformInterval_; y < worldMaxY_; y += platformInterval_) {
        // Add some randomness to platform Y position (±50 units)
        float randomY = y + (unitDist_(rng_) - 0.5f) * 100.0f;
        
        PlatformType type = getRandomPlatformType_();
        ObstacleInfo platform = createPlatform_(randomY, type);
        platforms.push_back(platform);
    }
    
    return platforms;
}

ObstacleInfo MarbleManiaMapGenerator::createPlatform_(float y, PlatformType type) {
    ObstacleInfo platform;
    platform.type = ObstacleType::Rectangle;
    platform.width = getPlatformWidth_();
    platform.height = 20.0f + unitDist_(rng_) * 15.0f;  // Height 20-35
    platform.position.y = y;
    
    switch (type) {
        case PlatformType::LEFT_WALL:
            platform.position.x = worldMinX_ + platform.width * 0.4f;
            platform.rotation = minTilt_ + unitDist_(rng_) * (maxTilt_ - minTilt_);  // Positive tilt (right)
            break;
            
        case PlatformType::RIGHT_WALL:
            platform.position.x = worldMaxX_ - platform.width * 0.4f;
            platform.rotation = -(minTilt_ + unitDist_(rng_) * (maxTilt_ - minTilt_));  // Negative tilt (left)
            break;
            
        case PlatformType::CENTER:
            // Ensure marble-width clearance from walls
            float minX = worldMinX_ + platform.width * 0.4f + marbleRadius_ * 4;
            float maxX = worldMaxX_ - platform.width * 0.4f - marbleRadius_ * 4;
            platform.position.x = minX + unitDist_(rng_) * (maxX - minX);
            
            // Random tilt direction
            float tilt = minTilt_ + unitDist_(rng_) * (maxTilt_ - minTilt_);
            platform.rotation = (unitDist_(rng_) < 0.5f) ? tilt : -tilt;
            break;
    }
    
    platform.id = generateObstacleId_("platform");
    return platform;
}

PlatformType MarbleManiaMapGenerator::getRandomPlatformType_() {
    float rand = unitDist_(rng_);
    if (rand < 0.33f) return PlatformType::LEFT_WALL;
    if (rand < 0.66f) return PlatformType::RIGHT_WALL;
    return PlatformType::CENTER;
}

float MarbleManiaMapGenerator::getPlatformWidth_() {
    // 20-40% of map width
    float minWidth = worldWidth_ * 0.2f;
    float maxWidth = worldWidth_ * 0.4f;
    return minWidth + unitDist_(rng_) * (maxWidth - minWidth);
}

float MarbleManiaMapGenerator::getRandomRotation_() {
    float rotation;
    do {
        rotation = rotationDist_(rng_);
    } while (std::abs(rotation) < minTilt_);  // Ensure minimum tilt
    
    return rotation;
}

std::string MarbleManiaMapGenerator::generateObstacleId_(const std::string& prefix) {
    return prefix + "_" + std::to_string(++obstacleCounter_);
}