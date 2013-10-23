
#include <btBulletDynamicsCommon.h>

class CObject{
	public:
		float x,y,z;
		float vx,vy,vz;
		void force(float,float,float);
		void clearForce();
		CObject();
};

class CBullet{
	public:
		CBullet();
		~CBullet();
		void update(float);                                                              
		
		btBroadphaseInterface* broadphase;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* dynamicsWorld;
		
		// Board
		btDefaultMotionState* boardMotionState;
		btRigidBody* boardRigidBody;
		btCollisionShape* boardShape;
		
		//Box
		btRigidBody* boxRigidBody;
		btCollisionShape* boxShape;
		btDefaultMotionState* boxMotionState;
		
		//Wall
		btRigidBody* wallRigidBody;
		btCollisionShape* wallShape;
		btDefaultMotionState* wallMotionState;
		
		// Sphere
		btCollisionShape* fallShape;		
		btDefaultMotionState* fallMotionState;
		btRigidBody* fallRigidBody;
		
		// Cylinder
		btCollisionShape* cylinderCollisionShape;		
		btDefaultMotionState* cylinderMotionState;
		btRigidBody* cylinderRigidBody;
		
		// Objects
		CObject sphere;
		CObject box;
		CObject cylinder;
		CObject board;
};
