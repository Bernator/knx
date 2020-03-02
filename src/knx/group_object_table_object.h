#pragma once

#include "table_object.h"
#include "group_object.h"



class GroupObjectTableObject : public TableObject
{
    friend class GroupObject;

  public:
    GroupObjectTableObject();
    virtual ~GroupObjectTableObject();
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    uint16_t entryCount();
    GroupObject& get(uint16_t asap);
    GroupObject& nextUpdatedObject(bool& valid);
    void groupObjects(GroupObject* objs, uint16_t size);

    virtual void restore(uint8_t* startAddr);
    GroupObject* getObjectToSend();
  protected:
    virtual void beforeStateChange(LoadState& newState);
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();
    void sendObject(GroupObject* obj);

  private:
    void freeGroupObjects();
    bool initGroupObjects();
    uint16_t* _tableData = 0;
    GroupObject* _groupObjects = 0;
    uint16_t _groupObjectCount = 0;
    uint16_t _startIdx = 1;
    struct queue_entry_t
    {
    	GroupObject* _groupObject;
    	queue_entry_t* next;
    };

    struct _send_queue_t
    {
    	queue_entry_t* front = nullptr;
    	queue_entry_t* back = nullptr;
    } _send_queue;
};
