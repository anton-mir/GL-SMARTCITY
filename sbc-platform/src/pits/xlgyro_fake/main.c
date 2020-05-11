#include <stdio.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */
#include <errno.h>   /* ERROR Number Definitions           */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#define DATA_BUF_SIZE               (1024)
#define DATA_BUFS_NUM               (2)
#define PACKET_PREAMBULE            (0xAA55)
#define TX_BUF_SIZE                 (1024 * 6)
#define FIELD_SIZEOF(t, f)          (sizeof(((t*)0)->f))
#define LSM9DS1_SLIDING_WINDOW_SIZE             (32)
#define LSM9DS1_I2C_WRITE_BUFFER_SIZE           (32)
typedef enum
{
    E_X_AXIS = 0,
    E_Y_AXIS,
    E_Z_AXIS,
    E_AXIS_COUNT
} AXISES;
typedef struct RAW_DATA_STRUCT
{
    int16_t rawData[E_AXIS_COUNT];
} RAW_DATA_S;
typedef struct __attribute__((packed, aligned(1))) DATA_PACKET_STRUCT
{
    uint16_t preambule1;
    uint16_t preambule2;
    uint16_t readySamplesNum;
    struct
    {
        RAW_DATA_S aValue;
        RAW_DATA_S gValue;
    } bufs[DATA_BUF_SIZE];
} DATA_PACKET_S;
typedef struct __attribute__((packed, aligned(1))) CIRCULAR_BUF_STRUCT
{
    uint32_t activeBufIdx;
    DATA_PACKET_S payload[DATA_BUFS_NUM];
} CIRCULAR_BUF_S;
CIRCULAR_BUF_S data = { 0 };
#define DATA_PACKET_LEN_NO_CRC(itms)  ( \
                                            FIELD_SIZEOF(DATA_PACKET_S, preambule1) + \
                                            FIELD_SIZEOF(DATA_PACKET_S, preambule2) + \
                                            FIELD_SIZEOF(DATA_PACKET_S, readySamplesNum) + \
                                            sizeof(RAW_DATA_S) * itms * 2 )
typedef void (*RAW_DATA_APPEND_FP)(RAW_DATA_S *pAccelValue, RAW_DATA_S *pgyroValue);
typedef struct RAW_SLIDING_WINDOW_STRUCT
{
    RAW_DATA_S window[LSM9DS1_SLIDING_WINDOW_SIZE];
    int32_t windowSum[E_AXIS_COUNT];
    bool inited;
} RAW_SLIDING_WINDOW_S;
static RAW_SLIDING_WINDOW_S xlSlidingWindow = { 0 };
static RAW_SLIDING_WINDOW_S gSlidingWindow = { 0 };
uint8_t txBuf[TX_BUF_SIZE] = { 0 };
int fd;/*File Descriptor*/
void DataBufValuesAppend(RAW_DATA_S *pAccelValue, RAW_DATA_S *pGyroValue)
{
    const uint32_t idx = data.activeBufIdx;
    const uint32_t sampleIdx = data.payload[idx].readySamplesNum;

    if (pAccelValue != NULL && pGyroValue != NULL)
    {
        if (sampleIdx < DATA_BUF_SIZE)
        {
            memcpy(
                &data.payload[idx].bufs[sampleIdx].aValue,
                pAccelValue,
                sizeof(RAW_DATA_S));

            memcpy(
                &data.payload[idx].bufs[sampleIdx].gValue,
                pGyroValue,
                sizeof(RAW_DATA_S));

            data.payload[idx].readySamplesNum++;
        }        
    }
}

