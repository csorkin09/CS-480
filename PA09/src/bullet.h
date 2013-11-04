#ifndef _BULLET_H_
#define _BULLET_H_

#include <btBulletDynamicsCommon.h>

#define P1_X -5
#define P1_Y 2
#define P1_Z 0

#define P2_X 3
#define P2_Y 2
#define P2_Z 0

#define PUCK_X 0
#define PUCK_Y 0
#define PUCK_Z 0
class CObject{
	public:
		float x,y,z;
		float fx,fy,fz;
		void force(float,float,float);
		void force(btVector3);
		void clearForce();
		CObject();
};

class CBullet{
	public:
		CBullet();
		~CBullet();
		void update(float);                                                              
		void clearForces();
		void serve(int,int,int);
		
		bool collisionDetected;
		
		btBroadphaseInterface* broadphase;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* dynamicsWorld;
				
		//Board
		btRigidBody* boardRigidBody;
		btCollisionShape* boardShape;
		btDefaultMotionState* boardMotionState;
		
		// Paddle1
		btCollisionShape* paddle1CollisionShape;		
		btDefaultMotionState* paddle1MotionState;
		btRigidBody* paddle1RigidBody;
		
		// Paddle2
		btCollisionShape* paddle2CollisionShape;		
		btDefaultMotionState* paddle2MotionState;
		btRigidBody* paddle2RigidBody;
		
		// Puck
		btCollisionShape* puckCollisionShape;		
		btDefaultMotionState* puckMotionState;
		btRigidBody* puckRigidBody;
		
		// 4 Walls
		btCollisionShape* wall1Shape;		
		btDefaultMotionState* wall1MotionState;
		btRigidBody* wall1RigidBody;
		btCollisionShape* wall2Shape;		
		btDefaultMotionState* wall2MotionState;
		btRigidBody* wall2RigidBody;
		btCollisionShape* wall3Shape;		
		btDefaultMotionState* wall3MotionState;
		btRigidBody* wall3RigidBody;
		btCollisionShape* wall4Shape;		
		btDefaultMotionState* wall4MotionState;
		btRigidBody* wall4RigidBody;
		
		// Objects
		CObject puck;
		CObject paddle1;
		CObject paddle2;
		CObject board;
};

#endif
