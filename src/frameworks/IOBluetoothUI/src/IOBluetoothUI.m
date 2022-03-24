/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <IOBluetoothUI/IOBluetoothUI.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* BluetoothPreferencesChanged(void)
{
    if (verbose) puts("STUB: BluetoothPreferencesChanged called");
    return NULL;
}

void* DeviceControllerHCIControllerNotificationHandler(void)
{
    if (verbose) puts("STUB: DeviceControllerHCIControllerNotificationHandler called");
    return NULL;
}

void* IOBluetoothAlertHCIControllerPublished(void)
{
    if (verbose) puts("STUB: IOBluetoothAlertHCIControllerPublished called");
    return NULL;
}

void* IOBluetoothAlertHCIPowerStateOn(void)
{
    if (verbose) puts("STUB: IOBluetoothAlertHCIPowerStateOn called");
    return NULL;
}

void* IOBluetoothDecodeCommand(void)
{
    if (verbose) puts("STUB: IOBluetoothDecodeCommand called");
    return NULL;
}

void* IOBluetoothDecodeEvent(void)
{
    if (verbose) puts("STUB: IOBluetoothDecodeEvent called");
    return NULL;
}

void* IOBluetoothDecodeGroup(void)
{
    if (verbose) puts("STUB: IOBluetoothDecodeGroup called");
    return NULL;
}

void* IOBluetoothDecodeManufacturerName(void)
{
    if (verbose) puts("STUB: IOBluetoothDecodeManufacturerName called");
    return NULL;
}

void* IOBluetoothDecodeStatus(void)
{
    if (verbose) puts("STUB: IOBluetoothDecodeStatus called");
    return NULL;
}

void* IOBluetoothDeviceSelectorRunPanelWithAttributes(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceSelectorRunPanelWithAttributes called");
    return NULL;
}

void* IOBluetoothGetDefaultNameOfL2CAPPSMString(void)
{
    if (verbose) puts("STUB: IOBluetoothGetDefaultNameOfL2CAPPSMString called");
    return NULL;
}

void* IOBluetoothGetDefaultNameOfServiceString(void)
{
    if (verbose) puts("STUB: IOBluetoothGetDefaultNameOfServiceString called");
    return NULL;
}

void* IOBluetoothGetDeviceSelectorController(void)
{
    if (verbose) puts("STUB: IOBluetoothGetDeviceSelectorController called");
    return NULL;
}

void* IOBluetoothGetGenericTypeStringForDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothGetGenericTypeStringForDevice called");
    return NULL;
}

void* IOBluetoothGetGenericTypeStringForDeviceClasses(void)
{
    if (verbose) puts("STUB: IOBluetoothGetGenericTypeStringForDeviceClasses called");
    return NULL;
}

void* IOBluetoothGetLocalizedUIStringForKey(void)
{
    if (verbose) puts("STUB: IOBluetoothGetLocalizedUIStringForKey called");
    return NULL;
}

void* IOBluetoothGetMajorClassOfDeviceString(void)
{
    if (verbose) puts("STUB: IOBluetoothGetMajorClassOfDeviceString called");
    return NULL;
}

void* IOBluetoothGetMajorClassOfDeviceStringClass(void)
{
    if (verbose) puts("STUB: IOBluetoothGetMajorClassOfDeviceStringClass called");
    return NULL;
}

void* IOBluetoothGetMinorClassOfDeviceString(void)
{
    if (verbose) puts("STUB: IOBluetoothGetMinorClassOfDeviceString called");
    return NULL;
}

void* IOBluetoothGetMinorClassOfDeviceStringClass(void)
{
    if (verbose) puts("STUB: IOBluetoothGetMinorClassOfDeviceStringClass called");
    return NULL;
}

void* IOBluetoothGetPairingController(void)
{
    if (verbose) puts("STUB: IOBluetoothGetPairingController called");
    return NULL;
}

void* IOBluetoothIsOptionKeyDown(void)
{
    if (verbose) puts("STUB: IOBluetoothIsOptionKeyDown called");
    return NULL;
}

void* IOBluetoothIsServiceUsableByUser(void)
{
    if (verbose) puts("STUB: IOBluetoothIsServiceUsableByUser called");
    return NULL;
}

void* IOBluetoothIsShiftKeyDown(void)
{
    if (verbose) puts("STUB: IOBluetoothIsShiftKeyDown called");
    return NULL;
}

