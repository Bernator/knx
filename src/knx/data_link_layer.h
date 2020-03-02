#pragma once

#include <stdint.h>
#include "device_object.h"
#include "address_table_object.h"
#include "knx_types.h"
#include "network_layer.h"
#include "instance_container.h"
#include "LinkedList.h"



class DataLinkLayer
{
  public:
    DataLinkLayer();

    // from network layer
    void dataRequest(uint8_t instanceID, AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format,
                     Priority priority, NPDU& npdu);
    void systemBroadcastRequest(uint8_t instanceID, AckType ack, FrameFormat format, Priority priority, NPDU& npdu);
    virtual void loop() = 0;
    virtual void enabled(bool value) = 0;
    virtual bool enabled() const = 0;
    void addInstance(InstanceContainer* instance);
  protected:
    void frameRecieved(CemiFrame& frame);
    void dataConReceived(uint8_t instanceID, CemiFrame& frame, bool success);
    bool sendTelegram(uint8_t instanceID, NPDU& npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority);
    virtual bool sendFrame(uint8_t instanceID, CemiFrame& frame) = 0;
    uint8_t* frameData(CemiFrame& frame);
    bool containsIndividualAddress(uint16_t addr);
    bool containsGroupAddress(uint16_t addr);
    InstanceContainer* getInstance(uint8_t instanceID);
  private:
    LinkedList<InstanceContainer*> _LayerList;
};
