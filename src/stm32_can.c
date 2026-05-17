#include "stm32_can.h"

// PB8  CAN_RX
// PB9  CAN_TX
volatile CanRxMsg CANRecMsg;// 接收缓冲区
volatile uint8_t  CANRecFlag = 0;

static void CAN_GPIO_Config(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;   	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
							          
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  								               
	
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
}

/***********CAN接收中断优先级配置**************/
static void CAN_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
	// CAN1_RX1_IRQn
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn; //引脚重定义用CAN1_RX1_IRQHandler
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		   //子优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void CAN_Mode_Config(void)
{
	CAN_InitTypeDef CAN_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
    // CAN寄存器初始化
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);
	
    // CAN单元初始化
	CAN_InitStructure.CAN_TTCM=DISABLE;			   // MCR-TTCM 时间触发通信模式使能
	CAN_InitStructure.CAN_ABOM=DISABLE;			   // MCR-ABOM 自动离线管理 
	CAN_InitStructure.CAN_AWUM=DISABLE;			   // MCR-AWUM 自动唤醒模式
	CAN_InitStructure.CAN_NART=DISABLE;			   // MCR-NART 禁止报文自动重传  DISABLE-自动重传
	CAN_InitStructure.CAN_RFLM=DISABLE;			   // MCR-RFLM 接收FIFO 锁定模式 DISABLE-溢出时新报文会覆盖原有报文  
	CAN_InitStructure.CAN_TXFP=ENABLE;			   // MCR-TXFP 发送FIFO优先级 DISABLE-优先级取决于报文标示符 
	CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;    // 正常发送模式

    // bps= Fpclk/((BRP+1)*((Tseg1+1)+(Tseg2+1)+1) ,bps=36000000/(45*(4+3+1))=100kHz														  //此处Tseg1+1 = CAN_BS1_8tp
    // 配置大方向: Tseg1>=Tseg2  Tseg2>=tq; Tseg2>=2TSJW
    // 波特率分频器 定义了时间单元的长度 72/(1+1)/(1+4+3)/18=250kbps
	// 波特率分频器 72000/(1+1)/(1+4+3)/36=125kbps
    CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;		   // BTR-SJW 重新同步跳跃宽度 21个时间单元
	CAN_InitStructure.CAN_BS1=CAN_BS1_8tq;		   // BTR-TS1 时间段1 占用了12个时间单元
	CAN_InitStructure.CAN_BS2=CAN_BS2_3tq;		   // BTR-TS1 时间段2 占用了3个时间单元
    CAN_InitStructure.CAN_Prescaler = 6;
    CAN_Init(CAN1, &CAN_InitStructure);
}
/***********CAN过滤器配置**************/
static void CAN_Filter_Config(void)
{
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
	
	CAN_FilterInitStructure.CAN_FilterNumber= 0;					// 过滤器组0
	CAN_FilterInitStructure.CAN_FilterMode  = CAN_FilterMode_IdMask;// 工作在标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;// 过滤器位宽为单个32位。
	
    CAN_FilterInitStructure.CAN_FilterIdHigh    = 0xFFFF;// 要过滤的ID高位
    CAN_FilterInitStructure.CAN_FilterIdLow     = 0xf000;// 要过滤的ID低位
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh= 0xFFFF;// 过滤器高16位每位必须匹配
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xFFFF;// 过滤器低16位每位必须匹配
     
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	// CAN通信中断使能
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}

// CAN初始化
void STM_CAN_Init(void)
{
	CAN_GPIO_Config();
	CAN_NVIC_Config();
	CAN_Mode_Config();
	CAN_Filter_Config();       
}

// 中断方式接收数据
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, (CanRxMsg*)&CANRecMsg);
	CANRecFlag = 1;
}


















