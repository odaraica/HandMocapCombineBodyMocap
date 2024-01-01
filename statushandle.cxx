#include "statushandle.h"

StatusHandle* StatusHandle::mInstance = 0;
StatusHandle* StatusHandle::One()
{
	if (!mInstance)
	{
		mInstance = new StatusHandle();
	}
	return mInstance;
}

StatusHandle::StatusHandle()
{
}

StatusHandle::~StatusHandle()
{
	mInstance = NULL;
}

void StatusHandle::PopMessage(eMessageType pStatus)
{
	switch (pStatus)
	{
	case eSTATUS_MOCAP_HARDWARE_INIT_ERROR:
		FBMessageBox("Mocap Device Initialize Error:",
			"Create wsadata error,please check Net and Socket",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_INITIALIZING:
		FBMessageBox("Mocap Device Error:",
			"The device is connected, but still initializing",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_NOTCONNECTED:
		FBMessageBox("Mocap Device Error:",
			"The device is not connected",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_NOTGENUINE:
		FBMessageBox("Mocap Device Error:",
			"The device is not a valid Kinect",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_NOTSUPPORTED:
		FBMessageBox("Mocap Device Error:",
			"The device is an unsupported model",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_INSUFFICIENTBANDWIDTH:
		FBMessageBox("Mocap Device Error:",
			"The device is connected to a hub without the necessary bandwidth requirements",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_NOTPOWERED:
		FBMessageBox("Mocap Device Error:",
			"The device is connected, but unpowered",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_NOTREADY:
		FBMessageBox("Mocap Device Error:",
			"There was some other unspecified error",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_SOCKET_CREATE_SUCCESS:
		FBMessageBox("Mocap Device Connect Success:",
			"The connect socket created success",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_REQUEST_SEND_SUCCESS:
		FBMessageBox("Mocap Device Requested Data Success:",
			"The Socket send request message success",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_RESEIVE_SUCCESS:
		FBMessageBox("Mocap Device Receive Data Success:",
			"The Socket receive data success",
			"OK");
		break;
	case eSTATUS_MOCAP_HARDWARE_CLOSE_SEND_SUCCESS:
		FBMessageBox("Mocap Device Close Data Send Success:",
			"The close message of Data Streaming  send   success",
			"OK");
		break;
	case eSTATUS_MOCAP_DEBUG:
		FBMessageBox("DEBUG USE:",
			"This program prosessing here meet the error",
			"OK");
		break;
	case eSTATUS_ONE:
		FBMessageBox("DEBUG USE:",
			"This program prosessing here meet the First Marker",
			"OK");
		break;
	case eSTATUS_TWO:
		FBMessageBox("DEBUG USE:",
			"This program prosessing here meet the Second Marker",
			"OK");
		break;
	case eSTATUS_THERE:
		FBMessageBox("DEBUG USE:",
			"This program prosessing here meet the Third Marker",
			"OK");
		break;
	case eSTATUS_FOUR:
		FBMessageBox("DEBUG USE:",
			"This program prosessing here meet the Fourth Marker",
			"OK");
		break;
	case eSTATUS_FIVE:
		FBMessageBox("DEBUG USE:",
			"This program prosessing here meet the Fifth Marker",
			"OK");
		break;
	default:
		break;
	}
}
