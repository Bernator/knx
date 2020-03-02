/*
 * data_link_container.cpp
 *
 *  Created on: 29.02.2020
 *      Author: Bernhard
 */

#include "instance_container.h"



InstanceContainer::InstanceContainer(DeviceObject& devObj, AddressTableObject& addrTab, NetworkLayer& layer, uint8_t instanceID):_deviceObject(devObj), _groupAddressTable(addrTab),  _networkLayer(layer) {
	_instanceID = instanceID;
}

InstanceContainer::~InstanceContainer() {
	// TODO Auto-generated destructor stub
}

