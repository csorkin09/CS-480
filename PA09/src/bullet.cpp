// Manages bullet physics engine interactions.

#include "bullet.h"
#include <iostream>
#include <time.h>
using namespace std;

CObject::CObject(){
	x=0;
	y=0;
	z=0;
	fx=0;
	fy=0;
	fz=0;	
};

void CObject::force(float x,float y,float z){
	fx += x;
	fy += y;
	fz += z;
}

void CObject::force(btVector3 vec){
	fx += vec.getX();
	fy += vec.getY();
	fz += vec.getZ();
}
void CObject::clearForce(){
	fx = 0;
	fy = 0;
	fz = 0;
}


CBullet::CBullet(){
	collisionDetected = false;
	
	// Build the broadphase
	broadphase = new btDbvtBroadphase();
	
	// Set up the collision configuration and dispatcher
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	
	// The actual physics solver
	solver = new btSequentialImpulseConstraintSolver;
	
	// The world.
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,-50,0.0));
	
	
	btScalar puckMass = 0.1;
	btScalar paddleMass = 10;
	btScalar infMass = 0;
	
	//Set Puck	
	btVector3 puckInertia(0,0,0);
	puckCollisionShape = new btCylinderShape(btVector3(1.0f, 0.1f, 1.0f));
	puckMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(PUCK_X,PUCK_Y,PUCK_Z)));
	puckCollisionShape->calculateLocalInertia(puckMass,puckInertia);
	btRigidBody::btRigidBodyConstructionInfo puckRigidBodyCI(puckMass,puckMotionState,puckCollisionShape,puckInertia);
	puckRigidBody = new btRigidBody(puckRigidBodyCI);
	puckRigidBody->setRestitution(btScalar(1.1)); 
	dynamicsWorld->addRigidBody(puckRigidBody);
	
	//Set Paddle	1
	btVector3 paddle1Inertia(0,0,0);
	paddle1CollisionShape = new btCylinderShape(btVector3(1.2f, 0.1f, 1.2f));
	paddle1MotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(P1_X,P1_Y,P1_Z)));
	paddle1CollisionShape->calculateLocalInertia(paddleMass,paddle1Inertia);
	btRigidBody::btRigidBodyConstructionInfo paddle1RigidBodyCI(paddleMass,paddle1MotionState,paddle1CollisionShape,paddle1Inertia);
	paddle1RigidBody = new btRigidBody(paddle1RigidBodyCI);
	paddle1RigidBody->setRestitution(btScalar(0.1)); 
	paddle1RigidBodyCI.m_friction = 0.01f;
	dynamicsWorld->addRigidBody(paddle1RigidBody);
	
	//Set Paddle	2
	btVector3 paddle2Inertia(0,0,0);
	paddle2CollisionShape = new btCylinderShape(btVector3(1.2f, 0.1f, 1.2f));
	paddle2MotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(P2_X,P2_Y,P2_Z)));
	paddle2CollisionShape->calculateLocalInertia(paddleMass,paddle2Inertia);
	btRigidBody::btRigidBodyConstructionInfo paddle2RigidBodyCI(paddleMass,paddle2MotionState,paddle2CollisionShape,paddle2Inertia);
	paddle2RigidBodyCI.m_friction = 0.01f;	
	paddle2RigidBody = new btRigidBody(paddle2RigidBodyCI);
	paddle2RigidBody->setRestitution(btScalar(0.1));
	dynamicsWorld->addRigidBody(paddle2RigidBody);
	
	// Board
	boardShape = new btStaticPlaneShape(btVector3(0.0,1,0.0),1);
	boardMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
	btRigidBody::btRigidBodyConstructionInfo
		boardRigidBodyCI(0,boardMotionState,boardShape,btVector3(0,0,0));
	boardRigidBodyCI.m_friction = 0.15f;
	boardRigidBody = new btRigidBody(boardRigidBodyCI);
	boardRigidBody->setRestitution(btScalar(0.0));
	dynamicsWorld->addRigidBody(boardRigidBody);
	
	// Wall Bottom (norm Z)
	wall1Shape = new btStaticPlaneShape(btVector3(0.0,0.0,1),1);
	wall1MotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,-10))); 
	btRigidBody::btRigidBodyConstructionInfo
		wall1RigidBodyCI(0,wall1MotionState,wall1Shape,btVector3(0,0,0));
	wall1RigidBody = new btRigidBody(wall1RigidBodyCI);
	wall1RigidBody->setRestitution(btScalar(1.0)); 
	dynamicsWorld->addRigidBody(wall1RigidBody);
	
	// Wall Top (norm Z)
	wall2Shape = new btStaticPlaneShape(btVector3(0.0,0.0,-1),1);
	wall2MotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,5.11)));
	btRigidBody::btRigidBodyConstructionInfo
		wall2RigidBodyCI(0,wall2MotionState,wall2Shape,btVector3(0,0,0));
	wall2RigidBody = new btRigidBody(wall2RigidBodyCI);
	wall2RigidBody->setRestitution(btScalar(1.0));
	dynamicsWorld->addRigidBody(wall2RigidBody);

	// Wall Left (norm X)
	wall3Shape = new btStaticPlaneShape(btVector3(1,0.0,0.0),1);
	wall3MotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(-7.537952,0,0)));
	btRigidBody::btRigidBodyConstructionInfo
		wall3RigidBodyCI(0,wall3MotionState,wall3Shape,btVector3(0,0,0));
	wall3RigidBody = new btRigidBody(wall3RigidBodyCI);
	dynamicsWorld->addRigidBody(wall3RigidBody);

	// Wall Right (norm X)
	wall4Shape = new btStaticPlaneShape(btVector3(-1,0.0,0.0),1);
	wall4MotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(7.537952,0,0)));
	btRigidBody::btRigidBodyConstructionInfo
		wall4RigidBodyCI(0,wall4MotionState,wall4Shape,btVector3(0,0,0));
	wall4RigidBody = new btRigidBody(wall4RigidBodyCI);
	dynamicsWorld->addRigidBody(wall4RigidBody);

	srand (time(NULL));
	serve(rand()%2==1?-5:5,0,rand()%10);
}

