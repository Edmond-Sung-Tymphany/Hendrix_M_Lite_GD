/**
*  @file      uart_driver.c
*  @brief     uart driver to control serial port
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include     <stdio.h>      /*standard input and output*/
#include     <stdlib.h>     /*standard library*/
#include     <unistd.h>     /*Unix standard function*/
#include     "string.h"
#include     <fcntl.h>      /*file control*/
#include     <termios.h>    /*PPSIX terminal control */
#include     <errno.h>      /*error define */
#include     "uart_driver.h"

/* Debug switch */
#define UART_DRIVER_DEBUG_ENABLE
#ifdef UART_DRIVER_DEBUG_ENABLE
#define UART_DRIVER_DEBUG(x) {printf x;}
#else
#define UART_DRIVER_DEBUG(x)
#endif

#define UART_DRIVER_ERROR_ENABLE
#ifdef UART_DRIVER_ERROR_ENABLE
#define UART_DRIVER_ERROR(x) {printf x;}
#else
#define UART_DRIVER_ERROR(x)
#endif

/* allow user to select the UART port , disable it when developing */
//#define INPUT_PORT_SELECT
/* copy the UART setting from minicom (9600, 8N1), need more debug*/
#define MINICOM_SET


/* config */
static char UART_DEVICE[] = "/dev/ttyUSB0";
static int BAUD_RATE = 9600;
static int DATA_BIT = 8;
static int STOP_BIT = 1;
static int PARITY_BIT = 'N';


/* no need to change the config below */
static int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
static int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300};

static fd_set read_flag, write_flag;

#define FALSE  -1
#define TRUE   0

/* as the bit mark in tembits.h doesn't have the "0x", so the value will change to 8 Octal */
/* so we need to add the "0x" here */
#define _BIT_MARK(a)  (0x##a)
#define BIT_MARK(a)   _BIT_MARK(a)


static void test_block_read(int fd);

static int OpenDev(char *Dev);
static void set_speed(int fd, int speed);
static int set_Parity(int fd,int databits,int stopbits,int parity);
static void print_uart_setting(int fd);


int uart_driver_init()
{
    int fd;
#ifdef INPUT_PORT_SELECT
    while(1)
    {
        char str[50];
        char dev[100]= "/dev/";
        /* this has to be output */
        printf("Please input the serial port to use: (example: ttyUSB0)\r\n");
        fgets(str, 50, stdin);
        strncat(dev,str,strlen(str));
        dev[strlen(dev)-1] = 0;
        UART_DRIVER_DEBUG(("Uart_driver: dev is %s\r\n", dev));
        fd = OpenDev(dev);
        if(fd==-1)
        {
            UART_DRIVER_ERROR(("Uart_driver: device can NOT open! Please input again\r\n"));
        }
        else
        {
            break;
        }
    }
#else
    UART_DRIVER_DEBUG(("Uart_driver: opening... UART\r\n"));

    fd = OpenDev(UART_DEVICE);
    if(fd==-1)
    {
        UART_DRIVER_ERROR(("Uart_driver: device can NOT open! Please input again\r\n"));
        return -1;
    }
    //print_uart_setting(fd);
#endif

    /* uncomment it to debug uart setting */
    //print_uart_setting(fd);

#ifdef MINICOM_SET
    struct termios options;
    tcgetattr(fd, &options);
    options.c_iflag = 0x1;
    options.c_oflag = 0;
    options.c_cflag = 0x18b2;//0x8bd;
    options.c_lflag = 0;
    int status = tcsetattr(fd, TCSANOW, &options);
    if  (status != 0)
    {
        UART_DRIVER_ERROR(("Uart_driver: tcsetattr fd1"));
        return;
    }
#else
    set_speed(fd,BAUD_RATE);
    if (set_Parity(fd,DATA_BIT,STOP_BIT,PARITY_BIT) == FALSE)
    {
        UART_DRIVER_ERROR(("Uart_driver: Set Parity Error/n"));
        return -1;
    }
#endif
    UART_DRIVER_DEBUG(("Uart_driver: open UART device successfully\r\n"));
    /* uncomment it to debug uart setting */
    //print_uart_setting(fd);
    return fd;
}


