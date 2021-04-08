#ifndef __USBSRV_GPB_H__
#define __USBSRV_GPB_H__

typedef struct tagAse2FepEvt {
    QEvt super;
    QActive * sender;
    uint32_t param;
}Ase2FepEvt;

typedef struct tagAse2FepCmdEvt {
    QEvt super;
    QActive * sender;
    Proto_Core_AseFepMessage ase2fep_cmd;
}Ase2FepCmdEvt;

typedef struct tagAseReqEvt {
    QEvt super;
    QActive * sender;
    Proto_AseFep_Req ase2fep_req;
}AseReqEvt;

typedef struct tagFepRespAseEvt {
    QEvt super;
    uint32_t id;
    Proto_AseFep_ReqResp type;
    Proto_Core_GenericResponse_Status status;
}FepRespAseEvt;

typedef struct tagFepRespVerInfoEvt {
    QEvt super;
    uint32_t id;
    Proto_AseFep_ReqResp type;
    uint32_t    type_no;
    uint32_t    item_no;
    uint32_t    serial_no;
    uint32_t    hw_ver;
    uint32_t    btl_ver;
    uint32_t    app_ver;
    uint32_t    dsp_ver;
}FepRespVerInfoEvt;



void UsbSrv_SendString(char *msg);

#endif  // __USBSRV_GPB_H__

