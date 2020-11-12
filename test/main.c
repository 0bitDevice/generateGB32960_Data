#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLOUDPKG_MAXSIZE            512
#define CLOUDMSG_SIZE(datasize)     (2 + 2 + VINCODE_SIZE + 3 + datasize + 1)
#define VINCODE_SIZE                17

#define MOTOR_CNTMAX	4
#define CLOUDMSG_MOTOINFO_SIZE(msg)	(msg.count * 12 + 1)

typedef unsigned char       boolean;        /* for use with TRUE/FALSE      */
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned long       uint32;
typedef unsigned long long  uint64;
typedef signed char         sint8;
typedef short               sint16;
typedef long                sint32;
typedef long long           sint64;

enum cloudcmd_t
{
    CMD_VEH_SIGNIN = 0x01,
    CMD_RT_UPLOAD = 0x02,
    CMD_FIX_UPLOAD = 0x03,
    CMD_VEH_SIGNOUT = 0x04,
    CMD_PLT_SIGNIN = 0x05,
    CMD_PLT_SIGNOUT = 0x06,
    CMD_EP_RESV0 = 0x07,
    CMD_EP_RESV1 = 0x08,
    CMD_UPLOAD_RESV = 0x09,
    //RESV to 0x7F
    CMD_EP_RESV2 = 0x80,
    //RESV to 0x82
    CMD_DOWNLOAD_RESV = 0x83,
    //RESV to 0xBF
    CMD_PLT_RESV = 0xC0,
    //RESV to 0xFE
};

enum cloudack_t
{
    ACK_OK = 0x01,
    ACK_ERR = 0x02,
    ACK_DUPVIN = 0x03,
    ACK_CMD = 0xFE,//notify this is a cmd
};

enum cloudencript_t
{
    DAT_CRIPT_NONE = 0x01,
    DAT_CRIPT_RSA = 0x02,
    DAT_CRIPT_AES128 = 0x03,
    DAT_CRIPT_ERR = 0xFE,
};

typedef struct _gb32960DataTimePack
{
	//use GMT +8 Beijing Time
	uint8 year; //0-99
	uint8 mon;
	uint8 day;
	uint8 hour;
	uint8 min;
	uint8 sec;
}gb32960DataTimePack;

typedef struct _gb32960DataPack
{
    uint8   head[2];
    uint8   cmd;
    uint8   ack;
    uint8   vincode[VINCODE_SIZE];
    uint8   encryptmode;
    uint16  datalen;
    uint8   data[CLOUDPKG_MAXSIZE - CLOUDMSG_SIZE(0)];
    uint8   bcc;
}gb32960DataPack;

typedef struct _gb32960DataFullVehiclePack
{
	uint8   vehiclestate;
	uint8   chgstate;
	uint8   runmode;

    uint8   SOCvalue;
	//0 - 100       0xFE = ERR
	uint8   dcdcstate;
	//
	uint8   gearstate;
    //
	uint16  speed;
	//speed = km/h * 10             100km/s -- 1000
	uint32  accumulationkms;
	//accumulationkms = km * 10     10km -- 100
	uint16  totalvol;
	//totalvol = V * 10             100V -- 1000
	uint16  totalcur;
	//totalcur = (A + 1000) * 10    100A -- 11000
	uint16  isolateres;
	//kohm
	uint8   reserved1;
	uint8   reserved2;

}gb32960DataFullVehiclePack;



typedef struct _gb32960DataMotorDetailPack
{
	uint8 seq;
	uint8 state;
	uint8 drivertemp;
	uint8 motortemp;

	uint16 speedrpm;
	//speed = rpm - 20000
	uint16 torque;
	//torque = N.M /10 - 20000

	uint16 drivervoltage;
	//vol = V*10            100V -- 1000
	uint16 drivercurrent;
	//cur = A*10 - 10000    100A -- 11000
}gb32960DataMotorDetailPack;

typedef struct _gb32960DataMotorPack
{
	uint8 count;
	gb32960DataMotorDetailPack motor[MOTOR_CNTMAX];
}gb32960DataMotorPack;

