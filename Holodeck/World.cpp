#include "World.h"

World::World()
{
	this->broadphase = new btDbvtBroadphase();
	this->collisionConfiguration = new btDefaultCollisionConfiguration();
	this->collisionDispatcher = new btCollisionDispatcher(this->collisionConfiguration);
	this->solver = new btSequentialImpulseConstraintSolver();
	this->physicsWorld = new btDiscreteDynamicsWorld(this->collisionDispatcher, this->broadphase, this->solver, this->collisionConfiguration);
	this->physicsWorld->setGravity(btVector3(0, -9.81f, 0));

	this->fileLoader = new btBulletWorldImporter(this->physicsWorld);
	//this->fileLoader->setVerboseMode(true);
}

void World::addObject(const char* name, const char* modelName, const char* physicsFile)
{
	this->fileLoader->loadFile(physicsFile);

	this->objects.push_back(name);
	if (this->modelCache.find(modelName) == this->modelCache.end())
	{
		this->modelCache.insert(std::pair<std::string, ObjectModel*>(modelName, new ObjectModel(modelName)));
	}
	this->models.insert(std::pair<std::string, ObjectModel*>(name, this->modelCache[modelName]));
	this->collisionObjects.insert(std::pair<std::string, btCollisionObject*>(name, this->fileLoader->getRigidBodyByIndex(this->fileLoader->getNumRigidBodies() - 1)));
	this->modelMatrices.insert(std::pair<std::string, PV::Math::Matrix<float>*>(name, new PV::Math::Matrix<float>(4, 4)));
}

void World::setObjectPosition(const char* name, float x, float y, float z)
{
	this->collisionObjects[name]->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, z, y)));
}

void World::setObjectVelocity(const char* name, float x, float y, float z)
{
	((btRigidBody*)this->collisionObjects[name])->setLinearVelocity(btVector3(x, z, y));
}

void World::setObjectElasticity(const char* name, float elasticity)
{
	this->collisionObjects[name]->setRestitution(elasticity);
}

void World::setPerpsectiveMatrix(PV::Math::Matrix<float>* perspectiveMatrix)
{
	this->perspectiveMatrix = perspectiveMatrix;
}

void World::setViewMatrix(PV::Math::Matrix<float>* viewMatrix)
{
	this->viewMatrix = viewMatrix;
}

void World::Update()
{
	this->physicsWorld->stepSimulation(1 / 60.0f, 10);

	for (int i = 0; i < this->objects.size(); i += 1)
	{
		btScalar* scalar = new btScalar[16];
		btTransform transform = this->collisionObjects[this->objects[i]]->getWorldTransform();
		transform.setOrigin(btVector3(transform.getOrigin().x(), transform.getOrigin().z(), -transform.getOrigin().y()));
		transform.setRotation(btQuaternion(transform.getRotation().x(), transform.getRotation().z(), -transform.getRotation().y(), transform.getRotation().w()));
		transform.getOpenGLMatrix(scalar);
		*this->modelMatrices[this->objects[i]] = scalar;
		delete scalar;
	}
}

void World::Draw(unsigned int mvpUniformLocation)
{
	for (int i = 0; i < this->models.size(); i += 1)
	{
		PV::Math::Matrix<float> mvp(4, 4);
		mvp = *this->perspectiveMatrix * *this->viewMatrix * *this->modelMatrices[this->objects[i]];
		PV::pv_glUniformMatrix4fv(mvpUniformLocation, 1, false, mvp.getArray());
		this->models[this->objects[i]]->Draw();
	}
}
