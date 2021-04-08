/**
 * @file        DebugSrv.h
 * @brief       this\ is\ debug\ \ server
 * @author      Dmitry.Abdulov
 * @date        2014-02-12
 * @copyright   Tymphany Ltd.
 */


#ifndef DEBUGSRV_H
#define	DEBUGSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "server.h"

#define CR 0x0D
#define END_OF_CMD_SYM  0x0D  /* caret return */

enum DebugSrvSignals
{
    //TIMEOUT_SIG = MAX_SIG,
    CMD_READY_CALLBACK_SIG = MAX_SIG + 1,
    CMD_WAKEUP_SIG,
    DEBUG_TIMEOUT_SIG,
    CLI_ADC_MMI_REQ_SIG,
    CLI_AUX_REQ_SIG,
};



typedef enum
{
    DEBUG_PRINT_EVT,
    DEBUG_SIM_DONE_EVT, /* not used now */
} eDebugEvt;


IND_EVT(DebugCmdEvt)
END_IND_EVT(DebugCmdEvt)

REQ_EVT(DebugEvt)
    eDebugEvt evt;
    uint8 length;
    uint8* pMsg;
END_REQ_EVT(DebugEvt)

RESP_EVT(DebugRespEvt)
    uint32 resp;   /* return value from UartDrv_Write() */
END_RESP_EVT(DebugRespEvt)

RESP_EVT(DebugSnkyResp)
END_RESP_EVT(DebugSnkyResp)


#define MAX_PRINTSTR_SIZE   (SIZE_OF_LARGE_EVENTS - sizeof(QEvt) - sizeof(QActive*))
REQ_EVT(DebugPrintEvt)
    uint8 size;
    char msg[MAX_PRINTSTR_SIZE - sizeof(uint8)];
END_REQ_EVT(DebugPrintEvt)


SUBCLASS(cDebugSrv, cServer)
METHODS
    /* public functions */
/* Implement these so the controller can launch the server */
void DebugSrv_StartUp(cPersistantObj *me);
void DebugSrv_ShutDown(cPersistantObj *me);

END_CLASS

void DebugSrvAJPrintf(const char *s, ...);
void DebugSrvInputSourceCtor();
/* to print out pMsg through debug pc tool */
void DebugSrvPrintStr(char* pMsg);
#ifdef	__cplusplus
}
#endif

#endif	/* DEBUGSRV_H */

