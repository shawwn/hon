/*===============================================================================================
 ManualDecode Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2009.

 Sound played here decoded in realtime by the user with a 'decoder sound'    
 The mp3 is created as a stream opened with FMOD_OPENONLY. This is the       
 'decoder sound'.  The playback sound is a 16bit PCM FMOD_OPENUSER created   
 sound with a pcm read callback.  When the callback happens, we call readData
 on the decoder sound and use the pcmreadcallback data pointer as the parameter.
===============================================================================================*/
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "../../api/inc/fmod.hpp"
#include "../../api/inc/fmod_errors.h"

FMOD::Sound      *decodesound = 0;
CRITICAL_SECTION  decodecrit;

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

FMOD_RESULT F_CALLBACK pcmreadcallback(FMOD_SOUND *sound, void *data, unsigned int datalen)
{
    FMOD_RESULT result;
    unsigned int read;

    EnterCriticalSection(&decodecrit);
    
    if (!decodesound)
    {
        return FMOD_ERR_FILE_EOF;
    }

    result = decodesound->readData(data, datalen, &read);

    if (result == FMOD_ERR_FILE_EOF)    /* Handle looping. */
    {
        decodesound->seekData(0);
        
        datalen -= read;

        result = decodesound->readData((char *)data + read, datalen, &read);
    }

    LeaveCriticalSection(&decodecrit);

    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK pcmsetposcallback(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    /*
        The timeunit will usually be FMOD_TIMEUNIT_PCM, as FMOD will convert the user's timeunit to pcm for the codec.
        If it is not, do the necessary conversion of whatever is coming in, to PCM samples for Sound::seekData.
    */
    
    decodesound->seekData(position);

    return FMOD_OK;
}


int main(int argc, char *argv[])
{
    FMOD::System           *system;
    FMOD::Sound            *sound;
    FMOD::Channel          *channel = 0;
    FMOD_RESULT             result;
    int                     key;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    unsigned int            version, decodesound_lengthbytes = 0;
    int                     decodesound_channels;
    float                   decodesound_rate;

    /*
        Create a System object and initialize.
    */
    result = FMOD::System_Create(&system);
    ERRCHECK(result);

    result = system->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }

    result = system->init(32, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    InitializeCriticalSection(&decodecrit);

    /*
        First create the 'decoder sound'.  Note it is a stream that does not initially read any data, because FMOD_OPENONLY has been specified.
        We could use createSound instead of createStream but that would allocate memory for the whole sound which is a waste.
    */
    result = system->createStream("../media/wave.mp3", FMOD_OPENONLY | FMOD_LOOP_NORMAL | FMOD_LOWMEM | FMOD_CREATESTREAM, 0, &decodesound); 
    ERRCHECK(result);

    result = decodesound->getLength(&decodesound_lengthbytes, FMOD_TIMEUNIT_PCMBYTES);
    ERRCHECK(result);

    result = decodesound->getFormat(0, 0, &decodesound_channels, 0);
    ERRCHECK(result);

    result = decodesound->getDefaults(&decodesound_rate, 0, 0, 0);
    ERRCHECK(result);
        
    /*
        Now create a user created PCM stream that we will feed data into, and play.
    */
    memset(&createsoundexinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    createsoundexinfo.cbsize            = sizeof(FMOD_CREATESOUNDEXINFO);              /* required. */
    createsoundexinfo.decodebuffersize  = 44100;                                       /* Chunk size of stream update in samples.  This will be the amount of data passed to the user callback. */
    createsoundexinfo.numchannels       = decodesound_channels;                        /* Number of channels in the sound. */
    createsoundexinfo.length            = decodesound_lengthbytes;                     /* Length of PCM data in bytes of whole song.  -1 = infinite. */
    createsoundexinfo.defaultfrequency  = (int)decodesound_rate;                       /* Default playback rate of sound. */
    createsoundexinfo.format            = FMOD_SOUND_FORMAT_PCM16;                     /* Data format of sound. */
    createsoundexinfo.pcmreadcallback   = pcmreadcallback;                             /* User callback for reading. */
    createsoundexinfo.pcmsetposcallback = pcmsetposcallback;                           /* User callback for seeking. */

    result = system->createStream(0, FMOD_2D | FMOD_OPENUSER | FMOD_LOOP_NORMAL, &createsoundexinfo, &sound);
    ERRCHECK(result);

    printf("============================================================================\n");
    printf("Manual Decode example.  Copyright (c) Firelight Technologies 2004-2009.\n");
    printf("============================================================================\n");
    printf("Sound played here decoded in realtime by the user with a 'decoder sound'    \n");
    printf("The mp3 is created as a stream opened with FMOD_OPENONLY. This is the       \n");
    printf("'decoder sound'.  The playback sound is a 16bit PCM FMOD_OPENUSER created   \n");
    printf("sound with a pcm read callback.  When the callback happens, we call readData\n");
    printf("on the decoder sound and use the pcmreadcallback data pointer as the parameter.\n");
    printf("============================================================================\n");
    printf("\n");
    printf("Press space to pause, Esc to quit\n");
    printf("Press '<' to rewind 1 second.\n");
    printf("Press '>' to fast forward 1 second.\n");
    printf("\n");

    /*
        Play the sound.
    */

    result = system->playSound(FMOD_CHANNEL_FREE, sound, 0, &channel);
    ERRCHECK(result);

    /*
        Main loop.
    */
    do
    {
        if (kbhit())
        {
            key = getch();

            switch (key)
            {
                case ' ' :
                {
                    bool paused;
                    channel->getPaused(&paused);
                    channel->setPaused(!paused);
                    break;
                }
                case '<' :
                {
                    unsigned int position;

                    channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                    if (position >= 1000)
                    {
                        position -= 1000;
                    }
                    channel->setPosition(position, FMOD_TIMEUNIT_MS);
                    break;
                }
                case '>' :
                {
                    unsigned int position;

                    channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                    position += 1000;
                    channel->setPosition(position, FMOD_TIMEUNIT_MS);
                    break;
                }
            }
        }

        system->update();

        if (channel)
        {
            unsigned int ms;
            unsigned int lenms;
            bool         playing;
            bool         paused;

            channel->isPlaying(&playing);
            if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
            {
                ERRCHECK(result);
            }

            result = channel->getPaused(&paused);
            if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
            {
                ERRCHECK(result);
            }

            result = channel->getPosition(&ms, FMOD_TIMEUNIT_MS);
            if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
            {
                ERRCHECK(result);
            }

            result = sound->getLength(&lenms, FMOD_TIMEUNIT_MS);
            if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
            {
                ERRCHECK(result);
            }

            printf("Time %02d:%02d:%02d/%02d:%02d:%02d : %s\r", ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped");
        }

        Sleep(20);

    } while (key != 27);

    printf("\n");

    EnterCriticalSection(&decodecrit);
    {
        /*
            Remove the sound - wait! it might be still in use!
            Instead of releasing the decode sound first we could release it last, but this protection is here to make the issue obvious.
        */
        result = decodesound->release();
        ERRCHECK(result);
        decodesound = 0;    /* This will make the read callback fail from now on. */
    }
    LeaveCriticalSection(&decodecrit);

    /*
        Shut down
    */
    result = sound->release();
    ERRCHECK(result);
    result = system->close();
    ERRCHECK(result);
    result = system->release();
    ERRCHECK(result);

    DeleteCriticalSection(&decodecrit);

    return 0;
}
