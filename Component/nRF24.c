/**
*   nRF24无线2.4G的驱动库，开启了自动应答功能(接收和发送的地址宽度要一样)，为DMA写的
*
*   1.DMA设置中发送DMA要设置为地址自增，结束不要自增，除非你接收是数组
*
*
*/
#include "nRF24.h"
#include "spi.h"
#include "gpio.h"
#include "stm32g431xx.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_spi.h"
#include <stdint.h>

/* SPI阻塞死等时长 */
#define WaitTime         500

/* nRF24 on the SPI3 */
#define CS0_Set           nRF_CSN0_Reset
#define CS0_Reset         nRF_CSN0_Set
#define CE0_Set           nRF_CE_Set
#define CE0_Reset         nRF_CE_Reset

/*======================================== nRF24-参数配置 ========================================*/
/* nRF24_Init()-初始化配置(看手册配置) */
#define NRF24L01_AA_Config                            (uint8_t)(0x3F) /*!< 接收通道的自动应答 = (都使能自动应答) >*/
#define NRF24L01_ENRXADDR_Config                      (uint8_t)(0x03) /*!< 接收通道的使能 = (只使能通道0和1)  >*/
#define NRF24L01_AW_Config                            (uint8_t)(0x03) /*!< 接收通道的地址宽度 = (5个字节) >*/
#define NRF24L01_RETR_Config                          (uint8_t)(0xFF) /*!< 接收通道的自动重传的次数与间隔时间 = (3次重传250µS间隔) >*/
#define NRF24L01_RF_CarrierFreq_Config                (uint8_t)(90)   /*!< 射频的载波频率2400MHz + NRF24L01_RF_Freq_Config = (2.490GHz) >*/
#define NRF24L01_RF_FreqAndTransPower_Config          (uint8_t)(0x24) /*!< 射频的通讯频率和发射功率配置 = (250K/-6dBm)>*/

/* (发送配置)发送数据到的接收地址，接收端的地址 */
#define nRF24_SendAddress               (uint64_t)(0xFFFFFFFFFE) /*!< 发送端的地址 >*/
#define nRF24_SendData_DataWidth        (uint8_t)(4) /*!< 发送数据的数据宽度 >*/

/* (接收配置)本机的接收地址 */
#define nRF24_RecAddress_00         (uint64_t)(0xFFFFFFFFFF)    
#define nRF24_RecAddress_01         (uint64_t)(0xFFFFFFFF00)
#define nRF24_RecAddress_02         (uint8_t)(0x00) /*!< 对应RecAddress_01的高八位 >*/
#define nRF24_RecAddress_03         (uint8_t)(0x00) /*!< 对应RecAddress_01的高八位 >*/
#define nRF24_RecAddress_04         (uint8_t)(0x00) /*!< 对应RecAddress_01的高八位 >*/
#define nRF24_RecAddress_05         (uint8_t)(0x00) /*!< 对应RecAddress_01的高八位 >*/
/* (接收配置)本机的接收数据的位宽(字节，最大32) */
#define nRF24_RxChannel0_DataWidth          (uint8_t)(4) /*!< 接收数据的数据宽度 >*/
#define nRF24_RxChannel1_DataWidth          (uint8_t)(4)
#define nRF24_RxChannel2_DataWidth          (uint8_t)(4)
#define nRF24_RxChannel3_DataWidth          (uint8_t)(4)
#define nRF24_RxChannel4_DataWidth          (uint8_t)(4)

/*<--------------------------------------------------------回调函数----------------------------------------------------------->*/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI3) 
  {
    /* 通知取数据 */
    CS0_Reset();
  }

 
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI3) 
  {
    /* 通知取数据 */
    CS0_Reset();
  }

 
}

/*<--------------------------------------------------------底层函数----------------------------------------------------------->*/

/**
*   @brief nRF的CS操作
*/
void nRF_Set_CS(nRF24_t *nRF_Handle)
{
  nRF_Handle->CSN_Set();
}
void nRF_Reset_CS(nRF24_t *nRF_Handle)
{
  nRF_Handle->CSN_Reset();
}

