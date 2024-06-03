/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "misc_config.h"

#include "DataFlashProg.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/
#define PLL_CLOCK   							72000000

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TIMER_PERIOD_SPECIFIC                 (flag_PROJ_CTL.bit1)
#define FLAG_PROJ_SPI_MODE_0               				(flag_PROJ_CTL.bit2)
#define FLAG_PROJ_SPI_MODE_1                            (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_SPI_MODE_2                            (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_SPI_MODE_3                            (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_REVERSE6                              (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_REVERSE7                              (flag_PROJ_CTL.bit7)


/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_systick = 0;
volatile uint32_t counter_tick = 0;

uint32_t Storage_Block[BUFFER_PAGE_SIZE / 4] = {0};
uint32_t u32SPIMode = 0;
uint32_t record_idx = 0; 
#define SPI_FREQ 								        (100000ul)

#define MANUAL_SWITCH_MOSI_TO_GPIO

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

unsigned int get_systick(void)
{
	return (counter_systick);
}

void set_systick(unsigned int t)
{
	counter_systick = t;
}

void systick_counter(void)
{
	counter_systick++;
}

void SysTick_Handler(void)
{

    systick_counter();

    if (get_systick() >= 0xFFFFFFFF)
    {
        set_systick(0);      
    }

    // if ((get_systick() % 1000) == 0)
    // {
       
    // }

    #if defined (ENABLE_TICK_EVENT)
    TickCheckTickEvent();
    #endif    
}

void SysTick_delay(unsigned int delay)
{  
    
    unsigned int tickstart = get_systick(); 
    unsigned int wait = delay; 

    while((get_systick() - tickstart) < wait) 
    { 
    } 

}

void SysTick_enable(unsigned int ticks_per_second)
{
    set_systick(0);
    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        printf("Set system tick error!!\n");
        while (1);
    }

    #if defined (ENABLE_TICK_EVENT)
    TickInitTickEvent();
    #endif
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

void delay_ms(uint16_t ms)
{
	#if 1
    uint32_t tickstart = get_tick();
    uint32_t wait = ms;
	uint32_t tmp = 0;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000 -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
	
	#else
	TIMER_Delay(TIMER0, 1000*ms);
	#endif
}

