#pragma once
#include <openvr_driver.h>
#include <windows.h>
#include <atomic>
#include <array>
#include <thread>

enum Component {
	Component_a_click,
	Component_trigger_value,
	Component_joystick_x_value,
	Component_joystick_y_value,
	Component_MAX
};

/**
This class controls the behavior of the controller. This is where you 
tell OpenVR what your controller has (buttons, joystick, trackpad, etc.).
This is also where you inform OpenVR when the state of your controller 
changes (for example, a button is pressed).

For the methods, take a look at the comment blocks for the ITrackedDeviceServerDriver 
class too. Those comment blocks have some good information.

This example driver will simulate a controller that has a joystick and trackpad on it.
It is hardcoded to just return a value for the joystick and trackpad. It will cause 
the game character to move forward constantly.
**/
class ControllerDriver : public vr::ITrackedDeviceServerDriver
{
public:
	ControllerDriver(vr::ETrackedControllerRole role);
	/**
	Initialize your controller here. Give OpenVR information 
	about your controller and set up handles to inform OpenVR when 
	the controller state changes.
	**/
	vr::EVRInitError Activate(uint32_t unObjectId);

	/**
	Un-initialize your controller here.
	**/
	void Deactivate();

	/**
	Tell your hardware to go into stand-by mode (low-power).
	**/
	void EnterStandby();

	/**
	Take a look at the comment block for this method on ITrackedDeviceServerDriver. So as far 
	as I understand, driver classes like this one can implement lots of functionality that 
	can be categorized into components. This class just acts as an input device, so it will 
	return the IVRDriverInput class, but it could return other component classes if it had 
	more functionality, such as maybe overlays or UI functionality.
	**/
	void* GetComponent(const char* pchComponentNameAndVersion);

	/**
	Refer to ITrackedDeviceServerDriver. I think it sums up what this does well.
	**/
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);

	/**
	Returns the Pose for your device. Pose is an object that contains the position, rotation, velocity, 
	and angular velocity of your device.
	**/
	vr::DriverPose_t GetPose();

	/**
	You can retrieve the state of your device here and update OpenVR if anything has changed. This 
	method should be called every frame.
	**/
	void RunFrame();

	void PoseUpdateThread();

private:

	uint32_t driverId;
	
	float triggerValue;
	bool aValue;
	float joystickValue[2];

	vr::ETrackedControllerRole controllerRole;
	std::string controllerModelNumber;
	std::string controllerSerialNumber;
	std::array < vr::VRInputComponentHandle_t, Component_MAX > inputHandles;
	std::atomic< bool > isActive;
	std::thread poseUpdateThread;
};