/**
*   @brief 发送单独的指令并接收状态值
*/
uint8_t nRF24_W_Command(nRF24_t *nRF_Handle, uint8_t Command)
{
  uint8_t send_Data = Command;
  uint8_t Rec_Data = 0x00;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY)   // Check is if ready
  {
    nRF_Handle->CSN_Set();
    HAL_SPI_TransmitReceive(nRF_Handle->SPI_nRF24_Handle, &send_Data, &Rec_Data,1, WaitTime);
    nRF_Handle->CSN_Reset();
  }

  return Rec_Data;
}

/**
*   @brief 写入字节
*/
nRF24_Status_t nRF24_W_Reg_Normal(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t send_Data)
{
  nRF24_Status_t err;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY)   // Check is if ready
  {
    uint8_t SendBuffer[2] = {NRF24L01_W_REGISTER|Reg, send_Data};

    nRF_Handle->CSN_Set();
    HAL_SPI_Transmit(nRF_Handle->SPI_nRF24_Handle, SendBuffer, 2, WaitTime);
    nRF_Handle->CSN_Reset();

    err = Ok;
  }
  else err = Unripe;

  return err;
}

/**
*   @brief 读取字节
*/
nRF24_Status_t nRF24_R_Reg_Normal(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Rec_Data)
{
  nRF24_Status_t err;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY)   // Check is if ready
  {
    uint8_t SendBuffer[2] = {NRF24L01_R_REGISTER|Reg, NRF24L01_NOP};
    uint8_t RecBuffer[2];

    nRF_Handle->CSN_Set();
    HAL_SPI_TransmitReceive(nRF_Handle->SPI_nRF24_Handle, SendBuffer, RecBuffer, 2, WaitTime);
    nRF_Handle->CSN_Reset();

    *Rec_Data = RecBuffer[1];

    err = Ok;
  }
  else err = Unripe;

  return err;
}

/**
 *  @brief 以五个数组方式写入数据
 */
nRF24_Status_t nRF24_W_Reg_FiveArr(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Arr)
{
  nRF24_Status_t err;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY) { // Check is if ready

    uint8_t Send_Arr[6];
    Send_Arr[0] = NRF24L01_W_REGISTER|Reg;
    Send_Arr[1] = Arr[0];
    Send_Arr[2] = Arr[1];
    Send_Arr[3] = Arr[2];
    Send_Arr[4] = Arr[3];
    Send_Arr[5] = Arr[4];

    nRF_Handle->CSN_Set();
    HAL_SPI_Transmit(nRF_Handle->SPI_nRF24_Handle, Send_Arr, 6, WaitTime);
    nRF_Handle->CSN_Reset();

    err = Ok;
  }
  else err = Unripe;

  return err;

}

/**
*   @brief 用DMA读取数组
*/
nRF24_Status_t nRF24_R_Array_DMA(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Data_Point, uint8_t Len)
{
  nRF24_Status_t err;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY)   // Check is if ready
  {
   uint8_t SendBuffer[1] = {NRF24L01_R_REGISTER|Reg};

    nRF_Handle->CSN_Set();
    HAL_SPI_Transmit(nRF_Handle->SPI_nRF24_Handle, SendBuffer, 1, WaitTime);
    HAL_SPI_Receive_DMA(nRF_Handle->SPI_nRF24_Handle, Data_Point, Len);

    err = Ok;
  }
  else err = Unripe;

  return err;
}

/**
 *  @brief 用DMA写入数组
 *  @param Data_Point 对数组Data_Point有严格要求，第一位必须空出来放地址！！！
 *  @param Len 长度要包含头部放地址的
 */
nRF24_Status_t nRF24_W_Array_DMA(nRF24_t *nRF_Handle, uint8_t Reg, uint8_t *Data_Point, uint8_t Len)
{
  nRF24_Status_t err;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY)   // Check is if ready
  {
    Data_Point[0] = NRF24L01_W_REGISTER|Reg;
    
    nRF_Handle->CSN_Set();
    HAL_SPI_Transmit_DMA(nRF_Handle->SPI_nRF24_Handle, Data_Point, Len);

    err = Ok;
  }
  else err = Unripe;

  return err;
}

/**
 *  @brief 清除TxFIFO
 */
void nRF24_Reset_TxFIFO(nRF24_t *nRF_Handle)
{
  nRF24_W_Command(nRF_Handle, NRF24L01_FLUSH_TX);
}

