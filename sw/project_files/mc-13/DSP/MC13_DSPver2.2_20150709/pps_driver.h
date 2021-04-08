// ************************************************************
/*
     PPS GENERATED DRIVER SUPPORT FILE FOR [MC13_DSPver2.2_20150709]
     Generated 10 July 2015 11:27:19 
*/
// ************************************************************

// Constants and enums
typedef enum {
	PPS_FSI_8KHZ,
	PPS_FSI_11KHZ,
	PPS_FSI_16KHZ,
	PPS_FSI_22KHZ,
	PPS_FSI_24KHZ,
	PPS_FSI_32KHZ,
	PPS_FSI_44KHZ,
	PPS_FSI_48KHZ,
	PPS_FSI_88KHZ,
	PPS_FSI_96KHZ,
	PPS_FSI_176KHZ,
	PPS_FSI_192KHZ,
} PPS_FSI;


//INSTRUCTIONS & COEFFICIENTS
typedef struct {
    u8 reg_off;
    u8 reg_val;
} reg_value;

// Command Code Structure
typedef struct {
	int ncmd;           // # Commmands in structure
	reg_value *data;    // Transition data
} pps_commands;

// Command Transition structure
typedef struct {
	PPS_FSI sample_rate;        // Supported sample rate
	u8 supported_stchg;      // State changes supported
	u8 patched_stchg;        // Patch (?)
	pps_commands *stchg[7];   // Type 1-7 transitions
} pps_transitions;

// Control code Structure
typedef int (*CNTL_FCN)(int fs,int nparam, reg_value *params);

typedef struct {
	int book;
	int page;
	int reg;
} pps_cntl_param;

typedef struct {
	char *name;         // User based control name
	CNTL_FCN fcn;       // TI driver control function
	int nparam;         // # parameters for control
	pps_cntl_param params[];   // Control params
} pps_control;

// Configuration Structure
typedef struct {
	char *name;                 // Config name
	pps_transitions *patch[12];  // State transitions for supported rates
} pps_configuration;

// Overlay structure
typedef struct {
	char *name;                 // Overlay name
	int ncfg;                   // # Configs
	int ncntl;                  // # Controls in this overlay
	int initial_cfg;            // Startup config
	pps_transitions *main[12];  // Transitions to this overlay at sample rates
	pps_control *ctrl[5];       // Controls available
	pps_configuration *cfg[];   // Configurations supported
} pps_overlay;

// PFW structure
typedef struct {
	char *name;             // PFW name
	int novly;              // # Overlays
	u16  supported_fs;      // Supported sample rates
	int initial_ovly;       // Initial overlay
	pps_overlay *ovly[];    // Overlay list
} pps_pfw;

// ------ PFW Reference ------
extern pps_pfw MC13_DSPver2.2_20150709;
//----------------------------

