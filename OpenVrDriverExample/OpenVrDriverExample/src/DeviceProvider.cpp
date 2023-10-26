#include <DeviceProvider.h>
#include <driverlog.h>

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext)
{
    vr::EVRInitError initError = InitServerDriverContext(pDriverContext);
    //if (initError != vr::EVRInitError::VRInitError_None)
    //{
    //    return initError;
    //}
    
    //vr::VRDriverLog()->Log("Initializing example controller"); //this is how you log out Steam's log file.

    //controllerDriver = new ControllerDriver();
    //VRServerDriverHost()->TrackedDeviceAdded("example_controller", TrackedDeviceClass_Controller, controllerDriver); //add all your devices like this.

    //feetControllerDevice_l = std::make_unique<ControllerDriver>(vr::TrackedControllerRole_LeftHand);
    //feetControllerDevice_r = std::make_unique<ControllerDriver>(vr::TrackedControllerRole_RightHand);

    if (!vr::VRServerDriverHost()->TrackedDeviceAdded("feetControllerDevice_l", vr::TrackedDeviceClass_Controller, feetControllerDevice_l.get())) {
        DriverLog("Cannot create left device!");
        return vr::VRInitError_Driver_Unknown;
    }

    if (!vr::VRServerDriverHost()->TrackedDeviceAdded("feetControllerDevice_r", vr::TrackedDeviceClass_Controller, feetControllerDevice_r.get())) {
        DriverLog("Cannot create right device!");
        return vr::VRInitError_Driver_Unknown;
    }

    return vr::VRInitError_None;
}

void DeviceProvider::Cleanup()
{
    feetControllerDevice_l = nullptr;
    feetControllerDevice_r = nullptr; 
}
const char* const* DeviceProvider::GetInterfaceVersions()
{
    return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame()
{
    if (feetControllerDevice_l != nullptr) {
        feetControllerDevice_l->RunFrame();
    }
    if (feetControllerDevice_r != nullptr) {
        feetControllerDevice_r->RunFrame();
    }
}

bool DeviceProvider::ShouldBlockStandbyMode()
{
    return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}