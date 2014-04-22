#ifndef _WORLD_H_
#define _WORLD_H_

#include <pv/MinOpenGL.h>
#include <pvmm/MidOpenGL.h>

#include "ObjectLoader.h"

#include "btBulletDynamicsCommon.h"
#include "BulletWorldImporter/btBulletWorldImporter.h"

class World
{
public:
	World();

	void addObject(const char* name, const char* modelName, const char* physicsFile);

	void setObjectPosition(const char* name, float x, float y, float z);
	void setObjectVelocity(const char* name, float x, float y, float z);
	void setObjectElasticity(const char* name, float elasticity);

	void setPerpsectiveMatrix(PV::Math::Matrix<float>* perspectiveMatrix);
	void setViewMatrix(PV::Math::Matrix<float>* viewMatrix);

	void Update();
	void Draw(unsigned int mvpUniformLocation);
private:
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* physicsWorld;
	btBulletWorldImporter* fileLoader;

	PV::Math::Matrix<float>* perspectiveMatrix;
	PV::Math::Matrix<float>* viewMatrix;

	std::vector<std::string> objects;
	std::map<std::string, btCollisionObject*> collisionObjects;
	std::map<std::string, ObjectModel*> models;
	std::map<std::string, ObjectModel*> modelCache;
	std::map<std::string, PV::Math::Matrix<float>*> modelMatrices;
};

#endif

