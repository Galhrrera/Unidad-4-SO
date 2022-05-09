#ifndef _EVENT_H
#define _EVENT_H

// Event class
typedef struct _event
{
    char eventName[16];
    struct _event *next;
    int count_subdClients;
    int l_subdClients[];
} Event;

Event *CreateEvent(char *name);
void DestroyEvent(Event *this);

#endif