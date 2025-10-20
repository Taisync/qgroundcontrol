/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VehicleEFIFactGroup.h"
#include "Vehicle.h"

VehicleEFIFactGroup::VehicleEFIFactGroup(QObject *parent)
    : FactGroup(1000, QStringLiteral(":/json/Vehicle/EFIFact.json"), parent)
{
    _addFact(&_rpmFact);
    _addFact(&_fuelConsumedFact);
    _addFact(&_fuelFlowFact);
    _addFact(&_engineLoadFact);
    _addFact(&_cylinderTempFact);

    _healthFact.setRawValue(qQNaN());
    _ecuIndexFact.setRawValue(qQNaN());
    _rpmFact.setRawValue(qQNaN());
    _fuelConsumedFact.setRawValue(qQNaN());
    _fuelFlowFact.setRawValue(qQNaN());
    _engineLoadFact.setRawValue(qQNaN());
    _sparkTimeFact.setRawValue(qQNaN());
    _throttlePosFact.setRawValue(qQNaN());
    _baroPressFact.setRawValue(qQNaN());
    _intakePressFact.setRawValue(qQNaN());
    _intakeTempFact.setRawValue(qQNaN());
    _cylinderTempFact.setRawValue(qQNaN());
    _ignTimeFact.setRawValue(qQNaN());
    _exGasTempFact.setRawValue(qQNaN());
    _injTimeFact.setRawValue(qQNaN());
    _throttleOutFact.setRawValue(qQNaN());
    _ptCompFact.setRawValue(qQNaN());
}

void VehicleEFIFactGroup::handleMessage(Vehicle *vehicle, const mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    case MAVLINK_MSG_ID_EFI_STATUS:
        _handleEFIStatus(message);
        break;
    default:
        break;
    }
}

void VehicleEFIFactGroup::_handleEFIStatus(const mavlink_message_t &message)
{
    mavlink_efi_status_t efi{};
    mavlink_msg_efi_status_decode(&message, &efi);

    rpm()->setRawValue(efi.rpm);
    fuelConsumed()->setRawValue(efi.fuel_consumed);
    fuelFlow()->setRawValue(efi.fuel_flow);
    engineLoad()->setRawValue(efi.engine_load);
    cylinderTemp()->setRawValue(efi.cylinder_head_temperature);

    _setTelemetryAvailable(true);
}
