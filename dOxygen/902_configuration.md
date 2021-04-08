# Configuration

## Brief overview

In order to scale the feature set provided by Tymphany Platform we need to enable a great amount of customization and configuration. 

We are doing some of that already by modularising our code since we only pull in the servers we need to fulfil our requirements. However this alone is not enough in a resource limited environment.

Servers (or services) can have **features** and **components**. Some features are standard (e.g. Audio service must have a PLAY/STOP feature) but others can be additional to the base requirements (e.g. BT service provided Audio Tones).

We configure services (and their features/components) through our front end. 
Currently we must manually create config files via a copy/paste & edit process. See the section on how to currently do this.

\ref FIG1 "Fig1" shows the standard configuration files. The .config files contain all the configuration elements to enable features, components and config the service. 
You'll notice there is a product.config file which is used to contain general features such as MCU type, product version & name, size of internal message queues etc...
Also note that between the product creation there is a service change and we have pulled in a new service and removed an old one.

\anchor FIG1 \image html config.png "Fig1. The current copy, paste and edit method."

Some example config:
    
product.config
    
    #define HAS_DEBUG
    #define HAS_LEDS
    #define HAS_AUDIO_CONTROL
    #define HAS_KEYS
    #define HAS_BLUETOOTH
    ....
    #define TP_PRODUCT "iBT150"
    ...
    #define NUM_OF_SMALL_EVENTS 10
    #define NUM_OF_MEDIUM_EVENTS 10

KeySrv.config
    
    static cAdcKeyDrv adcKeySet[NUM_OF_ADC_KEY];

    tKeySrvConfig keySrvConfig =
    {
        .timing.debounceTime = 60, /* 60ms */
        .timing.longPressTime = 1000,  /* 1S */
        .timing.veryLongPressTime = 8000, /* 10S */

        /* If repeatHoldTime[x] > 0, then after sent KEY_EVT_HOLD, post KEY_EVT_REPEAT in every repeatHoldTime[x] *
        * for the specific key until veryLongPressTime                                                            *
        * If repeatHoldTime[x] = 0, it means that it is not required to send KEY_EVT_REPEAT for the specific key. */
        .timing.repeatHoldTime[VOLUME_DOWN_KEY] =  1000, /* 1s  this value should be changed according to user cases in applicaiton */
        .timing.repeatHoldTime[VOLUME_UP_KEY] =  1000,   /* 1s this value should be changed according to user cases in applicaiton*/

        .keyboardNum = KEYBOARD_NUMBER,
        .keyboardSet =
        {
            {(cKeyDrv*)adcKeySet,   sizeof(cAdcKeyDrv),    &adcDrvForKey},
            /**
             * you can add other types of keyboard here
             */
        }
    };


    #define NUM_OF_COMB_KEYS    4
    static tCombKeyElem  combKeyGroup0[] =
    {
        {.pKeysId = VOLUME_UP_KEY,    .combKeyTrigEvt = KEY_EVT_DOWN},
        {.pKeysId = PREV_KEY,         .combKeyTrigEvt = KEY_EVT_DOWN}
    };

LedSrv.config

    #define ALWAYS_REPEAT INT8_MAX

    tPatternData patternConfig[PAT_MAX_NUMBER] =
    {
    /* ALL_OFF */
       {
        .periodTimeInMs = 0,
        .onTimeInMs     = 0,
        .repeatNumber   = ALWAYS_REPEAT,
        .color          = BLACK,
        .nextPattern    = PAT_MAX_NUMBER,
        .style          = SOLID_STYLE
        },

    /*  COLOR_RED */
        {
        .periodTimeInMs = 0,
        .onTimeInMs     = 0,
        .repeatNumber   = ALWAYS_REPEAT,
        .color          = RED,
        .nextPattern    = PAT_MAX_NUMBER,
        .style          = SOLID_STYLE
        },

    

Later this will be done through a GUI tool and scripts \ref FIG2 "Fig2"

\anchor FIG2 \image html configflow.png "Fig2. The proposed way to do this config in the future. We use hierarchical data to feed the config tool which can then be used to create a config template for a project"


## Dictionary - what is a service, feature and component

### Services
A service is a high level generalisation of some capabilities within a Tymphany Platform enabled product. Typically they refer to some service that is only available through available HW.

Examples:
* Audio (usually making use of a DSP, AMP, GPIOS for detection)
* BT (typically enabled through a Bluetooth module)
* Keys (through GPIOS/ADC's or attached keyboards)
* Allplay (through an Allplay connected module)

### Features
A feature is an optional and configurable behaviour of a service. This means it is something which is not necessarily a part of the "stock" service but can be turned on (or off) via a configuration flag. 

Examples:
* Multiple audio sources - This helps reduce code if only one source may be requested
* BT auto-reconnect disable - This feature allows the BT service to disable reconnect if the product does not require this service.
* Combination key sequences - Enable simultaneous or sequence key handling

Features may need additional data beyond a simple flag to enable the feature correctly. So in fact you may have feature *FLAGS* or feature *DATA*. 
Flags are typically defined through macros and data through being defined through instances of data structures.  
These should be defined and agreed within the service documentation. See documenting configuration.

### Components
Not really used as a concept. More of a place holder. For now think of them as LARGE features.

\section Naming Naming features and components

Since services are encapsulated in servers they are already covered in a naming convention. For features and components the following should be adhered to.

### Features

For flags:

    #define BT_AUDIO_TONE // BT is the services and AUDIO_TONE is the feature

For data you can make use of the same naming conventions that are used for types.
    
    tKeyComboKey comboKeys0// Key service combo key feature data. There may be more than one combo key in this case so we are numbering them.

    tBTNFCConnectTag btNFCTag // BT service NFC connection tag data

### Components

TBD - Currently a place holder. Think of them as BIG features which may need their own source files.

\section currentmethod Current method of creating new product configuration

Currently all feature data and flags for services (including general features) as stored in .config files under the project folder. When a new project is created we copy these and modify by hand.

### Documenting features

To be updated. 