uint16 gb32960DataMotorPackToCharArray(gb32960DataMotorPack* src, uint8* pData)
{
	uint16 datsize = 0;

	if (src->count > MOTOR_CNTMAX)
	{
		return 0;
	}
	*pData++ = 0x02;        //类型标志
	*pData++ = src->count;
	++datsize;

	for (uint8 i = 0; i < src->count; ++i)
	{
       *pData++ = src->motor[i].seq;
       *pData++ = src->motor[i].state;
       *pData++ = src->motor[i].drivertemp;
       *pData++ = (src->motor[i].speedrpm) >> 8;
       *pData++ = (src->motor[i].speedrpm);
       *pData++ = (src->motor[i].torque) >> 8;
       *pData++ = (src->motor[i].torque);
       *pData++ = src->motor[i].motortemp;
       *pData++ = (src->motor[i].drivervoltage) >> 8;
       *pData++ = (src->motor[i].drivervoltage);
       *pData++ = (src->motor[i].drivercurrent) >> 8;
       *pData++ = (src->motor[i].drivercurrent);

       datsize += 12;
	}

	return datsize;
}


typedef struct _gb32960DataAlarmPack
{
	uint8   maxalarmlevel;
	uint32  commonalarmflag;
}gb32960DataAlarmPack;

uint16 gb32960DataPackToCharArray(gb32960DataPack* pDataPack,
                                  uint8* pCharArray)
{
    pCharArray[0] = pDataPack->head[0];
    pCharArray[1] = pDataPack->head[1];
    pCharArray[2] = pDataPack->cmd;
    pCharArray[3] = pDataPack->ack;
    memcpy(&pCharArray[4], pDataPack->vincode, VINCODE_SIZE);
    pCharArray[21] = pDataPack->encryptmode;

    pCharArray[22] = pDataPack->datalen >> 8;
    pCharArray[23] = pDataPack->datalen;

    memcpy(&pCharArray[24], pDataPack->data, pDataPack->datalen);

    return 24 + pDataPack->datalen;
}

uint16 gb32960FullVehiclePackToCharArray(gb32960DataFullVehiclePack* pDataPack,
                                         uint8* pCharArray)
{
    pCharArray[0] = 0x01;           //类型标志
    pCharArray[1] =  pDataPack->vehiclestate;
    pCharArray[2] =  pDataPack->chgstate;
    pCharArray[3] =  pDataPack->runmode;
    pCharArray[4] =  pDataPack->speed >> 8;
    pCharArray[5] =  pDataPack->speed;
    pCharArray[6] =  pDataPack->accumulationkms >> 24;
    pCharArray[7] =  pDataPack->accumulationkms >> 16;
    pCharArray[8] =  pDataPack->accumulationkms >> 8;
    pCharArray[9] =  pDataPack->accumulationkms;
    pCharArray[10] = pDataPack->totalvol >> 8;
    pCharArray[11] = pDataPack->totalvol;
    pCharArray[12] = pDataPack->totalcur >> 8;
    pCharArray[13] = pDataPack->totalcur;
    pCharArray[14] = pDataPack->SOCvalue;
    pCharArray[15] = pDataPack->dcdcstate;
    pCharArray[16] = pDataPack->gearstate;
    pCharArray[17] = pDataPack->isolateres >> 8;
    pCharArray[18] = pDataPack->isolateres;
    pCharArray[19] = pDataPack->reserved1;
    pCharArray[20] = pDataPack->reserved2;

    return 21;
}

uint8 CalculateBCC(uint8* pData, uint16 len)
{
    uint8 bccCode = 0;
    uint16 i = 0;

    for(i = 0; i < len; ++i)
    {
        bccCode ^= pData[i];
    }

    return bccCode;
}

gb32960DataTimePack     testTimePack = {20, 11, 6, 12, 30, 22};
gb32960DataPack         testPack = {.head[0] = '#',
                                    .head[1] = '#',
                                    .cmd = CMD_RT_UPLOAD,
                                    .ack = ACK_CMD,
                                    .vincode = {0x4c,0x53,0x56,0x46,0x41,0x34,0x39,0x31,0x32,0x33,0x32,0x31,0x33,0x30,0x35,0x39,0x37},
                                    .encryptmode = DAT_CRIPT_NONE,
                                    };

gb32960DataPack         testACKPack = {.head[0] = '#',
                                    .head[1] = '#',
                                    .cmd = CMD_EP_RESV2,
                                    .ack = ACK_OK,
                                    .vincode = "LSVFA491232130597",
                                    .encryptmode = DAT_CRIPT_NONE,
                                    };

gb32960DataFullVehiclePack     testFullVehiclePack = { .vehiclestate = 0xF1,
                                                       .chgstate = 0xF2,
                                                       .runmode = 0xF3,
                                                       .speed = 0xFFF4,
                                                       .accumulationkms = 0xFFFFFFF6,
                                                       .totalvol = 0xFFF7,
                                                       .totalcur = 0xFFF8,
                                                       .SOCvalue = 0xF9,
                                                       .dcdcstate = 0xFA,
                                                       .gearstate = 0xFB,
                                                       .isolateres = 0xFFFC,
                                                       .reserved1 = 0xFD,
                                                       .reserved2 = 0x5A,
                                                       };

