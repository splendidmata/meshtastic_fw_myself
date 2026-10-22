#pragma once

#include "mesh/generated/meshtastic/mesh.pb.h"

namespace gcj02
{
// Device-local switch: rewrite positions sent to the phone as GCJ-02.
bool enabled();
void setEnabled(bool on);

// WGS-84 -> GCJ-02, in degrees. Returns false when the point is outside China
// and the inputs were left untouched.
bool wgs84ToGcj02(double lat, double lon, double &outLat, double &outLon);

// GCJ-02 -> WGS-84, in degrees. Returns false when the point is outside China.
bool gcj02ToWgs84(double lat, double lon, double &outLat, double &outLon);

// In-place rewrite of the Position / Waypoint payload before it is handed to
// the phone. Decoded packets only; encrypted ones are passed through.
void transformPacketForPhone(meshtastic_MeshPacket &pkt);

// In-place rewrite of a Waypoint received from the phone, so the mesh keeps
// WGS-84. Phone Position packets are left alone: CoreLocation is WGS-84.
void transformPacketFromPhone(meshtastic_MeshPacket &pkt);

// In-place rewrite of a NodeInfo position before it is handed to the phone.
void transformNodeInfo(meshtastic_NodeInfo &info);
} // namespace gcj02
