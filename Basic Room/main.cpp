#include <stdio.h>
#include <time.h>
#include "pv/Kinect1.h"
#include "pv/OculusRift.h"
#include "pvmm/windowSystem.h"
#include "pvmm/MidOpenGL.h"
#include "ObjectLoader.h"

#include "btBulletDynamicsCommon.h"

using namespace PV;

ObjectModel* landModel;
ObjectModel* ballModel;
ObjectModel* leftHandModel;
ObjectModel* rightHandModel;

void initQuad()
{
	landModel = new ObjectModel("land.obj");
	ballModel = new ObjectModel("ball.obj");
	leftHandModel = new ObjectModel("arm.obj");
	rightHandModel = new ObjectModel("arm.obj");
}

void handleInput(OculusRift* rift, Kinect1* kinect, Math::vec3 &position, Math::vec3 &rotation, btRigidBody* rightHandRigidBody, btRigidBody* leftHandRigidBody)
{
	if (rift->isConnected())
	{
		rift->Update();
		rotation.x = -rift->GetRotation().x * (float)M_PI / 180.0f;
		rotation.y = -rift->GetRotation().y *(float)M_PI / 180.0f;
		rotation.z = -rift->GetRotation().z *(float)M_PI / 180.0f;
	}

	const NUI_SKELETON_DATA skeleton = kinect->getMainPerson();
	if (skeleton.eTrackingState == NUI_SKELETON_TRACKED)
	{
		position.x = -skeleton.Position.x;
		if (skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] == NUI_SKELETON_POSITION_TRACKED)
		{
			position.y = -skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y;
		}
		position.z = -skeleton.Position.z;

		if (skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] == NUI_SKELETON_POSITION_TRACKED)
		{
			leftHandRigidBody->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z)));
		}
		if (skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] == NUI_SKELETON_POSITION_TRACKED)
		{
			rightHandRigidBody->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z)));
		}
	}

	if ((1 << 16) & GetAsyncKeyState(VK_LEFT))
	{
		rotation.y += 1.0f * (float)M_PI / 180.0f;
	}
	if ((1 << 16) & GetAsyncKeyState(VK_RIGHT))
	{
		rotation.y -= 1.0f * (float)M_PI / 180.0f;
	}
	if ((1 << 16) & GetAsyncKeyState(VK_UP))
	{
		position.x += sin(rotation.y);
		position.z += cos(rotation.y);
	}
	if ((1 << 16) & GetAsyncKeyState(VK_DOWN))
	{
		position.x -= sin(rotation.y);
		position.z -= cos(rotation.y);
	}

}

void drawGLScene(unsigned int program, Math::Matrix<float> perspectiveMatrix, Math::Matrix<float> viewMatrix,
	btRigidBody* ground, btRigidBody* ball, btRigidBody* leftHand, btRigidBody* rightHand)
{
	btTransform trans;
	ground->getMotionState()->getWorldTransform(trans);

	btScalar* scalar = new btScalar[16];
	trans.getOpenGLMatrix(scalar);

	Math::Matrix<float> modelMatrix(4, 4);
	modelMatrix = scalar;
	delete scalar;

	pv_glUseProgram(program);
	Math::Matrix<float> mvp = perspectiveMatrix * viewMatrix * modelMatrix;
	unsigned int mvpLocation = pv_glGetUniformLocation(program, "mvp");
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());

	glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 250.0f / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	landModel->Draw();

	btScalar* scalar2 = new btScalar[16];
	ball->getMotionState()->getWorldTransform(trans);
	trans.getOpenGLMatrix(scalar2);
	modelMatrix = scalar2;
	delete scalar2;
	mvp = perspectiveMatrix * viewMatrix * modelMatrix;
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());
	ballModel->Draw();

	btScalar* scalar3 = new btScalar[16];
	trans = leftHand->getWorldTransform();
	trans.getOpenGLMatrix(scalar3);
	modelMatrix = scalar3;
	delete scalar3;
	mvp = perspectiveMatrix * viewMatrix * modelMatrix;
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());
	leftHandModel->Draw();

	btScalar* scalar4 = new btScalar[16];
	trans = rightHand->getWorldTransform();
	trans.getOpenGLMatrix(scalar4);
	modelMatrix = scalar4;
	delete scalar4;
	mvp = perspectiveMatrix * viewMatrix * modelMatrix;
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());
	rightHandModel->Draw();
}

