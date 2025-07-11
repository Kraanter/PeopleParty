#include "spatial_grid.h"
#include <algorithm>
#include <cmath>

// Custom hash function for pair<RigidBody*, RigidBody*>
struct PairHash {
    std::size_t operator()(const std::pair<RigidBody*, RigidBody*>& pair) const {
        std::size_t h1 = std::hash<RigidBody*>{}(pair.first);
        std::size_t h2 = std::hash<RigidBody*>{}(pair.second);
        return h1 ^ (h2 << 1);
    }
};

SpatialGrid::SpatialGrid(float cellSize, const Vector2D& worldMin, const Vector2D& worldMax)
    : m_cellSize(cellSize)
    , m_worldMin(worldMin)
    , m_worldMax(worldMax)
{
}

void SpatialGrid::Clear() {
    m_grid.clear();
    m_bodyToCells.clear();
}

void SpatialGrid::AddBody(RigidBody* body) {
    if (!body || m_bodyToCells.find(body) != m_bodyToCells.end()) {
        return; // Body already exists
    }
    
    InsertBodyIntoGrid(body);
}

void SpatialGrid::RemoveBody(RigidBody* body) {
    if (!body) return;
    
    RemoveBodyFromGrid(body);
}

void SpatialGrid::UpdateBody(RigidBody* body) {
    if (!body) return;
    
    // Remove from old position
    RemoveBodyFromGrid(body);
    
    // Add to new position
    InsertBodyIntoGrid(body);
}

std::vector<RigidBody*> SpatialGrid::GetNearbyBodies(RigidBody* body) const {
    if (!body) return {};
    
    std::vector<RigidBody*> nearbyBodies;
    std::unordered_set<RigidBody*> uniqueBodies;
    
    // Get all cells this body occupies
    std::vector<GridCell> cells = GetCellsForBody(body);
    
    for (const GridCell& cell : cells) {
        auto it = m_grid.find(cell);
        if (it != m_grid.end()) {
            for (RigidBody* otherBody : it->second) {
                if (otherBody != body && uniqueBodies.find(otherBody) == uniqueBodies.end()) {
                    nearbyBodies.push_back(otherBody);
                    uniqueBodies.insert(otherBody);
                }
            }
        }
    }
    
    return nearbyBodies;
}

std::vector<RigidBody*> SpatialGrid::GetBodiesInCell(const GridCell& cell) const {
    auto it = m_grid.find(cell);
    if (it != m_grid.end()) {
        return it->second;
    }
    return {};
}

std::vector<RigidBody*> SpatialGrid::GetBodiesInRadius(const Vector2D& center, float radius) const {
    std::vector<RigidBody*> result;
    std::unordered_set<RigidBody*> uniqueBodies;
    
    // Calculate which cells are within the radius
    int minX = static_cast<int>(floor((center.x - radius - m_worldMin.x) / m_cellSize));
    int maxX = static_cast<int>(ceil((center.x + radius - m_worldMin.x) / m_cellSize));
    int minY = static_cast<int>(floor((center.y - radius - m_worldMin.y) / m_cellSize));
    int maxY = static_cast<int>(ceil((center.y + radius - m_worldMin.y) / m_cellSize));
    
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            GridCell cell(x, y);
            auto it = m_grid.find(cell);
            
            if (it != m_grid.end()) {
                for (RigidBody* body : it->second) {
                    if (uniqueBodies.find(body) == uniqueBodies.end()) {
                        float distance = body->GetPosition().Distance(center);
                        if (distance <= radius + body->GetRadius()) {
                            result.push_back(body);
                            uniqueBodies.insert(body);
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

void SpatialGrid::GetPotentialCollisions(std::vector<std::pair<RigidBody*, RigidBody*>>& collisionPairs) const {
    std::unordered_set<std::pair<RigidBody*, RigidBody*>, PairHash> uniquePairs;
    
    for (const auto& cellPair : m_grid) {
        const std::vector<RigidBody*>& bodies = cellPair.second;
        
        // Check all pairs within this cell
        for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                RigidBody* body1 = bodies[i];
                RigidBody* body2 = bodies[j];
                
                // Ensure consistent ordering to avoid duplicate pairs
                if (body1 > body2) {
                    std::swap(body1, body2);
                }
                
                std::pair<RigidBody*, RigidBody*> pair(body1, body2);
                
                if (uniquePairs.find(pair) == uniquePairs.end()) {
                    collisionPairs.push_back(pair);
                    uniquePairs.insert(pair);
                }
            }
        }
    }
}

GridCell SpatialGrid::WorldToGrid(const Vector2D& worldPos) const {
    int x = static_cast<int>(floor((worldPos.x - m_worldMin.x) / m_cellSize));
    int y = static_cast<int>(floor((worldPos.y - m_worldMin.y) / m_cellSize));
    return GridCell(x, y);
}

Vector2D SpatialGrid::GridToWorld(const GridCell& cell) const {
    float x = m_worldMin.x + cell.x * m_cellSize;
    float y = m_worldMin.y + cell.y * m_cellSize;
    return Vector2D(x, y);
}

std::vector<GridCell> SpatialGrid::GetCellsForBody(RigidBody* body) const {
    std::vector<GridCell> cells;
    
    Vector2D pos = body->GetPosition();
    float radius = body->GetRadius();
    
    // Calculate bounding box
    Vector2D min = pos - Vector2D(radius, radius);
    Vector2D max = pos + Vector2D(radius, radius);
    
    // Get grid cells that overlap with the bounding box
    GridCell minCell = WorldToGrid(min);
    GridCell maxCell = WorldToGrid(max);
    
    for (int y = minCell.y; y <= maxCell.y; ++y) {
        for (int x = minCell.x; x <= maxCell.x; ++x) {
            cells.emplace_back(x, y);
        }
    }
    
    return cells;
}

void SpatialGrid::InsertBodyIntoGrid(RigidBody* body) {
    std::vector<GridCell> cells = GetCellsForBody(body);
    
    for (const GridCell& cell : cells) {
        m_grid[cell].push_back(body);
    }
    
    m_bodyToCells[body] = std::unordered_set<GridCell, GridCellHash>(cells.begin(), cells.end());
}

void SpatialGrid::RemoveBodyFromGrid(RigidBody* body) {
    auto it = m_bodyToCells.find(body);
    if (it == m_bodyToCells.end()) {
        return;
    }
    
    // Remove body from all cells it occupies
    for (const GridCell& cell : it->second) {
        auto cellIt = m_grid.find(cell);
        if (cellIt != m_grid.end()) {
            auto& bodies = cellIt->second;
            bodies.erase(std::remove(bodies.begin(), bodies.end(), body), bodies.end());
            
            // Remove empty cells
            if (bodies.empty()) {
                m_grid.erase(cellIt);
            }
        }
    }
    
    // Remove body from tracking
    m_bodyToCells.erase(it);
}