void Write_SPI_Mode_Record(uint32_t mode)
{
    record_idx = 0x00;
	DataFlashRead(record_idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
    printf("target mode:0x%04X\r\n", mode);
    
    if (Storage_Block[0] == mode)
    {

        printf("%s:Read mode(NO change) 0x%04X\r\n" ,__FUNCTION__ , Storage_Block[0]);
        return;
    }
    else
    {
        Storage_Block[0] = mode;
		DataFlashWrite(record_idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
        printf("%s:Write mode 0x%04X\r\n" ,__FUNCTION__ , Storage_Block[0]);
        
	    DataFlashRead(record_idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
        printf("%s:Read mode[0]:0x%04X\r\n",__FUNCTION__ ,Storage_Block[0]);
    }

    printf("reboot\r\n");
    UART_WAIT_TX_EMPTY(UART0);
    SYS_UnlockReg();
    SYS_ResetChip();  

}

/*
    SPI_MODE_0:0x0004
    SPI_MODE_1:0x0002
    SPI_MODE_2:0x0802
    SPI_MODE_3:0x0804

    mode	    CPOL	CPHA
    mode 0	    0	    0
    mode 1	    0	    1
    mode 2	    1	    0
    mode 3	    1	    1

*/
void Check_SPI_Mode_Record(void)
{
    record_idx = 0x00;
	DataFlashRead(record_idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);

    printf("%s:[0]:0x%04X\r\n",__FUNCTION__ ,Storage_Block[0]);

    if ((Storage_Block[0] == SPI_MODE_0) || 
        (Storage_Block[0] == SPI_MODE_1) ||
        (Storage_Block[0] == SPI_MODE_2) ||
        (Storage_Block[0] == SPI_MODE_3)
        )
    {
        u32SPIMode = Storage_Block[0];
        printf("%s:Read SPI mode ready:0x%04X, u32SPIMode:0x%04X\r\n" ,__FUNCTION__ , Storage_Block[0],u32SPIMode);
        return;
    }
    else
    {
        Storage_Block[0] = SPI_MODE_0;
        u32SPIMode = SPI_MODE_0;

		DataFlashWrite(record_idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
        printf("%s:NOT correct SPI mode,set to default:0x%04X\r\n" ,__FUNCTION__ , Storage_Block[0]);
        
	    DataFlashRead(record_idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
        printf("%s:0x%04X\r\n" ,__FUNCTION__ , u32SPIMode);
        printf("%s:Read mode[0]:0x%04X\r\n",__FUNCTION__ ,Storage_Block[0]);
    }
}

/*
    0 : GPIO
    1 : SPI MOSI
*/
void SwitchToFunctionPin(unsigned char swap)
{
    if (swap == 0)  // GPIO
    {
        PC3 = 1;
        SYS->GPC_MFP &= ~(SYS_GPC_MFP_PC3_Msk);
        SYS->GPC_MFP |= SYS_GPC_MFP_PC3_GPIO;
        SYS->ALT_MFP &= ~(SYS_ALT_MFP_PC3_Msk);
        SYS->ALT_MFP |= SYS_ALT_MFP_PC3_GPIO;
        GPIO_SetMode(PC, BIT3, GPIO_PMD_OUTPUT);
    }
    else if (swap == 1) // MOSI
    {
        SYS->GPC_MFP &= ~(SYS_GPC_MFP_PC3_Msk);
        SYS->GPC_MFP |= SYS_GPC_MFP_PC3_SPI0_MOSI0;
        SYS->ALT_MFP &= ~(SYS_ALT_MFP_PC3_Msk);
        SYS->ALT_MFP |= SYS_ALT_MFP_PC3_SPI0_MOSI0;
    }

}

void SPI_Write_Packet(void)
{
    static uint8_t cnt = 0;
    uint8_t packet[8] = {0};
    uint8_t i = 0;

    packet[0] = 0x5A;
    packet[1] = 0x5A;
    packet[2] = cnt;
    packet[3] = 0;
    packet[4] = 0;
    packet[5] = cnt+1;
    packet[6] = 0x5A;
    packet[7] = 0x5A;

    #if defined (MANUAL_SWITCH_MOSI_TO_GPIO)
    SwitchToFunctionPin(1);
    #endif

    PC0 = 0; //SPI_SET_SS_LOW(SPI0);

    for (i = 0; i < 8 ;i++)
    {
        SPI_WRITE_TX(SPI0, packet[i]);
        SPI_TRIGGER(SPI0);
        while(SPI_IS_BUSY(SPI0)); 
    }

    PC0 = 1; //SPI_SET_SS_HIGH(SPI0);
    
    #if defined (MANUAL_SWITCH_MOSI_TO_GPIO)
    SwitchToFunctionPin(0);
    #endif

    cnt++;
}

void SPI_Init(void)
{
    switch(u32SPIMode)
    {
        case SPI_MODE_0:
            printf("%s:SET SPI mode 0:0x%04X\r\n" , __FUNCTION__, u32SPIMode);
            break;
        case SPI_MODE_1:
            printf("%s:SET SPI mode 1:0x%04X\r\n" , __FUNCTION__, u32SPIMode);
            break;
        case SPI_MODE_2:
            printf("%s:SET SPI mode 2:0x%04X\r\n" , __FUNCTION__, u32SPIMode);
            break;
        case SPI_MODE_3:
            printf("%s:SET SPI mode 3:0x%04X\r\n" , __FUNCTION__, u32SPIMode);
            break;
    }

	SPI_Open(SPI0, SPI_MASTER, u32SPIMode, 8, SPI_FREQ);
	// SPI_EnableAutoSS(SPI0, SPI_SS, SPI_SS_ACTIVE_LOW);  
    SPI_DisableAutoSS(SPI0);
}


//
// check_reset_source
//
uint8_t check_reset_source(void)
{
    uint32_t src = SYS_GetResetSrc();

    SYS->RSTSRC |= 0x0FF;
    printf("Reset Source <0x%08X>\r\n", src);

    #if 1   //DEBUG , list reset source
    if (src & BIT0)
    {
        printf("0)Power-On Reset Flag\r\n");       
    }
    if (src & BIT1)
    {
        printf("1)Reset Pin Reset Flag\r\n");       
    }
    if (src & BIT2)
    {
        printf("2)Watchdog Timer Reset Flag\r\n");       
    }
    if (src & BIT3)
    {
        printf("3)Low Voltage Reset Flag\r\n");       
    }
    if (src & BIT4)
    {
        printf("4)Brown-Out Detector Reset Flag\r\n");       
    }
    if (src & BIT5)
    {
        printf("5)SYS Reset Flag\r\n");       
    }
    if (src & BIT6)
    {
        printf("6)Reserved.\r\n");       
    }
    if (src & BIT7)
    {
        printf("7)CPU Reset Flag \r\n");       
    }
    #endif
    
    if (src & SYS_RSTSRC_RSTS_POR_Msk) {
        SYS_ClearResetSrc(SYS_RSTSRC_RSTS_POR_Msk);
        
        printf("power on from POR\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSRC_RSTS_RESET_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSRC_RSTS_RESET_Msk);
        
        printf("power on from nRESET pin\r\n");
        return FALSE;
    } 
    else if (src & SYS_RSTSRC_RSTS_WDT_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSRC_RSTS_WDT_Msk);
        
        printf("power on from WDT Reset\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSRC_RSTS_LVR_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSRC_RSTS_LVR_Msk);
        
        printf("power on from LVR Reset\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSRC_RSTS_BOD_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSRC_RSTS_BOD_Msk);
        
        printf("power on from BOD Reset\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSRC_RSTS_SYS_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSRC_RSTS_SYS_Msk);
        
        printf("power on from System Reset\r\n");
        return FALSE;
    } 
    else if (src & SYS_RSTSRC_RSTS_CPU_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSRC_RSTS_CPU_Msk);

        printf("power on from CPU reset\r\n");
        return FALSE;         
    } 
    
    printf("power on from unhandle reset source\r\n");
    return FALSE;
}

void TMR1_IRQHandler(void)
{
	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
            FLAG_PROJ_TIMER_PERIOD_1000MS = 1;//set_flag(flag_timer_period_1000ms ,ENABLE);
		}

		if ((get_tick() % 500) == 0)
		{
            FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 1;
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}

void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void loop(void)
{
	// static uint32_t LOG1 = 0;
	// static uint32_t LOG2 = 0;

    if ((get_systick() % 1000) == 0)
    {
        // printf("%s(systick) : %4d\r\n",__FUNCTION__,LOG2++);    
    }

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)//(is_flag_set(flag_timer_period_1000ms))
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0;//set_flag(flag_timer_period_1000ms ,DISABLE);

        // printf("%s(timer) : %4d\r\n",__FUNCTION__,LOG1++);
        PB4 ^= 1;        
    }

    if (FLAG_PROJ_TIMER_PERIOD_SPECIFIC)
    {
        FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 0;
        SPI_Write_Packet();
    }

    if (FLAG_PROJ_SPI_MODE_0)
    {
        FLAG_PROJ_SPI_MODE_0 = 0;
        Write_SPI_Mode_Record(SPI_MODE_0);
    }
    if (FLAG_PROJ_SPI_MODE_1)
    {
        FLAG_PROJ_SPI_MODE_1 = 0;
        Write_SPI_Mode_Record(SPI_MODE_1);
    }
    if (FLAG_PROJ_SPI_MODE_2)
    {
        FLAG_PROJ_SPI_MODE_2 = 0;
        Write_SPI_Mode_Record(SPI_MODE_2);
    }
    if (FLAG_PROJ_SPI_MODE_3)
    {
        FLAG_PROJ_SPI_MODE_3 = 0;
        Write_SPI_Mode_Record(SPI_MODE_3);
    }
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		printf("press : %c\r\n" , res);
		switch(res)
		{
			case '0':
                FLAG_PROJ_SPI_MODE_0 = 1;
				break;
			case '1':
                FLAG_PROJ_SPI_MODE_1 = 1;
				break;
			case '2':
                FLAG_PROJ_SPI_MODE_2 = 1;
				break;
			case '3':
                FLAG_PROJ_SPI_MODE_3 = 1;
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
                SYS_UnlockReg();
				// NVIC_SystemReset();	// Reset I/O and peripherals , only check BS(FMC_ISPCTL[1])
                // SYS_ResetCPU();     // Not reset I/O and peripherals
                SYS_ResetChip();    // Reset I/O and peripherals ,  BS(FMC_ISPCTL[1]) reload from CONFIG setting (CBS)	
				break;
		}
	}
}

void UART02_IRQHandler(void)
{

    if(UART_GET_INT_FLAG(UART0, UART_ISR_RDA_INT_Msk | UART_ISR_TOUT_IF_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
            UARTx_Process();
        }
    }

}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_IER_RDA_IEN_Msk | UART_IER_TOUT_IEN_Msk);
    NVIC_EnableIRQ(UART02_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLKFreq : %8d\r\n",CLK_GetPCLKFreq());	
	#endif	

    #if 0
    printf("FLAG_PROJ_TIMER_PERIOD_1000MS : 0x%2X\r\n",FLAG_PROJ_TIMER_PERIOD_1000MS);
    printf("FLAG_PROJ_REVERSE1 : 0x%2X\r\n",FLAG_PROJ_REVERSE1);
    printf("FLAG_PROJ_REVERSE2 : 0x%2X\r\n",FLAG_PROJ_REVERSE2);
    printf("FLAG_PROJ_REVERSE3 : 0x%2X\r\n",FLAG_PROJ_REVERSE3);
    printf("FLAG_PROJ_REVERSE4 : 0x%2X\r\n",FLAG_PROJ_REVERSE4);
    printf("FLAG_PROJ_REVERSE5 : 0x%2X\r\n",FLAG_PROJ_REVERSE5);
    printf("FLAG_PROJ_REVERSE6 : 0x%2X\r\n",FLAG_PROJ_REVERSE6);
    printf("FLAG_PROJ_REVERSE7 : 0x%2X\r\n",FLAG_PROJ_REVERSE7);
    #endif

}

void GPIO_Init (void)
{
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB4_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB4_GPIO);
	
    GPIO_SetMode(PB, BIT4, GPIO_PMD_OUTPUT);

}


void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
//    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
//    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(1));
	
    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1_S_HIRC, 0);

    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL1_SPI0_S_HCLK, MODULE_NoMsk);
    CLK_EnableModuleClock(SPI0_MODULE);

    /* Setup SPI0 multi-function pins */
    SYS->GPC_MFP &= ~(/*SYS_GPC_MFP_PC0_Msk |*/ SYS_GPC_MFP_PC1_Msk | SYS_GPC_MFP_PC2_Msk | SYS_GPC_MFP_PC3_Msk);
    SYS->GPC_MFP |= /*SYS_GPC_MFP_PC0_SPI0_SS0 |*/ SYS_GPC_MFP_PC1_SPI0_CLK | SYS_GPC_MFP_PC2_SPI0_MISO0 | SYS_GPC_MFP_PC3_SPI0_MOSI0;
    SYS->ALT_MFP &= ~(/*SYS_ALT_MFP_PC0_Msk |*/ SYS_ALT_MFP_PC1_Msk | SYS_ALT_MFP_PC2_Msk | SYS_ALT_MFP_PC3_Msk);
    SYS->ALT_MFP |= /*SYS_ALT_MFP_PC0_SPI0_SS0 |*/ SYS_ALT_MFP_PC1_SPI0_CLK | SYS_ALT_MFP_PC2_SPI0_MISO0 | SYS_ALT_MFP_PC3_SPI0_MOSI0;


    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);

   /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

int main()
{
    SYS_Init();

	GPIO_Init();
	UART0_Init();
	TIMER1_Init();
    check_reset_source();

    SysTick_enable(1000);
    #if defined (ENABLE_TICK_EVENT)
    TickSetTickEvent(1000, TickCallback_processA);  // 1000 ms
    TickSetTickEvent(5000, TickCallback_processB);  // 5000 ms
    #endif

	DataFlashInit();

    Check_SPI_Mode_Record();
    SPI_Init();

    /* Got no where to go, just loop forever */
    while(1)
    {
        loop();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
