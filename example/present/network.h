#ifndef __NETWORK_H__
#define __NETWORK_H__

#define WLAN_DEL_ALL_PROFILES   0xFF

// Application specific status/error codes
typedef enum{
  // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
  LAN_CONNECTION_FAILED = -0x7D0,
  DEVICE_NOT_IN_STATION_MODE = LAN_CONNECTION_FAILED - 1,
  STATUS_CODE_MAX = -0xBB8
} e_AppStatusCodes;

int SmartConfigConnect();

#endif //  __NETWORK_H__
