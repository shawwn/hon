/* ======================================================================================== */
/* FMOD Ex - Memory info header file. Copyright (c), Firelight Technologies Pty, Ltd. 2008. */
/*                                                                                          */
/* Use this header if you are interested in getting detailed information on FMOD's memory   */
/* usage. See the documentation for more details.                                           */
/*                                                                                          */
/* ======================================================================================== */

namespace FMOD
{

    /*
    [ENUM]
    [
        [DESCRIPTION]
        Description of "memoryused_array" fields returned by the getMemoryInfo function of every public FMOD class.

        [REMARKS]
        Every public FMOD class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
        The FMOD_MEMTYPE enumeration values can be used to address the "memoryused_array" returned by getMemoryInfo. See System::getMemoryInfo for an example.

        [PLATFORMS]
        Win32, Win64, Linux, Linux64, Macintosh, Xbox, Xbox360, PlayStation 2, GameCube, PlayStation Portable, PlayStation 3, Wii, Solaris

        [SEE_ALSO]
        System::getMemoryInfo
        EventSystem::getMemoryInfo
        FMOD_MEMBITS
        FMOD_EVENT_MEMBITS
    ]
    */
    public enum MEMTYPE :int
    {
        OTHER                   = 0,      /* Memory not accounted for by other types */
        STRING                  = 1,      /* String data */

        SYSTEM                  = 2,      /* System object and various internals */
        PLUGINS                 = 3,      /* Plugin objects and internals */
        OUTPUT                  = 4,      /* Output module object and internals */
        CHANNEL                 = 5,      /* Channel related memory */
        CHANNELGROUP            = 6,      /* ChannelGroup objects and internals */
        CODEC                   = 7,      /* Codecs allocated for streaming */
        FILE                    = 8,      /* File buffers and structures */
        SOUND                   = 9,      /* Sound objects and internals */
        SOUND_SECONDARYRAM      = 10,     /* Sound data stored in secondary RAM */
        SOUNDGROUP              = 11,     /* SoundGroup objects and internals */
        STREAMBUFFER            = 12,     /* Stream buffer memory */
        DSPCONNECTION           = 13,     /* DSPConnection objects and internals */
        DSP                     = 14,     /* DSP implementation objects */
        DSPCODEC                = 15,     /* Realtime file format decoding DSP objects */
        PROFILE                 = 16,     /* Profiler memory footprint. */
        RECORDBUFFER            = 17,     /* Buffer used to store recorded data from microphone */
        REVERB                  = 18,     /* Reverb implementation objects */
        REVERBCHANNELPROPS      = 19,     /* Reverb channel properties structs */
        GEOMETRY                = 20,     /* Geometry objects and internals */
        SYNCPOINT               = 21,     /* Sync point memory. */

        EVENTSYSTEM             = 22,     /* EventSystem and various internals */
        MUSICSYSTEM             = 23,     /* MusicSystem and various internals */
        FEV                     = 24,     /* Definition of objects contained in all loaded projects e.g. events, groups, categories */
        MEMORYFSB               = 25,     /* Data loaded with registerMemoryFSB */
        EVENTPROJECT            = 26,     /* EventProject objects and internals */
        EVENTGROUPI             = 27,     /* EventGroup objects and internals */
        SOUNDBANKCLASS          = 28,     /* Objects used to manage wave banks */
        SOUNDBANKLIST           = 29,     /* Data used to manage lists of wave bank usage */
        STREAMINSTANCE          = 30,     /* Stream objects and internals */
        SOUNDDEFCLASS           = 31,     /* Sound definition objects */
        SOUNDDEFDEFCLASS        = 32,     /* Sound definition static data objects */
        SOUNDDEFPOOL            = 33,     /* Sound definition pool data */
        REVERBDEF               = 34,     /* Reverb definition objects */
        EVENTREVERB             = 35,     /* Reverb objects */
        USERPROPERTY            = 36,     /* User property objects */
        EVENTINSTANCE           = 37,     /* Event instance base objects */
        EVENTINSTANCE_COMPLEX   = 38,     /* Complex event instance objects */
        EVENTINSTANCE_SIMPLE    = 39,     /* Simple event instance objects */
        EVENTINSTANCE_LAYER     = 40,     /* Event layer instance objects */
        EVENTINSTANCE_SOUND     = 41,     /* Event sound instance objects */
        EVENTENVELOPE           = 42,     /* Event envelope objects */
        EVENTENVELOPEDEF        = 43,     /* Event envelope definition objects */
        EVENTPARAMETER          = 44,     /* Event parameter objects */
        EVENTCATEGORY           = 45,     /* Event category objects */
        EVENTENVELOPEPOINT      = 46,     /* Event envelope point objects */
        EVENTINSTANCEPOOL       = 47,     /* Event instance pool memory */

