/*===============================================================================================
 CDPlayer Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2009.

 This example shows how to play CD tracks digitally and generate a CDDB query
===============================================================================================*/
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "../../api/inc/fmod.h"
#include "../../api/inc/fmod_errors.h"

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

int cddb_sum(int n)
{
    int	ret = 0;

    while (n > 0)
    {
        ret += (n % 10);
        n /= 10;
    }

    return ret;
}

unsigned long cddb_discid(FMOD_CDTOC *toc)
{
    int	i, t, n = 0;

    for (i = 0; i < toc->numtracks; i++)
    {
        n += cddb_sum((toc->min[i] * 60) + toc->sec[i]);
    }

    t = ((toc->min[toc->numtracks] * 60) + toc->sec[toc->numtracks]) - ((toc->min[0] * 60) + toc->sec[0]);

    return ((n % 0xff) << 24 | t << 8 | toc->numtracks);
}

void dump_cddb_query(FMOD_CDTOC *toc)
{
    int i;

    printf("cddb query %08x %d", cddb_discid(toc), toc->numtracks);

    for (i = 0; i < toc->numtracks; i++)
    {
        printf(" %d", (toc->min[i] * (60 * 75)) + (toc->sec[i] * 75) + toc->frame[i]);
    }

    printf(" %d\n", (toc->min[toc->numtracks] * 60) + toc->sec[toc->numtracks]);
}


int main(int argc, char *argv[])
{
    FMOD_SYSTEM     *system;
    FMOD_SOUND      *cdsound;
    FMOD_CHANNEL    *channel = 0;
    FMOD_RESULT      result;
    int              key;
    unsigned int     numtracks, currenttrack = 0;
    unsigned int     version;

    printf("==================================================================\n");
    printf("CDPlayer Example.  Copyright (c) Firelight Technologies 2004-2009.\n");
    printf("==================================================================\n\n");

    /*
        Create a System object and initialize.
    */
    result = FMOD_System_Create(&system);
    ERRCHECK(result);

    result = FMOD_System_GetVersion(system, &version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }

    result = FMOD_System_Init(system, 1, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);

    /*
        Bump up the file buffer size a bit from the 16k default for CDDA, because it is a slower medium.
    */
    result = FMOD_System_SetStreamBufferSize(system, 64*1024, FMOD_TIMEUNIT_RAWBYTES);
    ERRCHECK(result);

    /*
        Try a few drive letters.
    */
    result = FMOD_System_CreateStream(system, "d:", FMOD_OPENONLY, 0, &cdsound);
    if (result != FMOD_OK)
    {
        result = FMOD_System_CreateStream(system, "e:", FMOD_OPENONLY, 0, &cdsound);
        if (result != FMOD_OK)
        {
            result = FMOD_System_CreateStream(system, "f:", FMOD_OPENONLY, 0, &cdsound);
            ERRCHECK(result);
        }
    }
    result = FMOD_Sound_GetNumSubSounds(cdsound, &numtracks);
    ERRCHECK(result);

    for (;;)
    {
        FMOD_TAG tag;

        if (FMOD_Sound_GetTag(cdsound, 0, -1, &tag) != FMOD_OK)
        {
            break;
        }
        if (tag.datatype == FMOD_TAGDATATYPE_CDTOC)
        {
            dump_cddb_query((FMOD_CDTOC *)tag.data);
        }
    }

    printf("\n========================================\n");
    printf("Press SPACE to pause\n");
    printf("      n     to skip to next track\n");
    printf("      <     re-wind 10 seconds\n");
    printf("      >     fast-forward 10 seconds\n");
    printf("      ESC   to exit\n");
    printf("========================================\n\n");

    /*
        Print out length of entire CD.  Did you know you can also play 'cdsound' and it will play the whole CD without gaps?
    */
    {
        unsigned int lenms;

        result = FMOD_Sound_GetLength(cdsound, &lenms, FMOD_TIMEUNIT_MS);
        ERRCHECK(result);

        printf("Total CD length %02d:%02d\n\n", lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100);
    }

    /*
        Play whole CD
    */
    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, cdsound, FALSE, &channel);
    ERRCHECK(result);

    /*
        Main loop
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
                    int paused;
                    FMOD_Channel_GetPaused(channel, &paused);
                    FMOD_Channel_SetPaused(channel, !paused);
                    break;
                }

                case '<' :
                {
                    unsigned int ms;

                    FMOD_Channel_GetPosition(channel, &ms, FMOD_TIMEUNIT_SENTENCE_MS);

                    if (ms >= 10000)
                    {
                        ms -= 10000;
                    }
                    else
                    {
                        ms = 0;
                    }

                    FMOD_Channel_SetPosition(channel, ms, FMOD_TIMEUNIT_SENTENCE_MS);
                    break;
                }


                case '>' :
                {
                    unsigned int ms;

                    FMOD_Channel_GetPosition(channel, &ms, FMOD_TIMEUNIT_SENTENCE_MS);

                    ms += 10000;

                    FMOD_Channel_SetPosition(channel, ms, FMOD_TIMEUNIT_SENTENCE_MS);
                    break;
                }

                case 'n' :
                {
                    FMOD_Channel_GetPosition(channel, &currenttrack, FMOD_TIMEUNIT_SENTENCE_SUBSOUND);

                    currenttrack++;
                    if (currenttrack >= numtracks)
                    {
                        currenttrack = 0;
                    }

                    FMOD_Channel_SetPosition(channel, currenttrack, FMOD_TIMEUNIT_SENTENCE_SUBSOUND);
                    break;
                }
            }
        }

        FMOD_System_Update(system);

        if (channel)
        {
            unsigned int ms;
            unsigned int lenms;
            FMOD_BOOL    playing;
            FMOD_BOOL    paused;
            int          busy;

            result = FMOD_Channel_GetPaused(channel, &paused);
            ERRCHECK(result);
            result = FMOD_Channel_IsPlaying(channel, &playing);
            ERRCHECK(result);
            result = FMOD_Channel_GetPosition(channel, &ms, FMOD_TIMEUNIT_SENTENCE_MS);
            ERRCHECK(result);
            result = FMOD_Sound_GetLength(cdsound, &lenms, FMOD_TIMEUNIT_SENTENCE_MS);
            ERRCHECK(result);

            result = FMOD_File_GetDiskBusy(&busy);
            ERRCHECK(result);

            printf("Track %d/%d : %02d:%02d:%02d/%02d:%02d:%02d : %s (%s)\r", currenttrack + 1, numtracks, ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped", busy ? "*" : " ");
        }

        Sleep(50);

    } while (key != 27);

    printf("\n");

    /*
        Shut down
    */
    result = FMOD_Sound_Release(cdsound);
    ERRCHECK(result);
    result = FMOD_System_Close(system);
    ERRCHECK(result);
    result = FMOD_System_Release(system);
    ERRCHECK(result);

    return 0;
}
