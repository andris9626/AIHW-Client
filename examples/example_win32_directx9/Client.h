#pragma once
#pragma comment (lib, "ws2_32.lib")
#include <iostream>
#include <thread>
#include <ctime>
#include "msg.h"
#include <d3d9.h>
#include <math.h>

#define CONV_KMPH_TO_CMPS(kmph)      (speed_t)round((float)(kmph) * 27.78f)
#define CONV_CMPS_TO_KMPH(cmps)      (speed_t)round((float)(cmps) * 0.036f)
#define CONV_DEG_TO_MIRAD(deg)       (swa_t)round((float)(deg) * 17.453f)
#define CONV_MIRAD_TO_DEG(mirad)     (swa_t)round((float)(mirad) * 0.0573f)
#define CONV_CLOCK_TO_S(clock)       (float)(((double)(clock) / (double)CLOCKS_PER_SEC))
#define CONV_CLOCK_TO_MS(clock)      (float)(((double)(clock) / (double)CLOCKS_PER_SEC) * 1000.f)

typedef int16_t speed_t;
typedef int16_t swa_t;

using namespace std;

class Client
{
public:
    static speed_t speed;
    static swa_t swa;
    speed_t speedRef;
    swa_t swaRef;
    bool isSelfDrive = false;

    Client();
    ~Client();
    void engageSelfDrive();

private:
    SOCKET out = INVALID_SOCKET;
    sockaddr_in server = { 0 };
    bool isClientReady = false;
    seqCntrs_t seqCntrs = { 1u, 1u, 1u, 1u };
    thread t1;
    thread t2;
    void startUDPClient();
    void sendMsg(uint8_t msgId, uint8_t* seqCntr, uint8_t* payload);
    void updateSeqCntr(uint8_t* cntr);
    void carStateReq();
    void setReferences(int16_t refSpeed, int16_t refSwa);
    void carStateReqTask();
};