gb32960DataMotorPack        testDataMotorPack = {.count = 2,
                                                 .motor[0].seq = 1,
                                                 .motor[0].state = 0x03,
                                                 .motor[0].drivertemp = 75,
                                                 .motor[0].speedrpm = 0,
                                                 .motor[0].torque = 0,
                                                 .motor[0].motortemp = 76,
                                                 .motor[0].drivervoltage = 0,
                                                 .motor[0].drivercurrent = 0,

                                                 .motor[1].seq = 2,
                                                 .motor[1].state = 0x01,
                                                 .motor[1].drivertemp = 95,
                                                 .motor[1].speedrpm = 30000,
                                                 .motor[1].torque = 35000,
                                                 .motor[1].motortemp = 115,
                                                 .motor[1].drivervoltage = 500,
                                                 .motor[1].drivercurrent = 130,
                                                 };

gb32960DataAlarmPack        testDataAlarmPack = {.maxalarmlevel = 0x01,
                                                 .commonalarmflag = 0x0004
                                                 };

uint16 gb32960AlarmPackToCharArray(gb32960DataAlarmPack* pDataPack,
                                         uint8* pCharArray)
{
	uint32      memAddr = pCharArray;
	*pCharArray++ = 0x07;       //类型标志
    *pCharArray++ = pDataPack->maxalarmlevel;
    *pCharArray++ = pDataPack->commonalarmflag >> 24;
    *pCharArray++ = pDataPack->commonalarmflag >> 16;
    *pCharArray++ = pDataPack->commonalarmflag >> 8;
    *pCharArray++ = pDataPack->commonalarmflag;

    return pCharArray - memAddr;
}


int main()
{
    uint8   charArr[256] = {0};
    uint8   charFullVehicleArr[256] = {0};
    uint8   charMotorDataArr[256] = {0};
    uint8   charAlarmArr[10] = {0};

    uint16  dataLen = 0;
    uint8   bccCode = 0;
    int     i = 0;

    FILE*   fp;
    fp = fopen ("file.txt", "w+");

    testPack.datalen = 27;

    gb32960FullVehiclePackToCharArray(&testFullVehiclePack, charFullVehicleArr);

    memcpy(testPack.data, &testTimePack, 6);
    memcpy(&testPack.data[6], charFullVehicleArr, 21);

    dataLen = gb32960DataPackToCharArray(&testPack, charArr);
    bccCode = CalculateBCC(charArr, dataLen);

    for(i = 0; i < dataLen; ++i)
    {
        fprintf(fp, "%.2x ", charArr[i]);
    }
    fprintf(fp, "%.2x\n", bccCode);
    fprintf(fp, "-------------------\n");

    dataLen = gb32960DataPackToCharArray(&testACKPack, charArr);
    bccCode = CalculateBCC(charArr, dataLen);

    for(i = 0; i < dataLen; ++i)
    {
        fprintf(fp, "%.2x ", charArr[i]);
    }
    fprintf(fp, "%.2x\n", bccCode);
    fprintf(fp, "-------------------\n");


    dataLen = gb32960DataMotorPackToCharArray(&testDataMotorPack, charMotorDataArr);
    memcpy(&testPack.data[6], charMotorDataArr, dataLen);
    testPack.datalen = 6 + dataLen + 1;
    dataLen = gb32960DataPackToCharArray(&testPack, charArr);
    bccCode = CalculateBCC(charArr, dataLen);

    for(i = 0; i < dataLen; ++i)
    {
        fprintf(fp, "%.2x ", charArr[i]);
    }
    fprintf(fp, "%.2x\n", bccCode);
    fprintf(fp, "-------------------\n");

    dataLen = gb32960AlarmPackToCharArray(&testDataAlarmPack, charAlarmArr);
    memcpy(&testPack.data[6], charAlarmArr, dataLen);
    testPack.datalen = 6 + dataLen + 1;
    dataLen = gb32960DataPackToCharArray(&testPack, charArr);
    bccCode = CalculateBCC(charArr, dataLen);

    for(i = 0; i < dataLen; ++i)
    {
        fprintf(fp, "%.2x ", charArr[i]);
    }
    fprintf(fp, "%.2x\n", bccCode);
    fprintf(fp, "-------------------\n");

    fclose(fp);
    return 0;
}