        MAX,                              /* Number of "memoryused_array" fields. */
    }


    /*
    [DEFINE]
    [
        [NAME]
        FMOD_MEMBITS

        [DESCRIPTION]
        Bitfield used to request specific memory usage information from the getMemoryInfo function of every public FMOD Ex class.<br>
        Use with the "memorybits" parameter of getMemoryInfo to get information on FMOD Ex memory usage.

        [REMARKS]
        Every public FMOD class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
        The FMOD_MEMBITS defines can be OR'd together to specify precisely what memory usage you'd like to get information on. See System::getMemoryInfo for an example.

        [PLATFORMS]
        Win32, Win64, Linux, Linux64, Macintosh, Xbox, Xbox360, PlayStation 2, GameCube, PlayStation Portable, PlayStation 3, Wii, Solaris

        [SEE_ALSO]
        FMOD_EVENT_MEMBITS
        EventSystem::getMemoryInfo
    ]
    */
    public enum MEMBITS :uint
    {
        OTHER                     = (1 << MEMTYPE.OTHER),               /* Memory not accounted for by other types */
        STRING                    = (1 << MEMTYPE.STRING),              /* String data */

        SYSTEM                    = (1 << MEMTYPE.SYSTEM),              /* System object and various internals */
        PLUGINS                   = (1 << MEMTYPE.PLUGINS),             /* Plugin objects and internals */
        OUTPUT                    = (1 << MEMTYPE.OUTPUT),              /* Output module object and internals */
        CHANNEL                   = (1 << MEMTYPE.CHANNEL),             /* Channel related memory */
        CHANNELGROUP              = (1 << MEMTYPE.CHANNELGROUP),        /* ChannelGroup objects and internals */
        CODEC                     = (1 << MEMTYPE.CODEC),               /* Codecs allocated for streaming */
        FILE                      = (1 << MEMTYPE.FILE),                /* Codecs allocated for streaming */
        SOUND                     = (1 << MEMTYPE.SOUND),               /* Sound objects and internals */
        SOUND_SECONDARYRAM        = (1 << MEMTYPE.SOUND_SECONDARYRAM),  /* Sound data stored in secondary RAM */
        SOUNDGROUP                = (1 << MEMTYPE.SOUNDGROUP),          /* SoundGroup objects and internals */
        STREAMBUFFER              = (1 << MEMTYPE.STREAMBUFFER),        /* Stream buffer memory */
        DSPCONNECTION             = (1 << MEMTYPE.DSPCONNECTION),       /* DSPConnection objects and internals */
        DSP                       = (1 << MEMTYPE.DSP),                 /* DSP implementation objects */
        DSPCODEC                  = (1 << MEMTYPE.DSPCODEC),            /* Realtime file format decoding DSP objects */
        PROFILE                   = (1 << MEMTYPE.PROFILE),             /* Profiler memory footprint. */
        RECORDBUFFER              = (1 << MEMTYPE.RECORDBUFFER),        /* Buffer used to store recorded data from microphone */
        REVERB                    = (1 << MEMTYPE.REVERB),              /* Reverb implementation objects */
        REVERBCHANNELPROPS        = (1 << MEMTYPE.REVERBCHANNELPROPS),  /* Reverb channel properties structs */
        GEOMETRY                  = (1 << MEMTYPE.GEOMETRY),            /* Geometry objects and internals */
        SYNCPOINT                 = (1 << MEMTYPE.SYNCPOINT),           /* Sync point memory. */
        ALL                       = 0xffffffff                          /* All memory used by FMOD Ex */
    }

