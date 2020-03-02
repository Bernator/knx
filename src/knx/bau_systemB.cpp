#include "bau_systemB.h"
#include <string.h>
#include <stdio.h>
#include "globalPlatform.h"

BauSystemB::BauSystemB(uint8_t instanceID): _deviceObj(), _memory(instanceID), _addrTable(),
    _assocTable(), _groupObjTable(), _appProgram(),_appLayer(_assocTable, *this),
    _transLayer(_appLayer, _addrTable), _netLayer(_transLayer,instanceID)
{
    _appLayer.transportLayer(_transLayer);
    _transLayer.networkLayer(_netLayer);
    _memory.addSaveRestore(&_addrTable);
    _memory.addSaveRestore(&_assocTable);
    _memory.addSaveRestore(&_groupObjTable);
    _memory.addSaveRestore(&_deviceObj);
    _memory.addSaveRestore(&_appProgram);
    _instanceID = instanceID;
}

void BauSystemB::loop()
{
    dataLinkLayer().loop();
    _transLayer.loop();
    sendNextGroupTelegram();
}

bool BauSystemB::enabled()
{
    return dataLinkLayer().enabled();
}

void BauSystemB::enabled(bool value)
{
    dataLinkLayer().enabled(value);
}

void BauSystemB::sendNextGroupTelegram()
{
    GroupObject* go = _groupObjTable.getObjectToSend();
	if(go != nullptr){
        ComFlag flag = go->commFlag();
        if (flag == WriteRequest && go->transmitEnable())
        {
            uint8_t* data = go->valueRef();
            _appLayer.groupValueWriteRequest(AckRequested, go->asap(), go->priority(), NetworkLayerParameter, data,
                go->sizeInTelegram());
        }
        else if (flag == ReadRequest)
        {
            _appLayer.groupValueReadRequest(AckRequested, go->asap(), go->priority(), NetworkLayerParameter);
        }

        go->commFlag(Transmitting);
	}
}

void BauSystemB::updateGroupObject(GroupObject & go, uint8_t * data, uint8_t length)
{
    uint8_t* goData = go.valueRef();
    if (length != go.valueSize())
    {
        go.commFlag(Error);
        return;
    }

    memcpy(goData, data, length);

    go.commFlag(Updated);
    GroupObjectUpdatedHandler handler = go.callback();
    if (handler)
        handler(_instanceID, go);
}

void BauSystemB::readMemory()
{
    _memory.readMemory();
}

void BauSystemB::writeMemory()
{
    _memory.writeMemory();
}

DeviceObject& BauSystemB::deviceObject()
{
    return _deviceObj;
}

GroupObjectTableObject& BauSystemB::groupObjectTable()
{
    return _groupObjTable;
}

ApplicationProgramObject& BauSystemB::parameters()
{
    return _appProgram;
}

bool BauSystemB::configured()
{
    // _configured is set to true initially, if the device was configured with ETS it will be set to true after restart
    
    if (!_configured)
        return false;
    
    _configured = _groupObjTable.loadState() == LS_LOADED
        && _addrTable.loadState() == LS_LOADED
        && _assocTable.loadState() == LS_LOADED
        && _appProgram.loadState() == LS_LOADED;
    
    return _configured;
}

void BauSystemB::deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t descriptorType)
{
    if (descriptorType != 0)
        descriptorType = 0x3f;

    _appLayer.deviceDescriptorReadResponse(AckRequested, priority, hopType, asap, descriptorType, descriptor());
}

void BauSystemB::memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint16_t memoryAddress, uint8_t * data)
{

    for(uint8_t i=0;i<number;i++)
        platform.writeNVMemory(platform.referenceNVMemory() + memoryAddress+i, data[i]);

    _memory.memoryModified();

    if (_deviceObj.verifyMode())
        memoryReadIndication(priority, hopType, asap, number, memoryAddress);
}