/**
 *  @brief 清除FxFIFO
 */
void nRF24_Reset_FxFIFO(nRF24_t *nRF_Handle)
{
  nRF24_W_Command(nRF_Handle, NRF24L01_FLUSH_RX);
}


/**
 *  @brief 获取nRF的状态值
 */
uint8_t nRF24_Get_State(nRF24_t *nRF_Handle)
{
  uint8_t Rec_Data = 0x00;

  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_STATUS, &Rec_Data);

  return Rec_Data;
}

/**
*   @brief TX_Payload提交要发送的数据
*/
nRF24_Status_t nRF24_Conduct_TX_Payload(nRF24_t* nRF_Handle, uint8_t *Data_Arr, uint8_t Len)
{
  nRF24_Status_t err;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY)   // Check is if ready
  {
    Data_Arr[0] = NRF24L01_W_TX_PAYLOAD;

    nRF_Handle->CSN_Set();
    HAL_SPI_Transmit(nRF_Handle->SPI_nRF24_Handle, Data_Arr, Len, WaitTime);
    nRF_Handle->CSN_Reset();

    err = Ok;
  }
  else err = Unripe;

  return err;
}

/**
*   @brief Rx_Payload接收收到的数据
*/
nRF24_Status_t nRF24_Read_Rx_Payload(nRF24_t* nRF_Handle, uint8_t *Data_Arr, uint8_t Len)
{
  nRF24_Status_t err;
  uint8_t Send_Command = NRF24L01_R_RX_PAYLOAD;

  if (nRF_Handle->SPI_nRF24_Handle->State == HAL_SPI_STATE_READY)   // Check is if ready
  {
    nRF_Handle->CSN_Set();
    HAL_SPI_TransmitReceive(nRF_Handle->SPI_nRF24_Handle, &Send_Command, Data_Arr, Len+1, WaitTime);
    nRF_Handle->CSN_Reset();

    err = Ok;
  }
  else err = Unripe;

  return err;

}

/*<--------------------------------------------------------应用函数----------------------------------------------------------->*/
/**
 *  @brief (工作模式)使nRF进入掉电模式
 */
nRF24_Status_t nRF24_PowerDOWN(nRF24_t *nRF_Handle)
{
  nRF24_Status_t err = Error;
  uint8_t Rec_Data = 0x00;
  uint8_t Rec_Check = 0x00;

  /* 失能CE */
  nRF_Handle->CE_Reset();

  /* 配置config */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Data);
  Rec_Data &= ~0x02;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, Rec_Data);

  /* 检查是否配置完成 */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Check);
  if (Rec_Check == Rec_Data) {
    err = Ok;
  }
  else {
    err = Error;
  }
  
  return err;
}

/**
 *  @brief (工作模式)使nRF进入准备模式一
 */
nRF24_Status_t nRF24_Standby1(nRF24_t *nRF_Handle)
{
  nRF24_Status_t err = Error;
  uint8_t Rec_Data = 0x00;
  uint8_t Rec_Check = 0x00;

  /* 失能CE */
  nRF_Handle->CE_Reset();

  /* 配置config */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Data);
  Rec_Data |= 0x02;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, Rec_Data);

  /* 检查是否配置完成 */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Check);
  if (Rec_Check == Rec_Data) {
    err = Ok;
  }
  else {
    err = Error;
  }
  
  return err;
}

/**
 *  @brief (工作模式)使nRF进入接收模式
 */
nRF24_Status_t nRF24_RecMode(nRF24_t *nRF_Handle)
{
  nRF24_Status_t err = Error;
  uint8_t Rec_Data = 0x00;
  uint8_t Rec_Check = 0x00;

  /* 失能CE */
  nRF_Handle->CE_Reset();

  /* 配置config */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Data);
  Rec_Data |= 0x03;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, Rec_Data);

  /* 检查是否配置完成 */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Check);
  if (Rec_Check == Rec_Data) {
    err = Ok;
    nRF_Handle->CE_Set();
  }
  else {
    err = Error;
    nRF_Handle->CE_Reset();
  }
  
  return err;
}

/**
 *  @brief (工作模式)使nRF进入发送模式
 */
