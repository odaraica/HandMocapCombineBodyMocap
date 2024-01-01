#include <fbsdk/fbsdk.h>

enum eMessageType
{
	// Driver errors
	eSTATUS_MOCAP_HARDWARE_INIT_ERROR,

	// Sensor errors
	eSTATUS_MOCAP_HARDWARE_INITIALIZING,
	eSTATUS_MOCAP_HARDWARE_NOTCONNECTED,
	eSTATUS_MOCAP_HARDWARE_NOTGENUINE,
	eSTATUS_MOCAP_HARDWARE_NOTSUPPORTED,
	eSTATUS_MOCAP_HARDWARE_INSUFFICIENTBANDWIDTH,
	eSTATUS_MOCAP_HARDWARE_NOTPOWERED,
	eSTATUS_MOCAP_HARDWARE_NOTREADY,
	eSTATUS_MOCAP_HARDWARE_SOCKET_CREATE_SUCCESS,
	eSTATUS_MOCAP_HARDWARE_REQUEST_SEND_SUCCESS,
	eSTATUS_MOCAP_HARDWARE_RESEIVE_SUCCESS,
	eSTATUS_MOCAP_HARDWARE_CLOSE_SEND_SUCCESS,
	eSTATUS_MOCAP_DATA_TRANS_FAIL,
	eSTATUS_MOCAP_DATA_GET_FAIL,
	eSTATUS_MOCAP_DATA_OPTION_FAIL,
	eSTATUS_MOCAP_DEBUG,
	eSTATUS_ONE,
	eSTATUS_TWO,
	eSTATUS_THERE,
	eSTATUS_FOUR,
	eSTATUS_FIVE
};

class StatusHandle
{
private:
	static StatusHandle *mInstance;
	StatusHandle();

public:
	static StatusHandle* One();
	~StatusHandle();

public:
	void PopMessage(eMessageType pStatus);
};
