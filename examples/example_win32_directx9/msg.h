#ifndef MSG_H
#define MSG_H

#include <stdint.h>

#define MSG_ID_INVALID        0x00u
#define MSG_ID_CAR_STATE_REQ  0x31u
#define MSG_ID_CAR_STATE      0x32u
#define MSG_ID_ENGAGE_REQ     0x33u
#define MSG_ID_SET_REF        0x34u

#define LONGEST_MSG_SIZE_SERVER 7u

typedef struct carStateReq_t
{
    uint8_t msgId;
    uint8_t sequCntr;
}carStateReq_t;

typedef struct carState_t
{
    uint8_t msgId;
    uint8_t sequCntr;
    bool engaged;
    uint16_t carSpeed;
    uint16_t swa;
}carState_t;

typedef struct engageReq_t
{
    uint8_t msgId;
    uint8_t sequCntr;
    bool engage;
}engageReq_t;

typedef struct setRef_t
{
    uint8_t msgId;
    uint8_t sequCntr;
    uint16_t targetSpeed;
    uint16_t swaRef;
}setRef_t;

typedef struct seqCntrs_t
{
    uint8_t carStateReqCntr;
    uint8_t carStateCntr;
    uint8_t engangeReqCntr;
    uint8_t setRefCntr;
}seqCntrs_t;

#endif // !MSG_H