nRF24_Status_t nRF24_SendMode(nRF24_t *nRF_Handle)
{
  nRF24_Status_t err = Error;
  uint8_t Rec_Data = 0x00;
  uint8_t Rec_Check = 0x00;

  /* 失能CE */
  nRF_Handle->CE_Reset();

  /* 配置config */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Data);
  Rec_Data |= 0x02;
  Rec_Data &= ~0x01;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, Rec_Data);

  /* 检查是否配置完成 */
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Rec_Check);
  if (Rec_Check == Rec_Data) {
    err = Ok;
    nRF_Handle->CE_Set();
  }
  else {
    err = Error;
    nRF_Handle->CE_Reset();
  }
  
  return err;
}

/**
 *  @brief 写接收设备端的地址（本机发送到对方的地址）
 *  @param Address 5个字节的接收端地址
 */
nRF24_Status_t nRF24_Set_TransAddress(nRF24_t *nRF_Handle, uint64_t Address)
{
  nRF24_Status_t err = Ok;
  uint8_t Arr[5];
  
  Arr[0] = (Address >> 32) & 0xFF;
  Arr[1] = (Address >> 24) & 0xFF;
  Arr[2] = (Address >> 16) & 0xFF;
  Arr[3] = (Address >> 8) & 0xFF;
  Arr[4] = (Address) & 0xFF;

  nRF24_W_Reg_FiveArr(nRF_Handle, NRF24L01_TX_ADDR, Arr);

  return err;
}

/**
 *  @brief 写本机的接收地址
 *  @param Num_Channel 接收地址通道号0~5
 *  @param Address 本机的接收端地址（2~5附属第1个的高八位）
 */
nRF24_Status_t nRF24_Set_RecAddress(nRF24_t *nRF_Handle, uint8_t Num_Channel, uint64_t Address)
{
  nRF24_Status_t err = Error;
  uint8_t Reg_Addr= NRF24L01_RX_ADDR_P0;
  uint8_t Arr[5];

  if (Num_Channel > 5) {
    err = Error;
    return err;
  }

  Arr[0] = (Address >> 32) & 0xFF;
  Arr[1] = (Address >> 24) & 0xFF;
  Arr[2] = (Address >> 16) & 0xFF;
  Arr[3] = (Address >> 8) & 0xFF;
  Arr[4] = (Address) & 0xFF;
  
  if (Num_Channel > 1) {

    Reg_Addr += Num_Channel;
    err = nRF24_W_Reg_Normal(nRF_Handle, Reg_Addr, (uint8_t)Address);
  }
  else {

    Reg_Addr += Num_Channel;
    err = nRF24_W_Reg_FiveArr(nRF_Handle, Reg_Addr, Arr);
  }
    
  return err;
}

/**
*   @brief (初始化一)新建nRF对象
*/
nRF24_Status_t nRF24_Object_Handle(nRF24_t* nRF_Handle, SPI_HandleTypeDef *SPI_nRF24_Handle)
{
  nRF24_Status_t err = Error;

  nRF_Handle->SPI_nRF24_Handle = SPI_nRF24_Handle;

  nRF_Handle->CSN_Set = (void*)(CS0_Set);
  nRF_Handle->CSN_Reset = (void*)(CS0_Reset);

  nRF_Handle->CE_Set = (void*)(CE0_Set);
  nRF_Handle->CE_Reset = (void*)(CE0_Reset);

  if ((nRF_Handle->SPI_nRF24_Handle) != NULL) err = Ok;
  
  return err;
}

/**
 *  @brief (初始化二)配置nRF
 */