    /*
    [DEFINE]
    [
        [NAME]
        FMOD_EVENT_MEMBITS

        [DESCRIPTION]   
        Bitfield used to request specific memory usage information from the getMemoryInfo function of every public FMOD Event System class.<br>
        Use with the "event_memorybits" parameter of getMemoryInfo to get information on FMOD Event System memory usage.

        [REMARKS]
        Every public FMOD Event System class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
        The FMOD_EVENT_MEMBITS defines can be OR'd together to specify precisely what memory usage you'd like to get information on. See EventSystem::getMemoryInfo for an example.

        [PLATFORMS]
        Win32, Win64, Linux, Linux64, Macintosh, Xbox, Xbox360, PlayStation 2, GameCube, PlayStation Portable, PlayStation 3, Wii, Solaris

        [SEE_ALSO]
        FMOD_MEMBITS
        System::getMemoryInfo
    ]
    */
    public enum EVENT_MEMBITS :uint
    {
        EVENTSYSTEM           = (1 << (MEMTYPE.EVENTSYSTEM           - MEMTYPE.EVENTSYSTEM)), /* EventSystem and various internals */
        MUSICSYSTEM           = (1 << (MEMTYPE.MUSICSYSTEM           - MEMTYPE.EVENTSYSTEM)), /* MusicSystem and various internals */
        FEV                   = (1 << (MEMTYPE.FEV                   - MEMTYPE.EVENTSYSTEM)), /* Definition of objects contained in all loaded projects e.g. events, groups, categories */
        MEMORYFSB             = (1 << (MEMTYPE.MEMORYFSB             - MEMTYPE.EVENTSYSTEM)), /* Data loaded with registerMemoryFSB */
        EVENTPROJECT          = (1 << (MEMTYPE.EVENTPROJECT          - MEMTYPE.EVENTSYSTEM)), /* EventProject objects and internals */
        EVENTGROUPI           = (1 << (MEMTYPE.EVENTGROUPI           - MEMTYPE.EVENTSYSTEM)), /* EventGroup objects and internals */
        SOUNDBANKCLASS        = (1 << (MEMTYPE.SOUNDBANKCLASS        - MEMTYPE.EVENTSYSTEM)), /* Objects used to manage wave banks */
        SOUNDBANKLIST         = (1 << (MEMTYPE.SOUNDBANKLIST         - MEMTYPE.EVENTSYSTEM)), /* Data used to manage lists of wave bank usage */
        STREAMINSTANCE        = (1 << (MEMTYPE.STREAMINSTANCE        - MEMTYPE.EVENTSYSTEM)), /* Stream objects and internals */
        SOUNDDEFCLASS         = (1 << (MEMTYPE.SOUNDDEFCLASS         - MEMTYPE.EVENTSYSTEM)), /* Sound definition objects */
        SOUNDDEFDEFCLASS      = (1 << (MEMTYPE.SOUNDDEFDEFCLASS      - MEMTYPE.EVENTSYSTEM)), /* Sound definition static data objects */
        SOUNDDEFPOOL          = (1 << (MEMTYPE.SOUNDDEFPOOL          - MEMTYPE.EVENTSYSTEM)), /* Sound definition pool data */
        REVERBDEF             = (1 << (MEMTYPE.REVERBDEF             - MEMTYPE.EVENTSYSTEM)), /* Reverb definition objects */
        EVENTREVERB           = (1 << (MEMTYPE.EVENTREVERB           - MEMTYPE.EVENTSYSTEM)), /* Reverb objects */
        USERPROPERTY          = (1 << (MEMTYPE.USERPROPERTY          - MEMTYPE.EVENTSYSTEM)), /* User property objects */
        EVENTINSTANCE         = (1 << (MEMTYPE.EVENTINSTANCE         - MEMTYPE.EVENTSYSTEM)), /* Event instance base objects */
        EVENTINSTANCE_COMPLEX = (1 << (MEMTYPE.EVENTINSTANCE_COMPLEX - MEMTYPE.EVENTSYSTEM)), /* Complex event instance objects */
        EVENTINSTANCE_SIMPLE  = (1 << (MEMTYPE.EVENTINSTANCE_SIMPLE  - MEMTYPE.EVENTSYSTEM)), /* Simple event instance objects */
        EVENTINSTANCE_LAYER   = (1 << (MEMTYPE.EVENTINSTANCE_LAYER   - MEMTYPE.EVENTSYSTEM)), /* Event layer instance objects */
        EVENTINSTANCE_SOUND   = (1 << (MEMTYPE.EVENTINSTANCE_SOUND   - MEMTYPE.EVENTSYSTEM)), /* Event sound instance objects */
        EVENTENVELOPE         = (1 << (MEMTYPE.EVENTENVELOPE         - MEMTYPE.EVENTSYSTEM)), /* Event envelope objects */
        EVENTENVELOPEDEF      = (1 << (MEMTYPE.EVENTENVELOPEDEF      - MEMTYPE.EVENTSYSTEM)), /* Event envelope definition objects */
        EVENTPARAMETER        = (1 << (MEMTYPE.EVENTPARAMETER        - MEMTYPE.EVENTSYSTEM)), /* Event parameter objects */
        EVENTCATEGORY         = (1 << (MEMTYPE.EVENTCATEGORY         - MEMTYPE.EVENTSYSTEM)), /* Event category objects */
        EVENTENVELOPEPOINT    = (1 << (MEMTYPE.EVENTENVELOPEPOINT    - MEMTYPE.EVENTSYSTEM)), /* Event envelope point objects */
        ALL                   = 0xffffffff,                                                   /* All memory used by FMOD Event System */

        /* All event instance memory */
        EVENTINSTANCE_GROUP   = (EVENTINSTANCE | EVENTINSTANCE_COMPLEX | EVENTINSTANCE_SIMPLE | EVENTINSTANCE_LAYER | EVENTINSTANCE_SOUND),

        /* All sound definition memory */
        SOUNDDEF_GROUP        = (SOUNDDEFCLASS | SOUNDDEFDEFCLASS | SOUNDDEFPOOL)
    }
}