void uart_driver_deinit(int fd)
{
    UART_DRIVER_DEBUG(("Uart_driver: close UART device\r\n"));
    close(fd);
}

int uart_driver_scan(int fd)
{
    struct timeval timeout;
    FD_ZERO(&read_flag);
    FD_SET(fd, &read_flag);
    FD_ZERO(&write_flag);
    FD_SET(fd, &write_flag);
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    return select(fd+1, &read_flag,&write_flag,NULL,&timeout);
}

int uart_driver_ready_to_read(int fd)
{
    return FD_ISSET(fd, &read_flag);
}

int uart_driver_ready_to_write(int fd)
{
    return FD_ISSET(fd, &write_flag);
}

int uart_driver_read(int fd, char* buff, int size)
{
    int number = read(fd, buff, size);
    return number;
}

//TODO: need to test the UART DRIVER sent
void uart_driver_write(int fd, char* buff, uint32 size)
{
    int number = write(fd, buff, size);
    if(number<=0)
    {
        UART_DRIVER_ERROR(("Uart_driver: fail to write/n"));
    }
    else
    {
        UART_DRIVER_DEBUG(("Uart_driver: send out %d bytes data\r\n", number));
        UART_DRIVER_DEBUG(("=============================================\r\n"));
    }

}


/***********************************************************/
/**************** Private Function *****************************/
/***********************************************************/
static int OpenDev(char *Dev)
{
    int fd = open( Dev, O_RDWR | O_NOCTTY);         //| O_NOCTTY | O_NDELAY
    if (-1 == fd)
    {
        UART_DRIVER_ERROR(("Uart_driver: Can't Open Serial Port\r\n"));
        return -1;
    }
    else
        return fd;
}
/**
*@brief  set the boardrate
*@param  fd
*@param  speed
*@return  void
*/
static void set_speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if  (status != 0)
            {
                UART_DRIVER_ERROR(("tcsetattr fd1\r\n"));
                return;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }
}
/**
*@brief
*@param  fd
*@param  databits
*@param  stopbits
*@param  parity
*/

static int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
    options.c_lflag  &= ~(BIT_MARK(ICANON) | BIT_MARK(ECHO) | BIT_MARK(ECHOE) | BIT_MARK(ISIG));  /*Input*/
    options.c_oflag  &= ~ BIT_MARK(OPOST);   /*Output*/
    if  ( tcgetattr( fd,&options)  !=  0)
    {
        UART_DRIVER_ERROR(("Uart_driver: SetupSerial 1\r\n"));
        return(FALSE);
    }
    options.c_cflag &= ~ BIT_MARK(CSIZE);
    switch (databits) /*??????*/
    {
        case 7:
            options.c_cflag |= BIT_MARK(CS7);
            UART_DRIVER_DEBUG(("Uart_driver: set 7 bit\r\n"));
            break;
        case 8:
            options.c_cflag |= BIT_MARK(CS8);
            UART_DRIVER_DEBUG(("Uart_driver: set 8 bit\r\n"));
            break;
        default:
            UART_DRIVER_ERROR(("Uart_driver: Unsupported data size\r\n"));
            return (FALSE);
    }
    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~ BIT_MARK(PARENB);   /* Clear parity enable */
            options.c_iflag &= ~ BIT_MARK(INPCK);     /* disable 'Enable parity check' bit */
            UART_DRIVER_DEBUG(("Uart_driver: parity n\r\n"));
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (BIT_MARK(PARODD) | BIT_MARK(PARENB)); /* ??????*/
            options.c_iflag |= BIT_MARK(INPCK);             /* Disnable parity checking */
            UART_DRIVER_DEBUG(("Uart_driver: parity o\r\n"));
            break;
        case 'e':
        case 'E':
            options.c_cflag |= BIT_MARK(PARENB);     /* Enable parity */
            options.c_cflag &= ~ BIT_MARK(PARODD);   /* ??????*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            UART_DRIVER_DEBUG(("Uart_driver: parity e\r\n"));
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~ BIT_MARK(PARENB);
            options.c_cflag &= ~ BIT_MARK(CSTOPB);
            UART_DRIVER_DEBUG(("Uart_driver: parity s\r\n"));
            break;
        default:
            UART_DRIVER_ERROR(("Uart_driver: Unsupported parity\r\n"));
            return (FALSE);
    }
    /* ?????*/
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~ BIT_MARK(CSTOPB);
            UART_DRIVER_DEBUG(("Uart_driver: stop bit 1\r\n"));
            break;
        case 2:
            options.c_cflag |= BIT_MARK(CSTOPB);
            UART_DRIVER_DEBUG(("Uart_driver: stop bit 2\r\n"));
            break;
        default:
            UART_DRIVER_ERROR(("Uart_driver: Unsupported stop bits\r\n"));
            return (FALSE);
    }
    /* Set input parity option */
    if ((parity != 'n')&&(parity != 'N'))
    {
        options.c_iflag |= BIT_MARK(INPCK);
        UART_DRIVER_DEBUG(("Uart_driver: enable parity check\r\n"));
    }
    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* ????15 seconds*/
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */

    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        UART_DRIVER_ERROR(("Uart_driver: SetupSerial 3\r\n"));
        return (FALSE);
    }
    return (TRUE);
}


