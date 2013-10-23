#include "bullet.h"
#include <iostream>
using namespace std;

CObject::CObject(){
	x=0;
	y=0;
	z=0;
	vx=0;
	vy=0;
	vz=0;
	
};

void CObject::force(float x,float y,float z){
	vx += x;
	vy += y;
	vz += z;
}
void CObject::clearForce(){
	vx = 0;
	vy = 0;
	vz = 0;
}


CBullet::CBullet(){
	// Build the broadphase
	broadphase = new btDbvtBroadphase();
	
	// Set up the collision configuration and dispatcher
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	
	// The actual physics solver
	solver = new btSequentialImpulseConstraintSolver;
	
	// The world.
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,-10,0.0));
	
	// Board
	boardShape = new btStaticPlaneShape(btVector3(0.0,0.01,0.0),1);
	boardMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)));
	btRigidBody::btRigidBodyConstructionInfo
		boardRigidBodyCI(0,boardMotionState,boardShape,btVector3(0,0,0));
	boardRigidBody = new btRigidBody(boardRigidBodyCI);
	dynamicsWorld->addRigidBody(boardRigidBody);
	
	
	btScalar mass = 1;
	
	// Set Box
	boxShape = new btBoxShape(btVector3(1.5f, 1.5f, 1.5f));
	boxMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(-5.0,0,0)));
	btVector3 boxInertia(0,0,0);
	boxShape->calculateLocalInertia(mass, boxInertia);
	btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI(0,boxMotionState,boxShape,boxInertia);
	boxRigidBody = new btRigidBody(boxRigidBodyCI);
	dynamicsWorld->addRigidBody(boxRigidBody);
	
	
	
	// Set sphere
	btVector3 fallInertia(0,0,0);
	fallShape = new btSphereShape(0.6);
	fallMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(1.3,20,-1.5)));
	fallShape->calculateLocalInertia(mass,fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,fallShape,fallInertia);
	fallRigidBody = new btRigidBody(fallRigidBodyCI);
	dynamicsWorld->addRigidBody(fallRigidBody);
	
	//Set Cylinder	
	btVector3 cylinderInertia(0,0,0);
	cylinderCollisionShape = new btCylinderShape(btVector3(1.5f, 0.5f, 1.5f));
	cylinderMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
	cylinderCollisionShape->calculateLocalInertia(mass,cylinderInertia);
	btRigidBody::btRigidBodyConstructionInfo cylinderRigidBodyCI(mass,cylinderMotionState,cylinderCollisionShape,cylinderInertia);
	cylinderRigidBody = new btRigidBody(cylinderRigidBodyCI);
	dynamicsWorld->addRigidBody(cylinderRigidBody);
}

CBullet::~CBullet(){
	delete dynamicsWorld;
	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
	
	
	dynamicsWorld->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;
	
	dynamicsWorld->removeRigidBody(boardRigidBody);
}

void CBullet::update(float dt){
	
	dynamicsWorld->stepSimulation(dt,10);
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	cout << numManifolds << endl;

	btTransform s;
	btTransform c;
	btTransform b;
	btTransform bo;
	
	fallRigidBody->getMotionState()->getWorldTransform(s);
	fallRigidBody->applyForce(btVector3(sphere.vx,sphere.vy,sphere.vz),btVector3(50.0,0,0));
	sphere.clearForce();
	
	sphere.x = s.getOrigin().getX();
	sphere.y = s.getOrigin().getY();
	sphere.z = s.getOrigin().getZ();
	
	// Quick Check for boundaries
	if(sphere.x >= 5){ 
		fallRigidBody->setLinearVelocity(btVector3(0,0,0));
		fallRigidBody->clearForces();
		fallRigidBody->applyForce(btVector3(-10,0,0),btVector3(50.0,0,0));
	}
	else if(sphere.x <= -5){
		fallRigidBody->setLinearVelocity(btVector3(0,0,0));
		fallRigidBody->clearForces();
		fallRigidBody->applyForce(btVector3(10,0,0),btVector3(50.0,0,0));
	}
	
	if(sphere.z >= 1.5){
		fallRigidBody->setLinearVelocity(btVector3(0,0,0));
		fallRigidBody->clearForces();
		fallRigidBody->applyForce(btVector3(0,0,-10),btVector3(50.0,0,0));
	}
	else if(sphere.z <= -5){
		fallRigidBody->setLinearVelocity(btVector3(0,0,0));
		fallRigidBody->clearForces();
		fallRigidBody->applyForce(btVector3(0,0,10),btVector3(50.0,0,0));
	}
	
	cylinderRigidBody->getMotionState()->getWorldTransform(c);
	cylinder.x = c.getOrigin().getX();
	cylinder.y = c.getOrigin().getY();
	cylinder.z = c.getOrigin().getZ();
	
	boxRigidBody->getMotionState()->getWorldTransform(b);
	box.x = b.getOrigin().getX();
	box.y = b.getOrigin().getY();
	box.z = b.getOrigin().getZ();
	
	boardRigidBody->getMotionState()->getWorldTransform(bo);
	board.x = bo.getOrigin().getX();
	board.y = bo.getOrigin().getY();
	board.z = bo.getOrigin().getZ();
	
	cout << "sphere: " << sphere.x << " " << sphere.y << " " << sphere.z << endl;
	cout << "cylinder: " << cylinder.x << " " << cylinder.y << " " << cylinder.z << endl;
	cout << "box: " << box.x << " " << box.y << " " << box.z << endl;
	cout << "board: " << board.x << " " << board.y << " " << board.z << endl << endl;
		
}
