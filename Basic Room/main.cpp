#include <stdio.h>
#include <time.h>
#include "pv/Kinect1.h"
#include "pv/OculusRift.h"
#include "pvmm/windowSystem.h"
#include "pvmm/MidOpenGL.h"
#include "ObjectLoader.h"

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

void handleInput(OculusRift* rift, Kinect1* kinect, Math::vec3 &position, Math::vec3 &rotation)
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

void drawGLScene(unsigned int program, Math::Matrix<float> perspectiveMatrix, Math::Matrix<float> viewMatrix)
{
	pv_glUseProgram(program);
	Math::Matrix<float> mvp = perspectiveMatrix * viewMatrix;
	unsigned int mvpLocation = pv_glGetUniformLocation(program, "mvp");
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());

	glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 250.0f / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	landModel->Draw();

	mvp = perspectiveMatrix * viewMatrix;
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());
	ballModel->Draw();

	mvp = perspectiveMatrix * viewMatrix;
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());
	leftHandModel->Draw();

	mvp = perspectiveMatrix * viewMatrix;
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());
	rightHandModel->Draw();
}

int main()
{
	Window testWindow;
	Math::Matrix<float> perspectiveMatrix(4, 4);
	Math::Matrix<float> viewMatrix(4, 4);
	Math::Matrix<float> viewOffsetMatrix(4, 4);

	Math::vec3 position = { 0, -2, -8.0f };
	Math::vec3 rotation = { 0, 0, 0 };

	Kinect1* kinect = new Kinect1();
	MSG msg;

	testWindow.create(L"Project Virtua - Basic Room");
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

	while (1)
	{
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

		handleInput(&rift, kinect, position, rotation);
		createLookAtMatrix(viewMatrix, position, rotation);

		testWindow.MakeCurrentGLContext();
		if (rift.StartRender())
		{
			rift.StartEyeRender(Left, viewOffsetMatrix);
			{
				rift.getPerspectiveMatrix(Left, perspectiveMatrix);
				drawGLScene(program, perspectiveMatrix, (viewOffsetMatrix * viewMatrix));
			}
			rift.EndEyeRender(Left);

			rift.StartEyeRender(Right, viewOffsetMatrix);
			{
				rift.getPerspectiveMatrix(Right, perspectiveMatrix);
				drawGLScene(program, perspectiveMatrix, (viewOffsetMatrix * viewMatrix));
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
			drawGLScene(program, perspectiveMatrix, viewMatrix);
			testWindow.Update();
		}
	}

	testWindow.destroyGLSystem();
	testWindow.destroy();
	return 0;
}