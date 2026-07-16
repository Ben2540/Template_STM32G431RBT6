#ifndef _nRF24_H_
#define _nRF24_H_

#include "main.h"
#include <stdint.h>

/*<! NRF24L01指令宏定义 */
#define NRF24L01_R_REGISTER			0x00	//读寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节读数据
#define NRF24L01_W_REGISTER			0x20	//写寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节写数据
#define NRF24L01_R_RX_PAYLOAD		0x61	//读Rx有效载荷，后续跟1~32字节读数据
#define NRF24L01_W_TX_PAYLOAD		0xA0	//写Tx有效载荷，后续跟1~32字节写数据
#define NRF24L01_FLUSH_TX			0xE1	//清空Tx FIFO所有数据，单独指令
#define NRF24L01_FLUSH_RX			0xE2	//清空Rx FIFO所有数据，单独指令
#define NRF24L01_REUSE_TX_PL		0xE3	//重新使用最后一次发送的有效载荷，单独指令
#define NRF24L01_R_RX_PL_WID		0x60	//读取Rx FIFO最前面一个数据包的宽度，后续跟1字节读数据，仅适用于动态包长模式
#define NRF24L01_W_ACK_PAYLOAD		0xA8	//写应答附带的有效载荷，高5位为指令码，低3位为通道号，后续跟1~32字节写数据，仅适用于应答附带载荷模式
#define NRF24L01_W_TX_PAYLOAD_NOACK	0xB0	//写Tx有效载荷，不要求应答，后续跟1~32字节写数据，仅适用于不要求应答模式
#define NRF24L01_NOP				0xFF	//空操作，单独指令，可以用读取状态寄存器

/*<! NRF24L01寄存器地址宏定义 */
#define NRF24L01_CONFIG				0x00	//配置寄存器，1字节
#define NRF24L01_EN_AA				0x01	//使能自动应答，1字节
#define NRF24L01_EN_RXADDR			0x02	//使能接收通道，1字节
#define NRF24L01_SETUP_AW			0x03	//设置地址宽度，1字节
#define NRF24L01_SETUP_RETR			0x04	//设置自动重传，1字节
#define NRF24L01_RF_CH				0x05	//射频通道，1字节
#define NRF24L01_RF_SETUP			0x06	//射频相关参数设置，1字节
#define NRF24L01_STATUS				0x07	//状态寄存器，1字节
#define NRF24L01_OBSERVE_TX			0x08	//发送观察寄存器，1字节
#define NRF24L01_RPD				0x09	//接收功率检测，1字节
#define NRF24L01_RX_ADDR_P0			0x0A	//接收通道0地址，5字节
#define NRF24L01_RX_ADDR_P1			0x0B	//接收通道1地址，5字节
#define NRF24L01_RX_ADDR_P2			0x0C	//接收通道2地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P3			0x0D	//接收通道3地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P4			0x0E	//接收通道4地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P5			0x0F	//接收通道5地址，1字节，高位地址与接收通道1相同
#define NRF24L01_TX_ADDR			0x10	//发送地址，5字节
#define NRF24L01_RX_PW_P0			0x11	//接收通道0有效载荷数据宽度，1字节
#define NRF24L01_RX_PW_P1			0x12	//接收通道1有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P2			0x13	//接收通道2有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P3			0x14	//接收通道3有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P4			0x15	//接收通道4有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P5			0x16	//接收通道5有效载荷的数据宽度，1字节
#define NRF24L01_FIFO_STATUS		0x17	//发送和接收FIFO状态，1字节
#define NRF24L01_DYNPD				0x1C	//使能接收通道的动态包长模式，1字节
#define NRF24L01_FEATURE			0x1D	//使能高级功能，1字节

/* Claim */
struct nRF24_t;
enum nRF24_Status_t;

/**
*   @brief nRF24返回的状态
*/
typedef enum {

    Error = 0x00, /*<! 报错 >*/
    Ok,           /*<! OK >*/
    Unripe,       /*<! 未准备好 >*/

    Trans_Complete, /*<! 发送完成 >*/
    Trans_FIFO_Full,    /*<! 发送错误 >*/
    Trans_Outtime,  /*<! 发送超时 >*/
    Trans_Wait,

    Rec_Complete,   /*<! 接收完成 >*/
    Rec_Error,      /*<! 接收错误 >*/
    Rec_Wait,

} nRF24_Status_t;

/**
*   @brief nRF24对象
*/
typedef struct {

    SPI_HandleTypeDef *SPI_nRF24_Handle;    //<! SPI对象

    /* CS引脚 */
    void (*CSN_Set)(void);
    void (*CSN_Reset)(void);

    /* CE引脚 */
    void (*CE_Set)(void);
    void (*CE_Reset)(void);

} nRF24_t;

/*< Base function */
void nRF_Set_CS(nRF24_t *nRF_Handle);
void nRF_Reset_CS(nRF24_t *nRF_Handle);

uint8_t nRF24_W_Command(nRF24_t *nRF_Handle, uint8_t Command);
nRF24_Status_t nRF24_R_Reg_Normal(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Rec_Data);
nRF24_Status_t nRF24_W_Reg_Normal(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t send_Data);
nRF24_Status_t nRF24_W_Reg_FiveArr(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Arr);

nRF24_Status_t nRF24_R_Array_DMA(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Data_Point, uint8_t Len);
nRF24_Status_t nRF24_W_Array_DMA(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Data_Point, uint8_t Len);

nRF24_Status_t nRF24_Set_TransAddress(nRF24_t *nRF_Handle, uint64_t Address);
nRF24_Status_t nRF24_Set_RecAddress(nRF24_t *nRF_Handle, uint8_t Num_Channel, uint64_t Address);

void nRF24_CleanFLag_TX(nRF24_t *nRF_Handle);
void nRF24_CleanFLag_RX(nRF24_t *nRF_Handle);
nRF24_Status_t nRF24_Conduct_TX_Payload(nRF24_t* nRF_Handle, uint8_t *Data_Arr, uint8_t Len);
nRF24_Status_t nRF24_Read_Rx_Payload(nRF24_t* nRF_Handle, uint8_t *Data_Arr, uint8_t Len);

/*< Application function */
nRF24_Status_t nRF24_Object_Handle(nRF24_t* nRF_Handle, SPI_HandleTypeDef *SPI_nRF24_Handle);
nRF24_Status_t nRF24_Init(nRF24_t *nRF_Handle);
nRF24_Status_t nRF24_PowerDOWN(nRF24_t *nRF_Handle);
nRF24_Status_t nRF24_Standby1(nRF24_t *nRF_Handle);
nRF24_Status_t nRF24_RecMode(nRF24_t *nRF_Handle);
nRF24_Status_t nRF24_SendMode(nRF24_t *nRF_Handle);

nRF24_Status_t nRF24_SendData(nRF24_t *nRF_Handle, uint64_t Address, uint8_t *Data_Array, uint8_t Len);
nRF24_Status_t nRF24_CheckState_Send(nRF24_t *nRF_Handle);
nRF24_Status_t nRF24_RecData(nRF24_t *nRF_Handle, uint8_t *Data_Array, uint8_t Len);


#endif