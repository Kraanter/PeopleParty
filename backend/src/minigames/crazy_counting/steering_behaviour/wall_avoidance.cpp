#include "wall_avoidance.h"
#include <cmath>

const float MaxFloat = (std::numeric_limits<float>::max)();
const float HalfPi = 3.14159 / 2;

WallAvoidance::WallAvoidance() {
    entity = nullptr;
}

WallAvoidance::WallAvoidance(CrazyCounting_Entity* entity) {
    this->entity = entity;

    CreateFeelers();

    walls.push_back(Wall2D(Vector2D(0.0, 0.0), Vector2D(1.0, 0.0)));
    walls.push_back(Wall2D(Vector2D(1.0, 0.0), Vector2D(1.0, 1.0)));
    walls.push_back(Wall2D(Vector2D(1.0, 1.0), Vector2D(0.0, 1.0)));
    walls.push_back(Wall2D(Vector2D(0.0, 1.0), Vector2D(0.0, 0.0)));
}

inline void Vec2DRotateAroundOrigin(Vector2D& v, float ang)
{
  //create a transformation matrix
  Matrix2D mat;

  //rotate
  mat.Rotate(ang);
	
  //now transform the object's vertices
  mat.TransformVector2Ds(v);
}

void WallAvoidance::CreateFeelers()
{
    float WallDetectionFeelerLength = 0.4f;
    //feeler pointing straight in front
    feelers.push_back(entity->position + WallDetectionFeelerLength * entity->heading);

    // //feeler to left
    // Vector2D temp = entity->heading;
    // Vec2DRotateAroundOrigin(temp, HalfPi * 3.5f);
    // feelers.push_back(entity->position + WallDetectionFeelerLength/2.0f * temp);

    // //feeler to right
    // temp = entity->heading;
    // Vec2DRotateAroundOrigin(temp, HalfPi * 0.5f);
    // feelers.push_back(entity->position + WallDetectionFeelerLength/2.0f * temp);
}

inline bool LineIntersection2D(Vector2D   A,
                               Vector2D   B,
                               Vector2D   C, 
                               Vector2D   D,
                               float&     dist,
                               Vector2D&  point)
{

  float rTop = (A.y-C.y)*(D.x-C.x)-(A.x-C.x)*(D.y-C.y);
	float rBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);

	float sTop = (A.y-C.y)*(B.x-A.x)-(A.x-C.x)*(B.y-A.y);
	float sBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);

	if ( (rBot == 0) || (sBot == 0))
	{
		//lines are parallel
		return false;
	}

	float r = rTop/rBot;
	float s = sTop/sBot;

	if( (r > 0) && (r < 1) && (s > 0) && (s < 1) )
  {
  	dist = Vec2DDistance(A,B) * r;

    point = A + r * (B - A);

    return true;
  }

	else
  {
		dist = 0;

    return false;
  }
}

Vector2D WallAvoidance::Calculate() {
    if (entity == nullptr) {
        return Vector2D(0, 0);
    }
  float DistToThisIP    = 0.0;
  float DistToClosestIP = MaxFloat;

  //this will hold an index into the vector of walls
  int ClosestWall = -1;

  Vector2D SteeringForce,
            point,         //used for storing temporary info
            ClosestPoint;  //holds the closest intersection point

  //examine each feeler in turn
  for (unsigned int flr=0; flr<feelers.size(); ++flr)
  {
    //run through each wall checking for any intersection points
    for (unsigned int w=0; w<walls.size(); ++w)
    {
      if (LineIntersection2D(entity->position,
                             feelers[flr],
                             walls[w].From(),
                             walls[w].To(),
                             DistToThisIP,
                             point))
      {
        //is this the closest found so far? If so keep a record
        if (DistToThisIP < DistToClosestIP)
        {
          DistToClosestIP = DistToThisIP;

          ClosestWall = w;

          ClosestPoint = point;
        }
      }
    }//next wall

  
    //if an intersection point has been detected, calculate a force  
    //that will direct the agent away
    if (ClosestWall >=0)
    {
      //calculate by what distance the projected position of the agent
      //will overshoot the wall
      Vector2D OverShoot = feelers[flr] - ClosestPoint;

      //create a force in the direction of the wall normal, with a 
      //magnitude of the overshoot
      SteeringForce = walls[ClosestWall].Normal() * OverShoot.Length();
    }

  }//next feeler

  return SteeringForce;
}