#include <stdio.h>
#include "pv/OculusRift.h"
#include "pvmm/windowSystem.h"
#include "pvmm/MidOpenGL.h"
#include "pvmm/WavefrontObject.h"

using namespace PV;

WavefrontObject* floorModel;

void handleInput(OculusRift* rift, Math::vec3 &position, Math::vec3 &rotation)
{
	// Rotate based on head orientation if rift is connected.
	if (rift->isConnected())
	{
		rift->Update();
		rotation.x = -rift->GetRotation().x * (float)M_PI / 180.0f;
		rotation.y = -rift->GetRotation().y *(float)M_PI / 180.0f;
		rotation.z = -rift->GetRotation().z *(float)M_PI / 180.0f;
	}
	// Use keyboard keys for rotation otherwise.
	else
	{
		if (65536 & GetAsyncKeyState(VK_LEFT))
		{
			rotation.y += 1.0f * (float)M_PI / 180.0f;
		}
		if (65536 & GetAsyncKeyState(VK_RIGHT))
		{
			rotation.y -= 1.0f * (float)M_PI / 180.0f;
		}
	}

	// Move with keyboard keys.
	if (65536 & GetAsyncKeyState(VK_UP))
	{
		position.x += sin(rotation.y);
		position.z += cos(rotation.y);
	}
	if (65536 & GetAsyncKeyState(VK_DOWN))
	{
		position.x -= sin(rotation.y);
		position.z -= cos(rotation.y);
	}

}

void drawGLScene(unsigned int program, Math::Matrix<float> perspectiveMatrix, Math::Matrix<float> viewMatrix)
{
	// Move floor model below the player.
	Math::Matrix<float> modelMatrix(4, 4);
	modelMatrix.Translate(0.0f, -1.0f, 0.0f);

	// Pass the matrices to the shader.
	pv_glUseProgram(program);
	Math::Matrix<float> mvp = perspectiveMatrix * viewMatrix * modelMatrix;
	unsigned int mvpLocation = pv_glGetUniformLocation(program, "mvp");
	pv_glUniformMatrix4fv(mvpLocation, 1, false, mvp.getArray());

	// Clear the previously rendered scene.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw everything here.
	floorModel->Draw();
}

int main()
{
	//Setup a test window.
	Window testWindow;
	testWindow.create(L"Project Virtua - Basic Rift Only");
	testWindow.setWindowDrawingStateGL();
	testWindow.setVisible(true);

	//Initialize the OpenGL side.
	initMidGL();
	wglSwapIntervalEXT(1);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	//Create OculusRift object, passing the rendering context, device context, and handle of this window
	OculusRift rift(false, testWindow.renderingContext, testWindow.windowHandle, testWindow.deviceContext);

	// Load the test model for the floor.
	floorModel = new WavefrontObject("test.obj");

	//Matrices to handle camera view and warping for OculusRift
	Math::Matrix<float> perspectiveMatrix(4, 4);
	Math::Matrix<float> viewMatrix(4, 4);
	Math::Matrix<float> viewOffsetMatrix(4, 4);

	//Position and rotation of the camera
	Math::vec3 position = { 0, 0, -2.0f };
	Math::vec3 rotation = { 0, 0, 0 };

	// Setup the shaders and perspective matrix
	unsigned int program = createShaders("vertexShader.vs", "fragShader.fs");
	createPerspectiveMatrix(perspectiveMatrix, 45.0f, 1280.0f / 800.0f, 0.1f, 1000.0f);

	// Create the message to handle the window messages.
	MSG msg;
	while (1)
	{
		// Handle window messages.
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

		// Handle the input and lookat matrix creation.
		handleInput(&rift, position, rotation);
		createLookAtMatrix(viewMatrix, position, rotation);

		testWindow.MakeCurrentGLContext();
		//Attempts to render to OculusRift
		if (rift.StartRender())
		{
			//Renders left eye
			rift.StartEyeRender(Left, viewOffsetMatrix);
			{
				rift.getPerspectiveMatrix(Left, perspectiveMatrix);
				drawGLScene(program, perspectiveMatrix, (viewOffsetMatrix * viewMatrix));
			}
			rift.EndEyeRender(Left);

			//Renders right eye
			rift.StartEyeRender(Right, viewOffsetMatrix);
			{
				rift.getPerspectiveMatrix(Right, perspectiveMatrix);
				drawGLScene(program, perspectiveMatrix, (viewOffsetMatrix * viewMatrix));
			}
			rift.EndEyeRender(Right);

			//Frame buffering is not handled automatically!
			pv_glBindFramebuffer(PV_GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			rift.EndRender();
			glEnable(GL_DEPTH_TEST);
			// Ocuus Rift sets clear depth to 0, reset that here.
			glClearDepth(1);
		}
		//Rendering to window if OculusRift fails
		else
		{
			pv_glBindFramebuffer(PV_GL_FRAMEBUFFER, 0);
			drawGLScene(program, perspectiveMatrix, viewMatrix);
			testWindow.Update();
		}
	}

	// Cleanup everything.
	testWindow.destroyGLSystem();
	testWindow.destroy();
	return 0;
}