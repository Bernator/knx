#include "data_link_layer.h"
#include "bits.h"
#include "device_object.h"
#include "address_table_object.h"


DataLinkLayer::DataLinkLayer()
{
}

void DataLinkLayer::addInstance(InstanceContainer* instance){
	_LayerList.add(instance);

}

void DataLinkLayer::dataRequest(uint8_t instanceID, AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format, Priority priority, NPDU& npdu)
{
    sendTelegram(instanceID, npdu, ack, destinationAddr, addrType, format, priority);
}

void DataLinkLayer::systemBroadcastRequest(uint8_t instanceID, AckType ack, FrameFormat format, Priority priority, NPDU& npdu)
{
    sendTelegram(instanceID, npdu, ack, 0, GroupAddress, format, priority);
}

void DataLinkLayer::dataConReceived(uint8_t instanceID, CemiFrame& frame,bool success)
{


	InstanceContainer* instance = getInstance(instanceID);
	if(instance == nullptr)
		return;

	AckType ack = frame.ack();
    AddressType addrType = frame.addressType();
    uint16_t destination = frame.destinationAddress();
    uint16_t source = frame.sourceAddress();
    FrameFormat type = frame.frameType();
    Priority priority = frame.priority();
    NPDU& npdu = frame.npdu();

    if (addrType == GroupAddress && destination == 0)
    	instance->networkLayer().systemBroadcastConfirm(ack, type, priority, source, npdu, success);
    else
    	instance->networkLayer().dataConfirm(ack, addrType, destination, type, priority, source, npdu, success);


}
void DataLinkLayer::frameRecieved(CemiFrame& frame)
{
    AckType ack = frame.ack();
    AddressType addrType = frame.addressType();
    uint16_t destination = frame.destinationAddress();
    uint16_t source = frame.sourceAddress();
    FrameFormat type = frame.frameType();
    Priority priority = frame.priority();
    NPDU& npdu = frame.npdu();
    
    for(int i=0;i<_LayerList.size();i++){
    	InstanceContainer* instance = _LayerList.get(i);

		uint16_t ownAddr = instance->deviceObject().induvidualAddress();

		if (source == ownAddr)
			instance->deviceObject().induvidualAddressDuplication(true);

		if (addrType == GroupAddress && destination == 0)
			instance->networkLayer().systemBroadcastIndication(ack, type, npdu, priority, source);
		else
		{
			if (addrType == InduvidualAddress && destination != instance->deviceObject().induvidualAddress())
				continue;

			if (addrType == GroupAddress && !instance->groupAddressTable().contains(destination))
				continue;

			instance->networkLayer().dataIndication(ack, addrType, destination, type, npdu, priority, source);
		}
    }
}


bool DataLinkLayer::sendTelegram(uint8_t instanceID, NPDU & npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority)
{
    InstanceContainer* instance = getInstance(instanceID);
    if(instance == nullptr)
    	return false;

    uint16_t source = instance->deviceObject().induvidualAddress();
	CemiFrame& frame = npdu.frame();
    frame.messageCode(L_data_ind);
    frame.destinationAddress(destinationAddr);
    frame.sourceAddress(source);
    frame.addressType(addrType);
    frame.priority(priority);
    frame.repetition(RepititionAllowed);

    if (npdu.octetCount() <= 15)
        frame.frameType(StandardFrame);
    else
        frame.frameType(format);


    if (!frame.valid())
    {
        println("invalid frame");
        return false;
    }

    for(int i=0;i<_LayerList.size();i++){
    	InstanceContainer* instance = _LayerList.get(i);
    	if(instance->instanceID() == instanceID)
    		continue;

		uint16_t ownAddr = instance->deviceObject().induvidualAddress();

		if (source == ownAddr)
			instance->deviceObject().induvidualAddressDuplication(true);

		if (addrType == GroupAddress && destinationAddr == 0)
			instance->networkLayer().systemBroadcastIndication(ack, frame.frameType(), npdu, priority, source);
		else
		{
			if (addrType == InduvidualAddress && destinationAddr != instance->deviceObject().induvidualAddress())
				continue;

			if (addrType == GroupAddress && !instance->groupAddressTable().contains(destinationAddr))
				continue;

			instance->networkLayer().dataIndication(ack, addrType, destinationAddr, frame.frameType(), npdu, priority, source);
		}
    }

    return sendFrame(instanceID, frame);
}

uint8_t* DataLinkLayer::frameData(CemiFrame& frame)
{
    return frame._data;
}

bool DataLinkLayer::containsIndividualAddress(uint16_t addr){
	for(int i=0;i<_LayerList.size();i++){
		if (_LayerList.get(i)->deviceObject().induvidualAddress() == addr){
			return true;
		}
	}
	return false;
}
bool DataLinkLayer::containsGroupAddress(uint16_t addr){
	for(int i=0;i<_LayerList.size();i++){
		if (_LayerList.get(i)->groupAddressTable().contains(addr)){
			return true;
		}
	}
	return false;
}

InstanceContainer* DataLinkLayer::getInstance(uint8_t instanceID){
	for(int i=0;i<_LayerList.size();i++){
		InstanceContainer* instance = _LayerList.get(i);
		if (instance->instanceID() == instanceID){
			return instance;
		}
	}
	return nullptr;
}
