/**
*  @file      Mainapp_pteCmdHandler.h
*  @brief     Mainapp_pteCmdHandler function
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef MAINAPP_PTETESTCMD_HANDLER_H
#define MAINAPP_PTETESTCMD_HANDLER_H

#ifdef  __cplusplus
extern "C" {
#endif


typedef enum
{
    PTE_TEST_CMD_ID_SOURCE_SWITCH  = 0,
    PTE_TEST_CMD_ID_VOLUME,
    PTE_TEST_CMD_ID_SET_BRIGHTNESS,
    PTE_TEST_CMD_ID_SET_WORK_MODE,

} ePteTestCmdIds;

REQ_EVT(PteTestCmdEvt)
    ePteTestCmdIds  pteTestCmd;
    BOOL            enable;
    uint32          param;
    uint32          param2;
END_REQ_EVT(PteTestCmdEvt)

void MainApp_PteCmdHandler(cMainApp * const me, QEvt const * const e);


#ifdef  __cplusplus
}
#endif

#endif  /* MAINAPP_PTETESTCMD_HANDLER_H */