void BauSystemB::memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint16_t memoryAddress)
{
    uint8_t* buffer = new uint8_t[number];
    for(uint8_t i=0;i<number;i++)
        buffer[i] = platform.readNVMemory(platform.referenceNVMemory() + memoryAddress+i);

    _appLayer.memoryReadResponse(AckRequested, priority, hopType, asap, number, memoryAddress,buffer);
    delete[] buffer;
}

void BauSystemB::restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap)
{
    // Flush the EEPROM before resetting
    _memory.writeMemory();
    platform.restart();
}

void BauSystemB::authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, uint32_t key)
{
    _appLayer.authorizeResponse(AckRequested, priority, hopType, asap, 0);
}

void BauSystemB::userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress)
{
    uint8_t* buffer = new uint8_t[number];
    for(uint8_t i=0;i<number;i++)
        buffer[i] = platform.readNVMemory(platform.referenceNVMemory() + memoryAddress+i);
    _appLayer.userMemoryReadResponse(AckRequested, priority, hopType, asap, number, memoryAddress,buffer);
    delete[] buffer;
}

void BauSystemB::userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress, uint8_t* data)
{
    for(uint8_t i=0;i<number;i++)
        platform.writeNVMemory(platform.referenceNVMemory() + memoryAddress+i, data[i]);

    _memory.memoryModified();
    if (_deviceObj.verifyMode())
        userMemoryReadIndication(priority, hopType, asap, number, memoryAddress);
}

void BauSystemB::propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
    uint8_t propertyId, uint8_t propertyIndex)
{
    uint8_t pid = propertyId;
    bool writeEnable = false;
    uint8_t type = 0;
    uint16_t numberOfElements = 0;
    uint8_t access = 0;
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if (obj)
        obj->readPropertyDescription(pid, propertyIndex, writeEnable, type, numberOfElements, access);

    _appLayer.propertyDescriptionReadResponse(AckRequested, priority, hopType, asap, objectIndex, pid, propertyIndex,
        writeEnable, type, numberOfElements, access);
}

void BauSystemB::propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if(obj)
        obj->writeProperty((PropertyID)propertyId, startIndex, data, numberOfElements);
    propertyValueReadIndication(priority, hopType, asap, objectIndex, propertyId, numberOfElements, startIndex);
}

void BauSystemB::propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
    uint8_t size = 0;
    uint32_t elementCount = numberOfElements;
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if (obj)
    {
        uint8_t elementSize = obj->propertySize((PropertyID)propertyId);
        size = elementSize * numberOfElements;
    }
    else
        elementCount = 0;

    uint8_t data[size];
    if(obj)
        obj->readProperty((PropertyID)propertyId, startIndex, elementCount, data);
    _appLayer.propertyValueReadResponse(AckRequested, priority, hopType, asap, objectIndex, propertyId, elementCount,
        startIndex, data, size);
}

void BauSystemB::individualAddressReadIndication(HopCountType hopType)
{
    if (_deviceObj.progMode())
        _appLayer.individualAddressReadResponse(AckRequested, hopType);
}

void BauSystemB::individualAddressWriteIndication(HopCountType hopType, uint16_t newaddress)
{
    if (_deviceObj.progMode())
        _deviceObj.induvidualAddress(newaddress);
}

void BauSystemB::groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void BauSystemB::groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void BauSystemB::groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.readEnable())
        return;
    
    uint8_t* data = go.valueRef();
    _appLayer.groupValueReadResponse(AckRequested, asap, priority, hopType, data, go.sizeInTelegram());
}

void BauSystemB::groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data,
    uint8_t dataLength)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.responseUpdateEnable())
        return;

    updateGroupObject(go, data, dataLength);
}

void BauSystemB::groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.writeEnable())
        return;

    updateGroupObject(go, data, dataLength);
}

void BauSystemB::addSaveRestore(SaveRestore* obj)
{
    _memory.addSaveRestore(obj);
}


void BauSystemB::restartRequest(uint16_t asap)
{
    _appLayer.restartRequest(AckRequested, LowPriority, NetworkLayerParameter, asap);
}
