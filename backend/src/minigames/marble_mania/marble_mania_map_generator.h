#ifndef PEOPLEPARTY_BACKEND_MARBLE_MANIA_MAP_GENERATOR_H
#define PEOPLEPARTY_BACKEND_MARBLE_MANIA_MAP_GENERATOR_H

#include <vector>
#include <random>
#include "../../util/math/vector2d.h"
#include "marble_mania_obstacle.h"

// Forward declarations
class MarbleManiaMap;

enum class PlatformType {
    LEFT_WALL,      // Platform touches left wall, tilted right
    RIGHT_WALL,     // Platform touches right wall, tilted left  
    CENTER          // Platform in center, tilted to either side
};

struct ObstacleInfo {
    Vector2D position;
    ObstacleType type;
    float width;
    float height;
    float rotation;
    std::string id;
};

class MarbleManiaMapGenerator {
public:
    MarbleManiaMapGenerator(float worldMinX, float worldMaxX, float worldMinY, float worldMaxY, 
                           float marbleRadius = 15.0f, unsigned int seed = 0);
    
    // Generate all obstacles for the map
    std::vector<ObstacleInfo> GenerateObstacles();
    
    // Configuration methods
    void SetObstacleDensity(float density) { obstacleDensity_ = density; }
    void SetPlatformInterval(float interval) { platformInterval_ = interval; }
    void SetMinTilt(float minTilt) { minTilt_ = minTilt; }
    void SetMaxTilt(float maxTilt) { maxTilt_ = maxTilt; }
    
private:
    // Random obstacle generation
    std::vector<ObstacleInfo> generateRandomObstacles_();
    ObstacleInfo createRandomObstacle_(float y);
    Vector2D findValidPosition_(float y, float width, float height, int maxAttempts = 50);
    bool isPositionValid_(const Vector2D& pos, float width, float height);
    
    // Platform generation
    std::vector<ObstacleInfo> generatePlatforms_();
    ObstacleInfo createPlatform_(float y, PlatformType type);
    PlatformType getRandomPlatformType_();
    float getPlatformWidth_();
    
    // Utility methods
    float getRandomRotation_();
    std::string generateObstacleId_(const std::string& prefix);
    
    // World bounds
    float worldMinX_, worldMaxX_, worldMinY_, worldMaxY_;
    float worldWidth_, worldHeight_;
    
    // Generation parameters
    float marbleRadius_;
    float minObstacleSpacing_;  // Minimum distance between obstacles
    float obstacleDensity_;     // Obstacles per square unit of world area
    float platformInterval_;    // Y distance between platforms
    float minTilt_;            // Minimum rotation for obstacles/platforms
    float maxTilt_;            // Maximum rotation for obstacles/platforms
    
    // Random number generation
    std::mt19937 rng_;
    std::uniform_real_distribution<float> rotationDist_;
    std::uniform_real_distribution<float> positionXDist_;
    std::uniform_real_distribution<float> unitDist_;  // 0.0 to 1.0
    
    // Obstacle tracking
    std::vector<ObstacleInfo> generatedObstacles_;
    int obstacleCounter_;
};

#endif // PEOPLEPARTY_BACKEND_MARBLE_MANIA_MAP_GENERATOR_H