nRF24_Status_t nRF24_Init(nRF24_t *nRF_Handle)
{
  uint8_t Send_Data = 0x00;
  uint8_t Check_Data = 0x00;

  /*-------------- (一)基本寄存器配置 --------------*/
  /* 复位Config寄存器 */
  Send_Data = 0x08;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_CONFIG, &Check_Data);
  if (Check_Data != Send_Data) return Error;

  /* 各通道的自动应答 */
  Send_Data = NRF24L01_AA_Config;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_EN_AA, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_EN_AA, &Check_Data);
  if (Check_Data != Send_Data) return Error;

  /* 使能接收通道，默认开启通道0和通道1 */
  Send_Data = NRF24L01_ENRXADDR_Config;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_EN_RXADDR, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_EN_RXADDR, &Check_Data);
  if (Check_Data != Send_Data) return Error;

  /* 设置接收地址位宽，默认5字节 */
  Send_Data = NRF24L01_AW_Config;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_SETUP_AW, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_SETUP_AW, &Check_Data);
  if (Check_Data != Send_Data) return Error;

  /* 设置自动重传的次数与间隔时间 */
  Send_Data = NRF24L01_RETR_Config;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_SETUP_RETR, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_SETUP_RETR, &Check_Data);
  if (Check_Data != Send_Data) return Error;

  /* 设置RF的载波频率：2.4xxGHz ，默认设置为2.405GHz*/
  Send_Data = NRF24L01_RF_CarrierFreq_Config;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_RF_CH, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_RF_CH, &Check_Data);
  if (Check_Data != Send_Data) return Error;

  /* 设置RF的通讯频率以及发射功率：1M/2M/250K ，默认设置为250K，0dBm*/
  Send_Data = NRF24L01_RF_FreqAndTransPower_Config;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_RF_SETUP, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_RF_SETUP, &Check_Data);
  if (Check_Data != Send_Data) return Error;

  /* 无论发送还是接收模式，都应该设置至少一个接收通道的地址 */
  Send_Data = nRF24_RxChannel0_DataWidth;
  nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_RX_PW_P0, Send_Data);
  nRF24_R_Reg_Normal(nRF_Handle, NRF24L01_RX_PW_P0, &Check_Data);
  if (Check_Data != Send_Data) return Error;
  nRF24_Set_RecAddress(nRF_Handle, 0, nRF24_RecAddress_00);
  // .....other channel

  /* 进入Standby1模式 */
  if(nRF24_Standby1(nRF_Handle) == Error) return Error;
  HAL_Delay(100);

  return Ok;
}

/**
 *  @brief (发送数据出去)nRF24指定地址发送数据
 */
nRF24_Status_t nRF24_SendData(nRF24_t *nRF_Handle, uint64_t Address, uint8_t *Data_Array, uint8_t Len)
{
  nRF24_Status_t err;

  nRF24_Set_TransAddress(nRF_Handle, Address);
  nRF24_Conduct_TX_Payload(nRF_Handle, Data_Array, Len);
  nRF24_Set_RecAddress(nRF_Handle, 0, Address);   // 因为是自动应答，要写入发送的地址，还要位宽一样

  /* 进入发送模式 */
  do {
  {
    err = nRF24_SendMode(nRF_Handle);
  }
  }while (err == Error);

  return err;
}

nRF24_Status_t nRF24_CheckState_Send(nRF24_t *nRF_Handle)
{
  nRF24_Status_t err;
  uint8_t State = 0x00;

  /* 轮询发送状态 */
  State = nRF24_Get_State(nRF_Handle);
  if (State & 0x20) {
    err =  Trans_Complete;
  }
  else if (State & 0x10) {
    err = Trans_Outtime;
  }
  else if (State & 0x01) {
    err = Trans_FIFO_Full;
    nRF24_Reset_TxFIFO(nRF_Handle);
  }
  else err = Trans_Wait;

  if (err == Trans_Complete)
  {
    State &= 0xF0;
    nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_STATUS, State);

    nRF24_Reset_TxFIFO(nRF_Handle);
    nRF24_Set_RecAddress(nRF_Handle, 0, nRF24_RecAddress_00);
  }

  return err;
}

/**
 *  @brief (接收数据回来)
 */
nRF24_Status_t nRF24_RecData(nRF24_t *nRF_Handle, uint8_t *Data_Array, uint8_t Len)
{
  uint8_t State = 0x00;

  State = nRF24_Get_State(nRF_Handle);
  if (State & 0x40) {

    nRF24_Read_Rx_Payload(nRF_Handle, Data_Array, Len);

    State &= 0x40;
    nRF24_W_Reg_Normal(nRF_Handle, NRF24L01_STATUS, State);

    nRF24_Reset_FxFIFO(nRF_Handle);

    return Rec_Complete;
  }

  return Rec_Wait;
}
