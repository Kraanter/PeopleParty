#ifndef SPATIAL_GRID_H
#define SPATIAL_GRID_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../math/vector2d.h"
#include "rigid_body.h"

struct GridCell {
    int x, y;
    
    GridCell(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
    
    bool operator==(const GridCell& other) const {
        return x == other.x && y == other.y;
    }
};

struct GridCellHash {
    std::size_t operator()(const GridCell& cell) const {
        return std::hash<int>()(cell.x) ^ (std::hash<int>()(cell.y) << 1);
    }
};

class SpatialGrid {
private:
    float m_cellSize;
    Vector2D m_worldMin;
    Vector2D m_worldMax;
    
    std::unordered_map<GridCell, std::vector<RigidBody*>, GridCellHash> m_grid;
    std::unordered_map<RigidBody*, std::unordered_set<GridCell, GridCellHash>> m_bodyToCells;
    
public:
    SpatialGrid(float cellSize, const Vector2D& worldMin, const Vector2D& worldMax);
    
    void Clear();
    void AddBody(RigidBody* body);
    void RemoveBody(RigidBody* body);
    void UpdateBody(RigidBody* body);
    
    std::vector<RigidBody*> GetNearbyBodies(RigidBody* body) const;
    std::vector<RigidBody*> GetBodiesInCell(const GridCell& cell) const;
    std::vector<RigidBody*> GetBodiesInRadius(const Vector2D& center, float radius) const;
    
    void GetPotentialCollisions(std::vector<std::pair<RigidBody*, RigidBody*>>& collisionPairs) const;
    
    // Debug information
    int GetTotalCells() const { return m_grid.size(); }
    int GetTotalBodies() const { return m_bodyToCells.size(); }
    
private:
    GridCell WorldToGrid(const Vector2D& worldPos) const;
    Vector2D GridToWorld(const GridCell& cell) const;
    
    std::vector<GridCell> GetCellsForBody(RigidBody* body) const;
    void InsertBodyIntoGrid(RigidBody* body);
    void RemoveBodyFromGrid(RigidBody* body);
};

#endif // SPATIAL_GRID_H