CBullet::~CBullet(){
	delete dynamicsWorld;
	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
	 
	// Remove Rigid Bodies
}
void CBullet::clearForces(){ 
	puckRigidBody->setLinearVelocity(btVector3(0,0,0));
	puckRigidBody->clearForces();
	
	paddle1RigidBody->setLinearVelocity(btVector3(0,0,0));
	paddle1RigidBody->clearForces();
	
	paddle2RigidBody->setLinearVelocity(btVector3(0,0,0));
	paddle2RigidBody->clearForces();
	
}

void CBullet::serve(int x, int y, int z){
	btTransform pu;
	puckRigidBody->getMotionState()->getWorldTransform(pu);
	puck.x = pu.getOrigin().getX();
	puck.y = pu.getOrigin().getY();
	puck.z = pu.getOrigin().getZ();
	
	puckRigidBody->applyForce(btVector3(x*10,y*10,z*10),btVector3(puck.x,puck.y,puck.z));
	
}

void CBullet::update(float dt){
	
	dynamicsWorld->stepSimulation(dt,7);

	btTransform pu;
	btTransform p2;
	btTransform p1;
	btTransform bo;

	puckRigidBody->getMotionState()->getWorldTransform(pu);
	puck.x = pu.getOrigin().getX();
	puck.y = pu.getOrigin().getY();
	puck.z = pu.getOrigin().getZ();
	
	paddle1RigidBody->getMotionState()->getWorldTransform(p1);
	paddle1.x = p1.getOrigin().getX();
	paddle1.y = p1.getOrigin().getY();
	paddle1.z = p1.getOrigin().getZ();
	paddle1RigidBody->applyForce(btVector3(paddle1.fx,paddle1.fy,paddle1.fz),btVector3(paddle1.x,paddle1.y,paddle1.z));
	paddle1.clearForce();
	
	paddle2RigidBody->getMotionState()->getWorldTransform(p2);
	paddle2.x = p2.getOrigin().getX();
	paddle2.y = p2.getOrigin().getY();
	paddle2.z = p2.getOrigin().getZ();
	paddle2RigidBody->applyForce(btVector3(paddle2.fx,paddle2.fy,paddle2.fz),btVector3(paddle2.x,paddle2.y,paddle2.z));
	paddle2.clearForce();
	
	boardRigidBody->getMotionState()->getWorldTransform(bo);
	board.x = bo.getOrigin().getX();
	board.y = bo.getOrigin().getY();
	board.z = bo.getOrigin().getZ();

	// Collisions
	int count = 0;
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold =  dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		if ((obA->getCollisionShape() == puckCollisionShape || obB->getCollisionShape() == puckCollisionShape)
			&& (obA->getCollisionShape() != boardShape && obB->getCollisionShape() != boardShape)){
			count++;			
		}
	}
	collisionDetected=count > 4;		
}
