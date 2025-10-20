/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VehicleGeneratorFactGroup.h"
#include "Vehicle.h"

VehicleGeneratorFactGroup::VehicleGeneratorFactGroup(QObject *parent)
    : FactGroup(1000, QStringLiteral(":/json/Vehicle/GeneratorFact.json"), parent)
{
    _addFact(&_batteryCurrentFact);
    _addFact(&_loadCurrentFact);
    _addFact(&_powerGeneratedFact);
    _addFact(&_busVoltageFact);

    _statusFact.setRawValue(qQNaN());
    _genSpeedFact.setRawValue(qQNaN());
    _batteryCurrentFact.setRawValue(qQNaN());
    _loadCurrentFact.setRawValue(qQNaN());
    _powerGeneratedFact.setRawValue(qQNaN());
    _busVoltageFact.setRawValue(qQNaN());
    _batCurrentSetpointFact.setRawValue(qQNaN());
    _rectifierTempFact.setRawValue(qQNaN());
    _genTempFact.setRawValue(qQNaN());
    _runtimeFact.setRawValue(qQNaN());
    _timeMaintenanceFact.setRawValue(qQNaN());

    (void) connect(status(), &Fact::rawValueChanged, this,& VehicleGeneratorFactGroup::_updateGeneratorFlags);
}

void VehicleGeneratorFactGroup::handleMessage(Vehicle *vehicle, const mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    case MAVLINK_MSG_ID_GENERATOR_STATUS:
        _handleGeneratorStatus(message);
        break;
    default:
        break;
    }
}

void VehicleGeneratorFactGroup::_handleGeneratorStatus(const mavlink_message_t &message)
{
    mavlink_generator_status_t generator{};
    mavlink_msg_generator_status_decode(&message, &generator);

    status()->setRawValue((generator.status == UINT16_MAX) ? qQNaN() : generator.status);
    batteryCurrent()->setRawValue(generator.battery_current);
    loadCurrent()->setRawValue(generator.load_current);
    powerGenerated()->setRawValue(generator.power_generated);
    busVoltage()->setRawValue(generator.bus_voltage);

    _setTelemetryAvailable(true);
}

void VehicleGeneratorFactGroup::_updateGeneratorFlags(const QVariant &value)
{
    const int statusFlag = value.toInt();
    if (statusFlag == _prevFlag) {
        return;
    }

    _prevFlag = statusFlag;
    _flagsListGenerator.clear();

    const QBitArray bitsetFlags(23);
    for (qsizetype i = 0; i < bitsetFlags.size(); i++) {
        if (bitsetFlags[i]) {
            _flagsListGenerator.append(1);
        } else {
            _flagsListGenerator.append(0);
        }
    }

    emit flagsListGeneratorChanged();
}
