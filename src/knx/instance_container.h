/*
 * data_link_container.h
 *
 *  Created on: 29.02.2020
 *      Author: Bernhard
 */

#ifndef KNX_STACK_SRC_KNX_DATA_LINK_CONTAINER_H_
#define KNX_STACK_SRC_KNX_DATA_LINK_CONTAINER_H_

#include "device_object.h"
#include "address_table_object.h"
#include "knx_types.h"
#include "network_layer.h"

#define NO_INSTANCE		255

class InstanceContainer {
public:
	InstanceContainer(DeviceObject& devObj, AddressTableObject& addrTab, NetworkLayer& layer, uint8_t instanceID);
	virtual ~InstanceContainer();

	DeviceObject& deviceObject(){return _deviceObject;}
	AddressTableObject& groupAddressTable(){return _groupAddressTable;}
	NetworkLayer& networkLayer(){return _networkLayer;}
	uint8_t instanceID(){return _instanceID;}
protected:
    DeviceObject& _deviceObject;
    AddressTableObject& _groupAddressTable;
    NetworkLayer& _networkLayer;
private:
    uint8_t _instanceID = NO_INSTANCE;
};

#endif /* KNX_STACK_SRC_KNX_DATA_LINK_CONTAINER_H_ */