static bool Open_Port(char *port){
    printf("\n +----------------------------------+");
    printf("\n |        Serial Port Write         |");
    printf("\n +----------------------------------+");

    /*------------------------------- Opening the Serial Port -------------------------------*/
    fd = open(port,O_RDWR | O_NOCTTY | O_NDELAY);	
    /* O_RDWR Read/Write access to serial port           */
    /* O_NOCTTY - No terminal will control the process   */
    /* O_NDELAY -Non Blocking Mode,Does not care about-  */
    /* -the status of DCD line,Open() returns immediatly */

    if(fd == -1)						/* Error Checking */{
        printf("\n  Error! in Opening %s",port);
        return  1;
    }
    else
        printf("\n  %s Opened Successfully ",port);


    /*---------- Setting the Attributes of the serial port using termios structure --------- */

    struct termios config;	/* Create the structure                          */

    /* Input flags - Turn off input processing
     * convert break to null byte, no CR to NL translation,
     * no NL to CR translation, don't mark parity errors or breaks
     * no input parity check, don't strip high bit off */
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP);
    /* Disable xon/xoff ctrl */
    config.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* Output flags - Turn off output processing
     * no CR to NL translation, no NL to CR-NL translation,
     * no NL to CR translation, no column 0 CR suppression,
     * no Ctrl-D suppression, no fill characters, no case mapping,
     * no local output processing */
    config.c_oflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);

    /* no signaling chars, no echo,
     * no canonical processing
     * no remapping, no delays */
    config.c_lflag = 0;

    /* Turn off character processing
     * clear current char size mask, no parity checking,
     * no output processing, force 8 bit input */
    config.c_cflag &= ~(CSIZE);
    config.c_cflag |= CS8;
    /* gnore modem controls,
     * nable reading */
    config.c_cflag |= (CLOCAL | CREAD);
    /* Disable parity */
    config.c_cflag &= ~(PARENB | PARODD);
    config.c_cflag |= 0;
    config.c_cflag &= ~(CSTOPB | CRTSCTS);

    config.c_cc[VMIN]  = 0xFF;
    config.c_cc[VTIME] = 15;    // 15 * 10ms = 150ms

    cfsetispeed(&config, B115200);

}
static void Close_Port(){
        close(fd);/* Close the Serial port */
}
static void send (uint8_t *pData, uint16_t Size){

    /*------------------------------- Write data to serial port -----------------------------*/
     write(fd,pData,Size);/* use write() to send data to port                                            */
    /* "fd"                   - file descriptor pointing to the opened serial port */
    /*	"write_buffer"         - address of the buffer containing data	            */
    /* "sizeof(write_buffer)" - No of bytes to write                               */
    printf("Written to port");
    printf("\n +----------------------------------+\n");


}
static void dataBufSend(CIRCULAR_BUF_S *pData)
{
    const uint32_t idx = pData->activeBufIdx;
    const uint16_t samples = pData->payload[idx].readySamplesNum;
    uint32_t payloadLen = 0;
    if (samples > 0)
    {
        pData->activeBufIdx++;
        if (pData->activeBufIdx >= DATA_BUFS_NUM)
        {
            pData->activeBufIdx = 0;
        }

        pData->payload[idx].preambule1 = PACKET_PREAMBULE;
        pData->payload[idx].preambule2 = PACKET_PREAMBULE;
        payloadLen = DATA_PACKET_LEN_NO_CRC(samples);
        if ((payloadLen + 4) <= TX_BUF_SIZE)    // +2 for CRC
        {
            memcpy(txBuf, (uint8_t*)&pData->payload[idx], payloadLen);
        }
        //    crc16 = CalcCrc16(txBuf, payloadLen);
        //    txBuf[payloadLen] = (uint8_t)(crc16 >> 8);
        //    txBuf[payloadLen + 1] = (uint8_t)(crc16);
        txBuf[payloadLen] = 0xA5;
        txBuf[payloadLen + 1] = 0xA5;
        txBuf[payloadLen + 2] = 0xA5;
        txBuf[payloadLen + 3] = 0xA5;

        send(txBuf,(payloadLen + 4));

        pData->payload[idx].readySamplesNum = 0;

        return ;
    }
}
void LSM9DS1_AccelReadRawData(RAW_DATA_S *pRawData)
{
    uint8_t i2cBuf[LSM9DS1_I2C_WRITE_BUFFER_SIZE];
    memset(i2cBuf, 0, LSM9DS1_I2C_WRITE_BUFFER_SIZE);

    do
    {
        if (pRawData == NULL)
        {
            break;
        }
        pRawData->rawData[E_X_AXIS] = (rand() << 8 | rand());
        pRawData->rawData[E_Y_AXIS] = (rand() << 8 | rand());
        pRawData->rawData[E_Z_AXIS] = (rand() << 8 | rand());

    }while(0);

}
void  LSM9DS1_GyroReadRawData(RAW_DATA_S *pRawData)
{
    uint8_t i2cBuf[LSM9DS1_I2C_WRITE_BUFFER_SIZE];
    memset(i2cBuf, 0, LSM9DS1_I2C_WRITE_BUFFER_SIZE);
    do
    {
        if (pRawData == NULL)
        {
            break;
        }

        pRawData->rawData[E_X_AXIS] = (rand() << 8 | rand());
        pRawData->rawData[E_Y_AXIS] = (rand() << 8 | rand());
        pRawData->rawData[E_Z_AXIS] = (rand() << 8 | rand());


    }while(0);

}
static void LSM9DS1_slidingWindowPush(RAW_SLIDING_WINDOW_S *pWindow, RAW_DATA_S *pValue)
{
    pWindow->windowSum[E_X_AXIS] = 0;
    pWindow->windowSum[E_Y_AXIS] = 0;
    pWindow->windowSum[E_Z_AXIS] = 0;

    if (pWindow->inited != true)
    {
        /* If sliding window is not inited - fill it with frist given value */
        for (uint32_t i = 0; i < LSM9DS1_SLIDING_WINDOW_SIZE; ++i)
        {
            memcpy(&pWindow->window[i], pValue, sizeof(RAW_DATA_S));

            pWindow->windowSum[E_X_AXIS] += pWindow->window[i].rawData[E_X_AXIS];
            pWindow->windowSum[E_Y_AXIS] += pWindow->window[i].rawData[E_Y_AXIS];
            pWindow->windowSum[E_Z_AXIS] += pWindow->window[i].rawData[E_Z_AXIS];
        }

        pWindow->inited = true;
    }
    else
    {
        for (uint32_t i = (LSM9DS1_SLIDING_WINDOW_SIZE - 1); i > 0; --i)
        {
            pWindow->window[i] = pWindow->window[i - 1];
            pWindow->windowSum[E_X_AXIS] += pWindow->window[i].rawData[E_X_AXIS];
            pWindow->windowSum[E_Y_AXIS] += pWindow->window[i].rawData[E_Y_AXIS];
            pWindow->windowSum[E_Z_AXIS] += pWindow->window[i].rawData[E_Z_AXIS];
        }

        memcpy(&pWindow->window[0], pValue, sizeof(RAW_DATA_S));

        pWindow->windowSum[E_X_AXIS] += pWindow->window[0].rawData[E_X_AXIS];
        pWindow->windowSum[E_Y_AXIS] += pWindow->window[0].rawData[E_Y_AXIS];
        pWindow->windowSum[E_Z_AXIS] += pWindow->window[0].rawData[E_Z_AXIS];
    }
}
static uint32_t LSM9DS1_PollDataBlocking(RAW_DATA_APPEND_FP fpDataAppendCallback)
{
    uint32_t ret = 0;
    uint8_t samples = 0;
    RAW_DATA_S accelData = { 0 };
    RAW_DATA_S gyroData = { 0 };

    do
    {
        samples = 32;

        if (samples > 0)
        {
            ret = samples;
            do
            {
                LSM9DS1_AccelReadRawData(&accelData);

                LSM9DS1_GyroReadRawData(&gyroData);

                LSM9DS1_slidingWindowPush(&xlSlidingWindow, &accelData);
                LSM9DS1_slidingWindowPush(&gSlidingWindow, &gyroData);

                if (fpDataAppendCallback != NULL)
                {
                    fpDataAppendCallback(&accelData, &gyroData);
                }

                samples--;
            } while (samples > 0);
        }
    } while (0);

    return ret;
}

int main(int argc, char * argv[])
{
    if(argc==2){        
    if(Open_Port(argv[1])==0){
    while(1){
        (void)LSM9DS1_PollDataBlocking(DataBufValuesAppend);
        dataBufSend(&data);
        sleep(1);
    }
    Close_Port();
    }
    }else{
    printf("Example: ./xlgyro_fake_input /dev/ttyUSB0\n When \"/dev/ttyUSB0\" - any port\n");    
    }
}


