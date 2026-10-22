#include "Gcj02.h"

#include "FSCommon.h"
#include "configuration.h"
#include "mesh/generated/meshtastic/mesh.pb.h"
#include <cmath>
#include <pb_decode.h>
#include <pb_encode.h>

namespace
{
constexpr double kPi = 3.14159265358979323846;
constexpr double kSemiMajorAxis = 6378245.0;                 // Krasovsky 1940
constexpr double kEccentricitySquared = 0.00669342162296594323;
constexpr const char *kFlagFile = "/prefs/gcj02";

// -1 = not loaded yet, 0 = off, 1 = on. Read lazily from the flag file.
int8_t gEnabled = -1;

bool readFlagFromDisk()
{
    auto f = FSCom.open(kFlagFile, FILE_O_READ);
    if (!f)
        return false;

    uint8_t value = 0;
    const bool ok = f.read(&value, 1) == 1;
    f.close();
    return ok && value != 0;
}

void writeFlagToDisk(bool on)
{
    // Adafruit_LittleFS opens FILE_O_WRITE at the end of the file, so writing the
    // flag in place would append and leave the first byte (the one we read back)
    // stale. Recreate the file so it always holds exactly one byte.
    FSCom.remove(kFlagFile);
    auto f = FSCom.open(kFlagFile, FILE_O_WRITE);
    if (!f) {
        LOG_WARN("GCJ-02: unable to open %s for write", kFlagFile);
        return;
    }

    const uint8_t value = on ? 1 : 0;
    if (f.write(&value, 1) != 1)
        LOG_WARN("GCJ-02: failed to persist flag");
    f.close();
}

bool inChina(double lat, double lon)
{
    return lon >= 72.004 && lon <= 137.8347 && lat >= 0.8293 && lat <= 55.8271;
}

double transformLatitude(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(fabs(x));
    ret += (20.0 * sin(6.0 * x * kPi) + 20.0 * sin(2.0 * x * kPi)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * kPi) + 40.0 * sin(y / 3.0 * kPi)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * kPi) + 320.0 * sin(y * kPi / 30.0)) * 2.0 / 3.0;
    return ret;
}

double transformLongitude(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(fabs(x));
    ret += (20.0 * sin(6.0 * x * kPi) + 20.0 * sin(2.0 * x * kPi)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * kPi) + 40.0 * sin(x / 3.0 * kPi)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * kPi) + 300.0 * sin(x / 30.0 * kPi)) * 2.0 / 3.0;
    return ret;
}

// Rewrites a 1e-7 degree pair in place. Returns true when the point moved.
bool shiftFixedPoint(int32_t &latE7, int32_t &lonE7)
{
    double lat = latE7 * 1e-7;
    double lon = lonE7 * 1e-7;
    double shiftedLat = lat;
    double shiftedLon = lon;
    if (!gcj02::wgs84ToGcj02(lat, lon, shiftedLat, shiftedLon))
        return false;

    latE7 = static_cast<int32_t>(llround(shiftedLat * 1e7));
    lonE7 = static_cast<int32_t>(llround(shiftedLon * 1e7));
    return true;
}

bool unshiftFixedPoint(int32_t &latE7, int32_t &lonE7)
{
    double lat = latE7 * 1e-7;
    double lon = lonE7 * 1e-7;
    double shiftedLat = lat;
    double shiftedLon = lon;
    if (!gcj02::gcj02ToWgs84(lat, lon, shiftedLat, shiftedLon))
        return false;

    latE7 = static_cast<int32_t>(llround(shiftedLat * 1e7));
    lonE7 = static_cast<int32_t>(llround(shiftedLon * 1e7));
    return true;
}

bool shiftPosition(meshtastic_Position &pos)
{
    if (!pos.has_latitude_i || !pos.has_longitude_i)
        return false;
    if (pos.latitude_i == 0 && pos.longitude_i == 0)
        return false;
    return shiftFixedPoint(pos.latitude_i, pos.longitude_i);
}

bool shiftWaypoint(meshtastic_Waypoint &wp)
{
    bool moved = false;
    if (wp.has_latitude_i && wp.has_longitude_i && (wp.latitude_i != 0 || wp.longitude_i != 0))
        moved |= shiftFixedPoint(wp.latitude_i, wp.longitude_i);

    // The optional rectangular geofence follows the waypoint.
    if (wp.has_bounding_box) {
        auto &box = wp.bounding_box;
        if (box.latitude_south_i != 0 || box.longitude_west_i != 0)
            moved |= shiftFixedPoint(box.latitude_south_i, box.longitude_west_i);
        if (box.latitude_north_i != 0 || box.longitude_east_i != 0)
            moved |= shiftFixedPoint(box.latitude_north_i, box.longitude_east_i);
    }
    return moved;
}