int main()
{
	btBroadphaseInterface* broadphase = new btDbvtBroadphase();
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -9.8f, 0));

	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

	btCollisionShape* handShape = new btSphereShape(0.5f);
	btDefaultMotionState* handMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 1, 0)));
	btScalar handmass = 1;
	btVector3 handInertia(0, 0, 0);
	handShape->calculateLocalInertia(handmass, handInertia);
	btRigidBody::btRigidBodyConstructionInfo handRigidBodyCI(0, handMotionState, handShape, handInertia);
	btRigidBody* leftHandRigidBody = new btRigidBody(handRigidBodyCI);
	btRigidBody* rightHandRigidBody = new btRigidBody(handRigidBodyCI);

	btCollisionShape* ballShape = new btSphereShape(1);
	btDefaultMotionState* ballMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 15, 1.0f)));
	btScalar mass = 1;
	btVector3 ballInertia(0, 0, 0);
	ballShape->calculateLocalInertia(mass, ballInertia);
	btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(mass, ballMotionState, ballShape, ballInertia);
	btRigidBody* ballRigidBody = new btRigidBody(ballRigidBodyCI);

	Window testWindow;
	Math::Matrix<float> perspectiveMatrix(4, 4);
	Math::Matrix<float> viewMatrix(4, 4);
	Math::Matrix<float> viewOffsetMatrix(4, 4);

	Math::vec3 position = { 0, -2, -2.0f };
	Math::vec3 rotation = { 0, 0, 0 };

	Kinect1* kinect = new Kinect1();
	MSG msg;

	testWindow.create(L"Project Virtua - Basic Room", true, NULL);
	testWindow.setWindowDrawingStateGL();
	testWindow.setVisible(true);

	initMidGL();
	wglSwapIntervalEXT(1);

	glEnable(GL_LINE_SMOOTH);


	OculusRift rift(false, testWindow.renderingContext, testWindow.windowHandle, testWindow.deviceContext);

	initQuad();
	unsigned int program = createShaders("vertexShader.vs", "fragShader.fs");
	createPerspectiveMatrix(perspectiveMatrix, 45.0f, 1280.0f / 800.0f, 0.1f, 1000.0f);

	dynamicsWorld->addRigidBody(groundRigidBody);
	dynamicsWorld->addRigidBody(ballRigidBody);
	dynamicsWorld->addRigidBody(rightHandRigidBody);
	dynamicsWorld->addRigidBody(leftHandRigidBody);

	srand(time(NULL));

	while (1)
	{
		dynamicsWorld->stepSimulation(1 / 60.f, 10);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (abs(ballRigidBody->getWorldTransform().getOrigin().x()) > 80 || abs(ballRigidBody->getWorldTransform().getOrigin().z()) > 80)
		{
			ballRigidBody->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-10.0f + (rand() % 20), 15, -10.0f + (rand() % 20))));
		}
		if ((1 << 16) & GetAsyncKeyState(VK_SPACE))
		{
			ballRigidBody->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-10.0f + (rand() % 20), 15, -10.0f + (rand() % 20))));
		}

		handleInput(&rift, kinect, position, rotation, leftHandRigidBody, rightHandRigidBody);
		createLookAtMatrix(viewMatrix, position, rotation);

		testWindow.MakeCurrentGLContext();
		if (rift.StartRender())
		{
			rift.StartEyeRender(Left, viewOffsetMatrix);
			{
				rift.getPerspectiveMatrix(Left, perspectiveMatrix);
				drawGLScene(program, perspectiveMatrix, (viewOffsetMatrix * viewMatrix), groundRigidBody, ballRigidBody, leftHandRigidBody, rightHandRigidBody);
			}
			rift.EndEyeRender(Left);

			rift.StartEyeRender(Right, viewOffsetMatrix);
			{
				rift.getPerspectiveMatrix(Right, perspectiveMatrix);
				drawGLScene(program, perspectiveMatrix, (viewOffsetMatrix * viewMatrix), groundRigidBody, ballRigidBody, leftHandRigidBody, rightHandRigidBody);
			}
			rift.EndEyeRender(Right);

			pv_glBindFramebuffer(PV_GL_FRAMEBUFFER, 0);
			rift.EndRender();
		}
		else
		{
			pv_glBindFramebuffer(PV_GL_FRAMEBUFFER, 0);
			drawGLScene(program, perspectiveMatrix, viewMatrix, groundRigidBody, ballRigidBody, leftHandRigidBody, rightHandRigidBody);
			testWindow.Update();
		}
	}

	testWindow.destroyGLSystem();
	testWindow.destroy();
	return 0;
}