static void print_uart_setting(int fd)
{
    struct termios options;
    tcgetattr(fd, &options);
    UART_DRIVER_DEBUG(("Uart_driver: print out the uart setting \r\n"));
    UART_DRIVER_DEBUG(("c_iflag: %x\r\n", options.c_iflag));
    UART_DRIVER_DEBUG(("c_oflag: %x\r\n", options.c_oflag));
    UART_DRIVER_DEBUG(("c_cflag: %x\r\n", options.c_cflag));
    UART_DRIVER_DEBUG(("c_lflag: %x\r\n", options.c_lflag));
}


/* example */
static void test_block_read(int fd)
{
    int i;
    int nread;
    char buff[512];
    while (1)
    {
        if((nread = read(fd, buff, 100))>0)
        {
            UART_DRIVER_DEBUG(("\nget data: "));
            for(i=0; i<nread; i++)
            {
                UART_DRIVER_DEBUG(( "%x", buff[i]));
            }
        }
        else
        {
            UART_DRIVER_DEBUG(("no data\r\n"));
        }
    }
}

static void test_unblock_read(int fd)
{
    fd_set read_flag;
    struct timeval timeout;
    int ret_val, number, i;
    char buff[512];
    FD_ZERO(&read_flag);
    FD_SET(fd, &read_flag);
    timeout.tv_sec=2;
    timeout.tv_usec=0;
    UART_DRIVER_DEBUG(("start to check read flags \n"));
    while(1)
    {
        ret_val = select(fd+1, &read_flag,NULL,NULL,&timeout);
        if(ret_val == -1)
        {
            UART_DRIVER_DEBUG(("error at select \n"));
            return;
        }
        else if(ret_val>0)
        {
            if(FD_ISSET(fd, &read_flag))
            {
                number = read(fd, buff, 100);
                if(number > 0)
                {
                    UART_DRIVER_DEBUG(("get %d data: ", number));
                    for(i=0; i<number; i++)
                    {
                        UART_DRIVER_DEBUG(( "%c", buff[i]));
                    }
                    UART_DRIVER_DEBUG(("\n"));
                }
                else
                {
                    UART_DRIVER_DEBUG(("no data is read \n"));
                }
            }
        }
        FD_ZERO(&read_flag);
        FD_SET(fd, &read_flag);
        timeout.tv_sec=2;
        timeout.tv_usec=0;
    }
}


