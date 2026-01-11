/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VehicleDistanceSensorFactGroup.h"
#include "Vehicle.h"

#include <cmath>

VehicleDistanceSensorFactGroup::VehicleDistanceSensorFactGroup(QObject *parent)
    : FactGroup(1000, QStringLiteral(":/json/Vehicle/DistanceSensorFact.json"), parent)
{
    _addFact(&_rotationNoneFact);
    _addFact(&_rotationYaw45Fact);
    _addFact(&_rotationYaw90Fact);
    _addFact(&_rotationYaw135Fact);
    _addFact(&_rotationYaw180Fact);
    _addFact(&_rotationYaw225Fact);
    _addFact(&_rotationYaw270Fact);
    _addFact(&_rotationYaw315Fact);
    _addFact(&_rotationPitch90Fact);
    _addFact(&_rotationPitch270Fact);
    _addFact(&_minDistanceFact);
    _addFact(&_maxDistanceFact);

    // Initialize all distance values to NaN (displays as "--.--")
    const double nan = std::numeric_limits<double>::quiet_NaN();
    _rotationNoneFact.setRawValue(nan);
    _rotationYaw45Fact.setRawValue(nan);
    _rotationYaw90Fact.setRawValue(nan);
    _rotationYaw135Fact.setRawValue(nan);
    _rotationYaw180Fact.setRawValue(nan);
    _rotationYaw225Fact.setRawValue(nan);
    _rotationYaw270Fact.setRawValue(nan);
    _rotationYaw315Fact.setRawValue(nan);
    _rotationPitch90Fact.setRawValue(nan);
    _rotationPitch270Fact.setRawValue(nan);

    // Setup staleness check timer
    connect(&_stalenessCheckTimer, &QTimer::timeout, this, &VehicleDistanceSensorFactGroup::_checkForStaleValues);
    _stalenessCheckTimer.start(500);  // Check every 500ms
}

void VehicleDistanceSensorFactGroup::handleMessage(Vehicle *vehicle, const mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    if (message.msgid != MAVLINK_MSG_ID_DISTANCE_SENSOR) {
        return;
    }

    mavlink_distance_sensor_t distanceSensor{};
    mavlink_msg_distance_sensor_decode(&message, &distanceSensor);

    struct orientation2Fact_s {
        MAV_SENSOR_ORIENTATION orientation;
        Fact *fact;
        QElapsedTimer *lastUpdate;
    };

    orientation2Fact_s rgOrientation2Fact[] = {
        { MAV_SENSOR_ROTATION_NONE, rotationNone(), &_rotationNoneLastUpdate },
        { MAV_SENSOR_ROTATION_YAW_45, rotationYaw45(), &_rotationYaw45LastUpdate },
        { MAV_SENSOR_ROTATION_YAW_90, rotationYaw90(), &_rotationYaw90LastUpdate },
        { MAV_SENSOR_ROTATION_YAW_135, rotationYaw135(), &_rotationYaw135LastUpdate },
        { MAV_SENSOR_ROTATION_YAW_180, rotationYaw180(), &_rotationYaw180LastUpdate },
        { MAV_SENSOR_ROTATION_YAW_225, rotationYaw225(), &_rotationYaw225LastUpdate },
        { MAV_SENSOR_ROTATION_YAW_270, rotationYaw270(), &_rotationYaw270LastUpdate },
        { MAV_SENSOR_ROTATION_YAW_315, rotationYaw315(), &_rotationYaw315LastUpdate },
        { MAV_SENSOR_ROTATION_PITCH_90, rotationPitch90(), &_rotationPitch90LastUpdate },
        { MAV_SENSOR_ROTATION_PITCH_270, rotationPitch270(), &_rotationPitch270LastUpdate },
    };

    for (orientation2Fact_s &orientation2Fact : rgOrientation2Fact) {
        if (orientation2Fact.orientation == distanceSensor.orientation) {
            orientation2Fact.fact->setRawValue(distanceSensor.current_distance / 100.0); // cm to meters
            orientation2Fact.lastUpdate->restart();  // Reset the staleness timer
            break;
        }
    }

    maxDistance()->setRawValue(distanceSensor.max_distance / 100.0);

    _setTelemetryAvailable(true);
}

void VehicleDistanceSensorFactGroup::_checkForStaleValues()
{
    const double nan = std::numeric_limits<double>::quiet_NaN();

    struct factTimer_s {
        Fact *fact;
        QElapsedTimer *lastUpdate;
    };

    factTimer_s rgFactTimers[] = {
        { rotationNone(), &_rotationNoneLastUpdate },
        { rotationYaw45(), &_rotationYaw45LastUpdate },
        { rotationYaw90(), &_rotationYaw90LastUpdate },
        { rotationYaw135(), &_rotationYaw135LastUpdate },
        { rotationYaw180(), &_rotationYaw180LastUpdate },
        { rotationYaw225(), &_rotationYaw225LastUpdate },
        { rotationYaw270(), &_rotationYaw270LastUpdate },
        { rotationYaw315(), &_rotationYaw315LastUpdate },
        { rotationPitch90(), &_rotationPitch90LastUpdate },
        { rotationPitch270(), &_rotationPitch270LastUpdate },
    };

    for (const factTimer_s &factTimer : rgFactTimers) {
        // If timer is valid (has been started) and has exceeded timeout, mark as stale
        if (factTimer.lastUpdate->isValid() && factTimer.lastUpdate->elapsed() > _stalenessTimeoutMs) {
            // Only set to NaN if not already NaN (avoid unnecessary signal emissions)
            if (!std::isnan(factTimer.fact->rawValue().toDouble())) {
                factTimer.fact->setRawValue(nan);
            }
        }
    }
}
