#include "Client.h"

Client::Client()
{
    t1 = thread(&Client::startUDPClient, this);
    t2 = thread(&Client::carStateReqTask, this);
}
Client::~Client()
{
    t1.join();
    t2.join();
}


void Client::updateSeqCntr(uint8_t* cntr)
{
    (*cntr)++;

    /* Avoid invalid counter value */
    if (0u == *cntr)
    {
        *cntr = 1u;
    }
}


void Client::startUDPClient()
{
    using namespace literals::chrono_literals;

    WSADATA data;

    // To start WinSock, the required version must be passed to
    // WSAStartup(). This server is going to use WinSock version
    // 2 so I create a word that will store 2 and 2 in hex i.e.
    // 0x0202
    WORD version = MAKEWORD(2, 2);

    // Start WinSock
    int wsOk = WSAStartup(version, &data);
    if (wsOk != 0)
    {
        // Not ok! Get out quickly
        cout << "Can't start Winsock! " << wsOk;
        return;
    }

    //sockaddr_in server;
    server.sin_family = AF_INET; // AF_INET = IPv4 addresses
    server.sin_port = htons(54000); // Little to big endian conversion
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Socket creation, note that the socket type is datagram
    out = socket(AF_INET, SOCK_DGRAM, 0);

    if (out != SOCKET_ERROR)
    {
        isClientReady = true;
    }
    else
    {
        cout << "Socket error: " << WSAGetLastError() << endl;
    }

    // Write out to that socket
    char msg[7u];

    int sendOk = sendto(this->out, msg, sizeof(msg), 0, (sockaddr*)&server, sizeof(server));

    if (sendOk == SOCKET_ERROR)
    {
        cout << "That didn't work! " << WSAGetLastError() << endl;
    }

    char buffer[10u];
    int len = sizeof(server);

    while (1)
    {
        int bytesIn = recvfrom(out, buffer, 10u, 0, (struct sockaddr*)&server, &len);
        if (bytesIn == SOCKET_ERROR)
        {
            continue;
        }
        /* Check if msgId is correct */
        if (MSG_ID_CAR_STATE == buffer[0])
        {
            /* Check if sequence counter is as expected */
            if (seqCntrs.carStateCntr != (uint8_t)buffer[1])
            {
                /* Print error to console */
                //cerr << "Sequence counter is not as expected! (expected: " << (seqCntrs->carStateCntr) <<
                   // ", received: " << (buffer[1]) << ")" << endl;
                printf("Sequence counter is not as expected! (expected: %u, received: %u\n",
                    (seqCntrs.carStateCntr), buffer[1]);

                /* Store received counter value */
                seqCntrs.carStateCntr = buffer[1];
            }
            else
            {
                /*printf("Sequence counter is as expected! (expected: %u, received: %u\n",
                    (seqCntrs->carStateCntr), buffer[1]);*/
            }

            /* Store data */
            isSelfDrive = buffer[2];
            speed = (buffer[3] << 8) | (buffer[4] & 0x0FF);
            swa = (buffer[5] << 8) | (buffer[6] & 0x0FF);

            /* Update expected counter */
            updateSeqCntr(&seqCntrs.carStateCntr);
        }

        /*printf("MsgId: 0x%02X\n", (unsigned char)buffer[0]);
        printf("SeqCntr: 0x%02X\n", (unsigned char)buffer[1]);
        printf("SelfDriveEng: 0x%02X\n", (unsigned char)buffer[2]);
        printf("Speed: 0x%04X = %d cm/s\n", speed, speed);
        printf("Swa: 0x%04X = %d mrad\n", swa, swa);*/
    }

    // Close the socket
    closesocket(out);

    WSACleanup();
}

void Client::sendMsg(uint8_t msgId, uint8_t* seqCntr, uint8_t* payload)
{
    if (true == isClientReady)
    {
        char msg[7u];

        msg[0u] = msgId;
        msg[1u] = *seqCntr;

        for (uint8_t i = 2u; i < 7u; i++)
        {
            msg[i] = payload[i - 2u];
        }

        int sendOk = sendto(out, msg, sizeof(msg), 0, (sockaddr*)&server, sizeof(server));

        if (sendOk == SOCKET_ERROR)
        {
            cout << "That didn't work! " << WSAGetLastError() << endl;
        }
        else
        {
            updateSeqCntr(seqCntr);
        }
    }
}

void Client::carStateReq()
{
    uint8_t payload[] = { 0u, 0u, 0u, 0u, 0u };
    sendMsg(MSG_ID_CAR_STATE_REQ, &seqCntrs.carStateReqCntr, payload);
}

void Client::engageSelfDrive()
{
    uint8_t payload[] = { true, 0u, 0u, 0u, 0u };
    sendMsg(MSG_ID_ENGAGE_REQ, &seqCntrs.engangeReqCntr, payload);
}

void Client::setReferences(int16_t refSpeed, int16_t refSwa)
{
    uint8_t speedHigh = refSpeed >> 8u;
    uint8_t speedLow = refSpeed;
    uint8_t swaHigh = refSwa >> 8u;
    uint8_t swaLow = refSwa;
    uint8_t payload[] = { speedHigh, speedLow, swaHigh, swaLow, 0u };

    /* Save references for periodic send */
    speedRef = refSpeed;
    swaRef = refSwa;

    sendMsg(MSG_ID_SET_REF, &seqCntrs.setRefCntr, payload);
}

void Client::carStateReqTask()
{
    clock_t currTime = clock();
    clock_t prevTime = 0u;

    while (1)
    {
        currTime = clock();

        if (50u <= (uint32_t)CONV_CLOCK_TO_MS(currTime - prevTime))
        {
            prevTime = currTime;
            carStateReq();

            /* If seld-drive is engeged send references perodically */
            if (true == isSelfDrive)
            {
                setReferences(speedRef, swaRef);
            }
        }
    }
}
