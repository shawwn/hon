/*===============================================================================================
 Realtime Tweaking Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2009.

 Demonstrates basic usage of FMOD's network data-driven event library (fmod_event_net.dll)
===============================================================================================*/
#include "../../api/inc/fmod_event.h"
#include "../../api/inc/fmod_event_net.h"
#include "../../api/inc/fmod_errors.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define   NUM_EVENTS 3

const char *MEDIA_PATH = "..\\media\\";

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

FMOD_RESULT F_CALLBACK eventcallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
    if (type == FMOD_EVENT_CALLBACKTYPE_NET_MODIFIED)
    {
        unsigned int t = (unsigned int)param2;
        printf("%p %s %f (%d)\n", event, (unsigned int)param1 == FMOD_EVENTPROPERTY_VOLUME ? "volume" : FMOD_EVENTPROPERTY_PITCH ? "pitch" : "???", *((float *)&t), (unsigned int)userdata);
    }

    return FMOD_OK;
}

int main(int argc, char *argv[])
{
    FMOD_RESULT        result;
    FMOD_EVENTSYSTEM  *eventsystem;
    FMOD_EVENTGROUP   *eventgroup;
    FMOD_EVENT        *event[NUM_EVENTS];
    char              *event_name[NUM_EVENTS] = {
                           "Basics/SimpleEventWithLooping",
                           "SequencingAndStitching/LoopLogic",
                           "3D Events/2D-3DPanMorph"
                       };
    int                key, i, userdata = 0;

    printf("======================================================================\n");
    printf("Realtime Tweaking. Copyright (c) Firelight Technologies 2004-2009.\n");
    printf("======================================================================\n");
    printf("This example shows how to initialize the FMOD Net Event System so that\n");
    printf("FMOD Designer can connect to your game and tweak events as they're\n");
    printf("playing.\n");
    printf("Start some events then connect to this app using the Audition menu\n");
    printf("in FMOD Designer. You can use 127.0.0.1 for the IP address if you\n");
    printf("don't want to use two machines. Load tutorials.fdp and change the \n");
    printf("volume of the playing events using the volume slider in the event\n");
    printf("property sheet\n");
    printf("======================================================================\n\n");

    ERRCHECK(result = FMOD_EventSystem_Create(&eventsystem));
    ERRCHECK(result = FMOD_NetEventSystem_Init(eventsystem, FMOD_EVENT_NET_PORT));
    ERRCHECK(result = FMOD_EventSystem_Init(eventsystem, 64, FMOD_INIT_NORMAL, 0, FMOD_EVENT_INIT_NORMAL));
    ERRCHECK(result = FMOD_EventSystem_SetMediaPath(eventsystem, (char *)MEDIA_PATH));
    ERRCHECK(result = FMOD_EventSystem_Load(eventsystem, "examples.fev", 0, 0));
    ERRCHECK(result = FMOD_EventSystem_GetGroup(eventsystem, "examples/FeatureDemonstration", FMOD_EVENT_DEFAULT, &eventgroup));

    for (i=0; i < NUM_EVENTS; i++)
    {
        event[i] = 0;
    }

    printf("======================================================================\n");
    printf("Press 1 - 3  to start/stop events\n");
    printf("Press ESC    to quit\n");
    printf("======================================================================\n");

    key = 0;
    do
    {
        if (_kbhit())
        {
            key = _getch();

            if ((key >= '1') && (key <= '3'))
            {
                i = (int)(key - '1');

                if (event[i])
                {
                    ERRCHECK(result = FMOD_Event_Stop(event[i], 1));
                    event[i] = 0;
                    printf("Stopping '%s'\n", event_name[i]);
                }
                else
                {
                    ERRCHECK(result = FMOD_EventGroup_GetEvent(eventgroup, event_name[i], FMOD_EVENT_DEFAULT, &event[i]));
                    ERRCHECK(result = FMOD_Event_SetCallback(event[i], eventcallback, (void *)(userdata++)));
                    ERRCHECK(result = FMOD_Event_Start(event[i]));
                    printf("Starting '%s'\n", event_name[i]);
                }
            }
        }

        ERRCHECK(result = FMOD_EventSystem_Update(eventsystem));
        ERRCHECK(result = FMOD_NetEventSystem_Update());
        Sleep(10);

    } while (key != 27);

    ERRCHECK(result = FMOD_EventSystem_Release(eventsystem));
    ERRCHECK(result = FMOD_NetEventSystem_Shutdown());
    return 0;
}
