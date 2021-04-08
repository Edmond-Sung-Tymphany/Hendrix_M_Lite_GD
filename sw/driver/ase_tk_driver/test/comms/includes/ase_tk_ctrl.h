#ifndef _ASE_TK_CTRL_H_
#define _ASE_TK_CTRL_H_

/**
* @brief           init the ase_tk controller
* @param[in]    void
* @param[out]  void
*/
void ase_tk_ctrl_init();

/**
* @brief           de-init the ase_tk controller
* @param[in]    void
* @param[out]  void
*/
void ase_tk_ctrl_deinit();


/**
* @brief           run the ase_tk controller to scan the commands from MCU. e.g.  MCU=>ASE_TK_CTRL
* @param[in]    void
* @param[out]  void
*/
void ase_tk_ctrl_run();


/**
* @brief           excute the commands to MCU from user input, similate as a ase_tk module behaviour. e.g. ASE_TK_CTRL => MCU
* @param[in]    void
* @param[out]  void
*/
void ase_tk_ctrl_execute_cmd(char* cmd);


#endif
