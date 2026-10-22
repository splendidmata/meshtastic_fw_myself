#pragma once

#include "mesh/generated/meshtastic/mesh.pb.h"

namespace gcj02
{
// Device-local switch. Namiji has no screen, no button and no accelerometer, so
// it borrows the official device.double_tap_as_button_press field as the on/off
// carrier: the phone can flip it from Device Configuration, and NodeDB persists
// it with the rest of the config (no extra flag file needed). It is a positive
// boolean that defaults to false, so the conversion is off out of the box and
// the app switch reads the same way as the feature.
bool enabled();

// WGS-84 -> GCJ-02, in degrees. Returns false when the point is outside China
// and the inputs were left untouched.
bool wgs84ToGcj02(double lat, double lon, double &outLat, double &outLon);

// GCJ-02 -> WGS-84, in degrees. Returns false when the point is outside China.
bool gcj02ToWgs84(double lat, double lon, double &outLat, double &outLon);

// In-place rewrite of the Position / Waypoint payload before it is handed to
// the phone. Decoded packets only; encrypted ones are passed through.
void transformPacketForPhone(meshtastic_MeshPacket &pkt);

// In-place rewrite of a Waypoint received from the phone, so the mesh keeps
// WGS-84. Phone Position packets are left alone: the app's location provider
// already reports WGS-84.
void transformPacketFromPhone(meshtastic_MeshPacket &pkt);

// In-place rewrite of a NodeInfo position before it is handed to the phone.
void transformNodeInfo(meshtastic_NodeInfo &info);
} // namespace gcj02
