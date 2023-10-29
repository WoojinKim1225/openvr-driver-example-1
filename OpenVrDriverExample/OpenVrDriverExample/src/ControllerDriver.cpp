#include <ControllerDriver.h>
#include <string.h>
#include <driverlog.h>
#include <vrmath.h>
#include <Windows.h>
#include <SerialPort.h>
#include <iostream>
#include <WbemIdl.h>


static const char* controllerMainSettingsSection = "hellion_controller";

static const char* controllerSettingsSection_r = "hellion_controller_l";
static const char* controllerSettingsSection_l = "hellion-controller_r",
static const char* controllerSettingsKeyModelNumber = "controller_model_number";
static const char* controllerSettingsKeySerialNumber = "controller_serial_number";

ControllerDriver::ControllerDriver(vr::ETrackedControllerRole role) {
	isActive = false;
	controllerRole = role;
	char model_number[1024];
	vr::VRSettings()->GetString(controllerMainSettingsSection, controllerSettingsKeyModelNumber, model_number, sizeof(model_number));
	controllerModelNumber = model_number;

	char serial_number[1024];
	vr::VRSettings()->GetString((controllerRole == vr::TrackedControllerRole_LeftHand ? controllerSettingsSection_l : controllerSettingsSection_r), controllerSettingsKeySerialNumber, serial_number, sizeof(serial_number));
	controllerModelNumber = serial_number;

	DriverLog("My Controller Model Number : %s", controllerModelNumber.c_str());
	DriverLog("My Controller Serial Number : %s", controllerSerialNumber.c_str());
}

vr::EVRInitError ControllerDriver::Activate(uint32_t unObjectId)
{
	isActive = true;
	driverId = unObjectId; //unique ID for your driver

	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(driverId); //this gets a container object where you store all the information about your driver

	vr::VRProperties()->SetStringProperty(container, vr::Prop_ModelNumber_String, controllerModelNumber.c_str());

	vr::VRProperties()->SetStringProperty(container, vr::Prop_InputProfilePath_String, "{hellion}/input/controller_profile.json"); //tell OpenVR where to get your driver's Input Profile
	vr::VRProperties()->SetInt32Property(container, vr::Prop_ControllerRoleHint_Int32, controllerRole); //tells OpenVR what kind of device this is


	//vr::VRDriverInput()->CreateScalarComponent(container, "/input/joystick/y", &joystickYHandle, EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //sets up handler you'll use to send joystick commands to OpenVR with, in the Y direction (forward/backward)
	//vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/y", &trackpadYHandle, EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //sets up handler you'll use to send trackpad commands to OpenVR with, in the Y direction
	//vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &joystickXHandle, EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //Why VRScalarType_Absolute? Take a look at the comments on EVRScalarType.
	//vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/x", &trackpadXHandle, EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //Why VRScalarUnits_NormalizedTwoSided? Take a look at the comments on EVRScalarUnits.
	
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/a/click", &inputHandles[Component_a_click]);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trigger/value", &inputHandles[Component_trigger_value], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/joystick/y", &inputHandles[Component_joystick_y_value], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/joystick/x", &inputHandles[Component_joystick_x_value], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);

	poseUpdateThread = std::thread(&ControllerDriver::PoseUpdateThread, this);

	//The following properites are ones I tried out because I saw them in other samples, but I found they were not needed to get the sample working.
	//There are many samples, take a look at the openvr_header.h file. You can try them out.

	//VRProperties()->SetUint64Property(props, Prop_CurrentUniverseId_Uint64, 2);
	//VRProperties()->SetBoolProperty(props, Prop_HasControllerComponent_Bool, true);
	//VRProperties()->SetBoolProperty(props, Prop_NeverTracked_Bool, true);
	//VRProperties()->SetInt32Property(props, Prop_Axis0Type_Int32, k_eControllerAxis_TrackPad);
	//VRProperties()->SetInt32Property(props, Prop_Axis2Type_Int32, k_eControllerAxis_Joystick);
	//VRProperties()->SetStringProperty(props, Prop_SerialNumber_String, "example_controler_serial");
	//VRProperties()->SetStringProperty(props, Prop_RenderModelName_String, "vr_controller_vive_1_5");
	//uint64_t availableButtons = ButtonMaskFromId(k_EButton_SteamVR_Touchpad) |
	//	ButtonMaskFromId(k_EButton_IndexController_JoyStick);
	//VRProperties()->SetUint64Property(props, Prop_SupportedButtons_Uint64, availableButtons);

	return vr::VRInitError_None;
}


void ControllerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
	if (unResponseBufferSize >= 1) {
		pchResponseBuffer[0] = 0;
	}
}

vr::DriverPose_t ControllerDriver::GetPose()
{
	//Initialize the struct that we'll be submitting to the runtime to tell it we've updated our pose.
	vr::DriverPose_t pose = { 0 };

	pose.qWorldFromDriverRotation.w = 1.f;
	pose.qWorldFromDriverRotation.w = 1.f;

	vr::TrackedDevicePose_t hmdPose{};

	pose.poseIsValid = true;
	pose.deviceIsConnected = true;

	vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, &hmdPose, 1);

	// have to fix tis part using vector3.
	const vr::HmdVector3_t hmd_positionOS = HmdVector3_From34Matrix(hmdPose.mDeviceToAbsoluteTracking);
	// have to fix this part using quaternions.
	const vr::HmdQuaternion_t hmd_rotationOS = HmdQuaternion_FromMatrix(hmdPose.mDeviceToAbsoluteTracking);

	const vr::HmdVector3_t positionOffset;
	const vr::HmdQuaternion_t rotationOffset;

	const vr::HmdVector3_t hmd_positionWS = hmd_positionOS + (positionOffset * hmd_rotationOS);
	const vr::HmdQuaternion_t hmd_rotationWS = hmd_rotationOS * rotationOffset;
	
	for (int i = 0; i < 3; i++) {
		pose.vecPosition[i] = hmd_positionWS.v[i];
	}
	pose.qRotation = hmd_rotationWS;
	

	pose.result = vr::TrackingResult_Running_OK;

	return pose;
}

void ControllerDriver::PoseUpdateThread() {
	while (isActive) {
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(driverId, GetPose(), sizeof(vr::DriverPose_t));
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void ControllerDriver::RunFrame() {
	ReadSerial();
	//Since we used VRScalarUnits_NormalizedTwoSided as the unit, the range is -1 to 1.
	vr::VRDriverInput()->UpdateScalarComponent(inputHandles[Component_trigger_value], triggerValue, 0.f);
	vr::VRDriverInput()->UpdateBooleanComponent(inputHandles[Component_a_click], aValue, 0.f);
	vr::VRDriverInput()->UpdateScalarComponent(inputHandles[Component_joystick_x_value], joystickValue[0], 0.f);
	vr::VRDriverInput()->UpdateScalarComponent(inputHandles[Component_joystick_y_value], joystickValue[1], 0.f);
}

void ControllerDriver::Deactivate()
{
	if (isActive.exchange(false)) {
		poseUpdateThread.join();
	}
	driverId = vr::k_unTrackedDeviceIndexInvalid;
}

void* ControllerDriver::GetComponent(const char* pchComponentNameAndVersion)
{
	//I found that if this method just returns null always, it works fine. But I'm leaving the if statement in since it doesn't hurt.
	//Check out the IVRDriverInput_Version declaration in openvr_driver.h. You can search that file for other _Version declarations 
	//to see other components that are available. You could also put a log in this class and output the value passed into this 
	//method to see what OpenVR is looking for.
	if (strcmp(vr::IVRDriverInput_Version, pchComponentNameAndVersion) == 0)
	{
		return this;
	}
	return NULL;
}

void ControllerDriver::EnterStandby() {}

void ControllerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) 
{
	if (unResponseBufferSize >= 1)
	{
		pchResponseBuffer[0] = 0;
	}
}

SerialPort* pi;
char receivedData[255];
char savedData[511];

void ReadSerial() {
	int readResult = pi->readSerialPort(receivedData, 255);
	vr::VRDriverLog()->Log(receivedData);

	strcat_s(savedData, sizeof(savedData), receivedData);




}

int charlen(char* p) {
	int count = 0;
	if (p != nullptr) {
		while (*p != '\0')
		{
			count++;
			p++;
		}

		return count;
	}
}

char* safeStrcat(char* dest, char* origin) {
	int destSize = sizeof(dest) / sizeof(*dest);
	int originSize = sizeof(origin) / sizeof(*origin);

}