void* IOBluetoothOpenBluetoothHelpAtPath(void)
{
    if (verbose) puts("STUB: IOBluetoothOpenBluetoothHelpAtPath called");
    return NULL;
}

void* IOBluetoothPairingControllerRunPanelWithAttributes(void)
{
    if (verbose) puts("STUB: IOBluetoothPairingControllerRunPanelWithAttributes called");
    return NULL;
}

void* IOBluetoothPreferenceGetMenuExtraShowState(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetMenuExtraShowState called");
    return NULL;
}

void* IOBluetoothPreferenceSetMenuExtraShowState(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetMenuExtraShowState called");
    return NULL;
}

void* IOBluetoothServiceBrowserControllerBrowseDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothServiceBrowserControllerBrowseDevices called");
    return NULL;
}

void* IOBluetoothServiceBrowserControllerBrowseDevicesAsSheet(void)
{
    if (verbose) puts("STUB: IOBluetoothServiceBrowserControllerBrowseDevicesAsSheet called");
    return NULL;
}

void* IOBluetoothServiceBrowserControllerCreate(void)
{
    if (verbose) puts("STUB: IOBluetoothServiceBrowserControllerCreate called");
    return NULL;
}

void* IOBluetoothServiceBrowserControllerDiscover(void)
{
    if (verbose) puts("STUB: IOBluetoothServiceBrowserControllerDiscover called");
    return NULL;
}

void* IOBluetoothServiceBrowserControllerDiscoverAsSheet(void)
{
    if (verbose) puts("STUB: IOBluetoothServiceBrowserControllerDiscoverAsSheet called");
    return NULL;
}

void* IOBluetoothServiceBrowserControllerDiscoverWithDeviceAttributes(void)
{
    if (verbose) puts("STUB: IOBluetoothServiceBrowserControllerDiscoverWithDeviceAttributes called");
    return NULL;
}

void* IOBluetoothServiceBrowserControllerSetOptions(void)
{
    if (verbose) puts("STUB: IOBluetoothServiceBrowserControllerSetOptions called");
    return NULL;
}

void* IOBluetoothValidateHardware(void)
{
    if (verbose) puts("STUB: IOBluetoothValidateHardware called");
    return NULL;
}

void* IOBluetoothValidateHardwareWithDescription(void)
{
    if (verbose) puts("STUB: IOBluetoothValidateHardwareWithDescription called");
    return NULL;
}

void* ISCopyDeviceIconPath(void)
{
    if (verbose) puts("STUB: ISCopyDeviceIconPath called");
    return NULL;
}

void* ISShouldProposeToAddDeviceToISync(void)
{
    if (verbose) puts("STUB: ISShouldProposeToAddDeviceToISync called");
    return NULL;
}

void* ImageForTypeStr(void)
{
    if (verbose) puts("STUB: ImageForTypeStr called");
    return NULL;
}

void* ObjectTableDataSource_ColumnSorter(void)
{
    if (verbose) puts("STUB: ObjectTableDataSource_ColumnSorter called");
    return NULL;
}

void* TypeForModelStr(void)
{
    if (verbose) puts("STUB: TypeForModelStr called");
    return NULL;
}

void* _IOBluetoothUI_AlignCGPointToUserSpace(void)
{
    if (verbose) puts("STUB: _IOBluetoothUI_AlignCGPointToUserSpace called");
    return NULL;
}

void* _IOBluetoothUI_AlignCGRectToUserSpace(void)
{
    if (verbose) puts("STUB: _IOBluetoothUI_AlignCGRectToUserSpace called");
    return NULL;
}

void* _IOBluetoothUI_AlignPointToUserSpace(void)
{
    if (verbose) puts("STUB: _IOBluetoothUI_AlignPointToUserSpace called");
    return NULL;
}

void* _IOBluetoothUI_AlignRectToUserSpace(void)
{
    if (verbose) puts("STUB: _IOBluetoothUI_AlignRectToUserSpace called");
    return NULL;
}

void* isDebugLogEnabled(void)
{
    if (verbose) puts("STUB: isDebugLogEnabled called");
    return NULL;
}

void* registerValueTransformers(void)
{
    if (verbose) puts("STUB: registerValueTransformers called");
    return NULL;
}