bool unshiftWaypoint(meshtastic_Waypoint &wp)
{
    bool moved = false;
    if (wp.has_latitude_i && wp.has_longitude_i && (wp.latitude_i != 0 || wp.longitude_i != 0))
        moved |= unshiftFixedPoint(wp.latitude_i, wp.longitude_i);

    if (wp.has_bounding_box) {
        auto &box = wp.bounding_box;
        if (box.latitude_south_i != 0 || box.longitude_west_i != 0)
            moved |= unshiftFixedPoint(box.latitude_south_i, box.longitude_west_i);
        if (box.latitude_north_i != 0 || box.longitude_east_i != 0)
            moved |= unshiftFixedPoint(box.latitude_north_i, box.longitude_east_i);
    }
    return moved;
}
} // namespace

namespace gcj02
{
bool enabled()
{
    if (gEnabled < 0)
        gEnabled = readFlagFromDisk() ? 1 : 0;
    return gEnabled == 1;
}

void setEnabled(bool on)
{
    gEnabled = on ? 1 : 0;
    writeFlagToDisk(on);
    LOG_INFO("GCJ-02 output %s", on ? "enabled" : "disabled");
}

bool wgs84ToGcj02(double lat, double lon, double &outLat, double &outLon)
{
    outLat = lat;
    outLon = lon;
    if (!inChina(lat, lon))
        return false;

    double dLat = transformLatitude(lon - 105.0, lat - 35.0);
    double dLon = transformLongitude(lon - 105.0, lat - 35.0);
    const double radLat = lat / 180.0 * kPi;
    double magic = sin(radLat);
    magic = 1 - kEccentricitySquared * magic * magic;
    const double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((kSemiMajorAxis * (1 - kEccentricitySquared)) / (magic * sqrtMagic) * kPi);
    dLon = (dLon * 180.0) / (kSemiMajorAxis / sqrtMagic * cos(radLat) * kPi);

    outLat = lat + dLat;
    outLon = lon + dLon;
    return true;
}

bool gcj02ToWgs84(double lat, double lon, double &outLat, double &outLon)
{
    outLat = lat;
    outLon = lon;
    if (!inChina(lat, lon))
        return false;

    // A few fixed-point iterations are enough for centimetre accuracy.
    double wgsLat = lat;
    double wgsLon = lon;
    for (int i = 0; i < 5; ++i) {
        double forwardLat = wgsLat;
        double forwardLon = wgsLon;
        if (!wgs84ToGcj02(wgsLat, wgsLon, forwardLat, forwardLon))
            break;
        wgsLat += lat - forwardLat;
        wgsLon += lon - forwardLon;
    }

    outLat = wgsLat;
    outLon = wgsLon;
    return true;
}

void transformPacketForPhone(meshtastic_MeshPacket &pkt)
{
    if (!enabled() || pkt.which_payload_variant != meshtastic_MeshPacket_decoded_tag)
        return;

    auto &data = pkt.decoded;
    if (data.portnum == meshtastic_PortNum_POSITION_APP) {
        meshtastic_Position pos = meshtastic_Position_init_zero;
        if (!pb_decode_from_bytes(data.payload.bytes, data.payload.size, &meshtastic_Position_msg, &pos))
            return;
        if (!shiftPosition(pos))
            return;

        pb_ostream_t stream = pb_ostream_from_buffer(data.payload.bytes, sizeof(data.payload.bytes));
        if (pb_encode(&stream, &meshtastic_Position_msg, &pos))
            data.payload.size = stream.bytes_written;
    } else if (data.portnum == meshtastic_PortNum_WAYPOINT_APP) {
        meshtastic_Waypoint wp = meshtastic_Waypoint_init_zero;
        if (!pb_decode_from_bytes(data.payload.bytes, data.payload.size, &meshtastic_Waypoint_msg, &wp))
            return;
        if (!shiftWaypoint(wp))
            return;

        pb_ostream_t stream = pb_ostream_from_buffer(data.payload.bytes, sizeof(data.payload.bytes));
        if (pb_encode(&stream, &meshtastic_Waypoint_msg, &wp))
            data.payload.size = stream.bytes_written;
    }
}

void transformNodeInfo(meshtastic_NodeInfo &info)
{
    if (!enabled() || !info.has_position)
        return;
    shiftPosition(info.position);
}

void transformPacketFromPhone(meshtastic_MeshPacket &pkt)
{
    if (!enabled() || pkt.which_payload_variant != meshtastic_MeshPacket_decoded_tag)
        return;

    // Only waypoints come from the map frame. Phone Position packets originate
    // from CoreLocation/Android LocationManager and are already WGS-84.
    auto &data = pkt.decoded;
    if (data.portnum != meshtastic_PortNum_WAYPOINT_APP)
        return;

    meshtastic_Waypoint wp = meshtastic_Waypoint_init_zero;
    if (!pb_decode_from_bytes(data.payload.bytes, data.payload.size, &meshtastic_Waypoint_msg, &wp))
        return;
    if (!unshiftWaypoint(wp))
        return;

    pb_ostream_t stream = pb_ostream_from_buffer(data.payload.bytes, sizeof(data.payload.bytes));
    if (pb_encode(&stream, &meshtastic_Waypoint_msg, &wp))
        data.payload.size = stream.bytes_written;
}
} // namespace gcj02
