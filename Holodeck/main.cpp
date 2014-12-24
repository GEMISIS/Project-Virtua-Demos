#include <stdio.h>
#include <time.h>
#include "pv/Kinect1.h"
#include "pv/OculusRift.h"
#include "pvmm/windowSystem.h"
#include "pvmm/MidOpenGL.h"
#include "ObjectLoader.h"
#include "World.h"

#include "btBulletDynamicsCommon.h"

using namespace PV;

void initQuad()
{
}

void handleInput(World* world, OculusRift* rift, Kinect1* kinect, Math::vec3 &position, Math::vec3 &rotation)
{
	if (rift->isConnected())
	{
		rift->Update();
	}

	const NUI_SKELETON_DATA skeleton = kinect->getMainPerson();
	if (kinect->Status() == PV::KinectStatus::Ready)
	{
		if (skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] == NUI_SKELETON_POSITION_TRACKED)
		{
			position.x = skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HEAD].x;
			position.y = -skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y - 0.5f;
			position.z = skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HEAD].z;
		}
		if (skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] == NUI_SKELETON_POSITION_INFERRED || skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] == NUI_SKELETON_POSITION_TRACKED)
		{
			world->setObjectPosition("leftHand",
				-skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y + 0.5f,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z);
		}
		if (skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] == NUI_SKELETON_POSITION_INFERRED || skeleton.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] == NUI_SKELETON_POSITION_TRACKED)
		{
			world->setObjectPosition("rightHand",
				-skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y + 0.5f,
				skeleton.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z);
		}
	}

	if ((1 << 16) & GetAsyncKeyState(VK_LEFT))
	{
		if (rift->isConnected())
		{
			rift->DismissWarningScreen();
		}
		rotation.y += 1.0f * (float)M_PI / 180.0f;
	}
	if ((1 << 16) & GetAsyncKeyState(VK_RIGHT))
	{
		if (rift->isConnected())
		{
			rift->DismissWarningScreen();
		}
		rotation.y -= 1.0f * (float)M_PI / 180.0f;
	}
	if ((1 << 16) & GetAsyncKeyState(VK_SPACE))
	{
		if (rift->isConnected())
		{
			rift->DismissWarningScreen();
		}
		position.y += 0.1f;
	}
	if ((1 << 16) & GetAsyncKeyState(VK_RETURN))
	{
		if (rift->isConnected())
		{
			rift->DismissWarningScreen();
		}
		position.y -= 0.1f;
	}
	if ((1 << 16) & GetAsyncKeyState(VK_UP))
	{
		if (rift->isConnected())
		{
			rift->DismissWarningScreen();
		}
		position.x += sin(rotation.y);
		position.z += cos(rotation.y);
	}
	if ((1 << 16) & GetAsyncKeyState(VK_DOWN))
	{
		if (rift->isConnected())
		{
			rift->DismissWarningScreen();
		}
		position.x -= sin(rotation.y);
		position.z -= cos(rotation.y);
	}

}

void drawGLScene(unsigned int program, World* world)
{
	pv_glUseProgram(program);
	unsigned int mvpLocation = pv_glGetUniformLocation(program, "mvp");

	glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 250.0f / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	world->Draw(mvpLocation);
}

int main()
{
	InitRift();
	World* world = new World();

	Window testWindow;
	Math::Matrix<float> perspectiveMatrix(4, 4);
	Math::Matrix<float> viewMatrix(4, 4);
	Math::Matrix<float> viewOffsetMatrix(4, 4);

	Math::vec3 position = { 0, -2, 5.0f };
	Math::vec3 rotation = { 0, M_PI, 0 };

	Kinect1* kinect = new Kinect1();
	MSG msg;

	testWindow.create(L"Project Virtua - Holodeck");
	testWindow.SetFullscreen(true);
	testWindow.setWindowDrawingStateGL();
	testWindow.setVisible(true);

	initMidGL();
	wglSwapIntervalEXT(1);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	OculusRift rift(false, testWindow.renderingContext, testWindow.windowHandle, testWindow.deviceContext);

	initQuad();
	unsigned int program = createShaders("vertexShader.vs", "fragShader.fs");
	createPerspectiveMatrix(perspectiveMatrix, 45.0f, 1280.0f / 800.0f, 0.1f, 1000.0f);

	srand(time(NULL));

	world->addObject("ground", "room.obj", "test2.bullet");
	world->addObject("box", "box.obj", "box.bullet");
	world->setObjectPosition("box", 2, 20, 0);
	world->addObject("leftHand", "hand.obj", "hand.bullet");
	world->setObjectPosition("leftHand", 2, -200, 0);
	world->addObject("rightHand", "hand.obj", "hand.bullet");
	world->setObjectPosition("rightHand", -2, -200, 0);
	/*
	for (long double i = 0; i < 10; i += 1)
	{
		std::string name = "ball";
		std::string temp = std::to_string(i);
		name.append(temp);
		world->addObject(name.c_str(), "test.obj", "test.bullet");
		world->setObjectPosition(name.c_str(), 0, 1
			, rand() % 2);
		world->setObjectElasticity(name.c_str(), 0.75f);
	}
	*/

	while (1)
	{
		world->Update();

		if ((1 << 16) & GetAsyncKeyState(VK_BACK))
		{
			rift.DismissWarningScreen();
			world->setObjectVelocity("box", 1, 1, 1);
		}
		if ((1 << 16) & GetAsyncKeyState(VK_DELETE))
		{
			rift.DismissWarningScreen();
			world->setObjectVelocity("box", -1, -1, -1);
		}

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

		handleInput(world, &rift, kinect, position, rotation);
		createLookAtMatrix(viewMatrix, position, rotation);

		testWindow.MakeCurrentGLContext();
		if (rift.StartRender())
		{
			rift.StartEyeRender(Left, viewOffsetMatrix);
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				rift.getPerspectiveMatrix(Left, perspectiveMatrix);
				world->setPerpsectiveMatrix(&perspectiveMatrix);
				world->setViewMatrix(&(viewOffsetMatrix * viewMatrix));
				drawGLScene(program, world);
			}
			rift.EndEyeRender(Left);

			rift.StartEyeRender(Right, viewOffsetMatrix);
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				rift.getPerspectiveMatrix(Right, perspectiveMatrix);
				world->setPerpsectiveMatrix(&perspectiveMatrix);
				world->setViewMatrix(&(viewOffsetMatrix * viewMatrix));
				drawGLScene(program, world);
			}
			rift.EndEyeRender(Right);

			pv_glBindFramebuffer(PV_GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			rift.EndRender();
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1);
		}
		else
		{
			pv_glBindFramebuffer(PV_GL_FRAMEBUFFER, 0);
			world->setPerpsectiveMatrix(&perspectiveMatrix);
			world->setViewMatrix(&viewMatrix);
			drawGLScene(program, world);
			testWindow.Update();
		}
	}

	testWindow.destroyGLSystem();
	testWindow.destroy();
	return 0;
}