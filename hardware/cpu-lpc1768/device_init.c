
#include "LPC17xx.h"
#include "cmsis_nvic.h"

uint32_t SystemCoreClock = 99000000;

static void ClockInit(void)
{
	/* Disable peripherals power. */
	LPC_SC->PCONP = 0;

	/* Enable GPIO power. */
	LPC_SC->PCONP = PCONP_PCGPIO;

	/* Disable TPIU. */
	LPC_PINCON->PINSEL10 = 0;

	if ( LPC_SC->PLL0STAT & ( 1 << 25 ) )
	{
		/* Enable PLL, disconnected. */
		LPC_SC->PLL0CON = 1;			
		LPC_SC->PLL0FEED = PLLFEED_FEED1;
		LPC_SC->PLL0FEED = PLLFEED_FEED2;
	}
	
	/* Disable PLL, disconnected. */
	LPC_SC->PLL0CON = 0;				
	LPC_SC->PLL0FEED = PLLFEED_FEED1;
	LPC_SC->PLL0FEED = PLLFEED_FEED2;
	    
	/* Enable main OSC. */
	LPC_SC->SCS |= 0x20;			
	while( !( LPC_SC->SCS & 0x40 ) );
	
	/* select main OSC, 12MHz, as the PLL clock source. */
	LPC_SC->CLKSRCSEL = 0x1;		
	
	LPC_SC->PLL0CFG = 0x20031;
	LPC_SC->PLL0FEED = PLLFEED_FEED1;
	LPC_SC->PLL0FEED = PLLFEED_FEED2;
	      
	/* Enable PLL, disconnected. */
	LPC_SC->PLL0CON = 1;				
	LPC_SC->PLL0FEED = PLLFEED_FEED1;
	LPC_SC->PLL0FEED = PLLFEED_FEED2;
	
	/* Set clock divider. */
	LPC_SC->CCLKCFG = 0x03;
	
	/* Configure flash accelerator. */
	LPC_SC->FLASHCFG = 0x403a;
	
	/* Check lock bit status. */
	while( ( ( LPC_SC->PLL0STAT & ( 1 << 26 ) ) == 0 ) );	
	    
	/* Enable and connect. */
	LPC_SC->PLL0CON = 3;				
	LPC_SC->PLL0FEED = PLLFEED_FEED1;
	LPC_SC->PLL0FEED = PLLFEED_FEED2;
	while( ( ( LPC_SC->PLL0STAT & ( 1 << 25 ) ) == 0 ) );	
	
	/* Configure the clock for the USB. */
	  
	if( LPC_SC->PLL1STAT & ( 1 << 9 ) )
	{
		/* Enable PLL, disconnected. */
		LPC_SC->PLL1CON = 1;			
		LPC_SC->PLL1FEED = PLLFEED_FEED1;
		LPC_SC->PLL1FEED = PLLFEED_FEED2;
	}
	
	/* Disable PLL, disconnected. */
	LPC_SC->PLL1CON = 0;				
	LPC_SC->PLL1FEED = PLLFEED_FEED1;
	LPC_SC->PLL1FEED = PLLFEED_FEED2;
	
	LPC_SC->PLL1CFG = 0x23;
	LPC_SC->PLL1FEED = PLLFEED_FEED1;
	LPC_SC->PLL1FEED = PLLFEED_FEED2;
	      
	/* Enable PLL, disconnected. */
	LPC_SC->PLL1CON = 1;				
	LPC_SC->PLL1FEED = PLLFEED_FEED1;
	LPC_SC->PLL1FEED = PLLFEED_FEED2;
	while( ( ( LPC_SC->PLL1STAT & ( 1 << 10 ) ) == 0 ) );
	
	/* Enable and connect. */
	LPC_SC->PLL1CON = 3;				
	LPC_SC->PLL1FEED = PLLFEED_FEED1;
	LPC_SC->PLL1FEED = PLLFEED_FEED2;
	while( ( ( LPC_SC->PLL1STAT & ( 1 << 9 ) ) == 0 ) );

	/*  Setup the peripheral bus to be the same as the PLL output (64 MHz). */
	LPC_SC->PCLKSEL0 = 0x05555555;
}

#define SHCSR_USGFAULTENA	(1 << 18)
#define SHCSR_BUSFAULTENA	(1 << 17)
#define SHCSR_MEMFAULTENA	(1 << 16)

void LowLevelInit(void)
{
	extern void vPortSVCHandler( void );
	extern void xPortPendSVHandler(void);
	extern void xPortSysTickHandler(void);
	extern void MpuManagerInit(void);

	ClockInit();

	SCB->SHCSR |= SHCSR_USGFAULTENA | SHCSR_BUSFAULTENA; // SHCSR_MEMFAULTENA gets set by kernel when MPU is init

	// Setup FreeRTOS vectors
	NVIC_SetVector(SVCall_IRQn, (unsigned int)vPortSVCHandler);
	NVIC_SetVector(PendSV_IRQn, (unsigned int)xPortPendSVHandler);
	NVIC_SetVector(SysTick_IRQn, (unsigned int)xPortSysTickHandler);
	// Priorities for these are set by kernel
}



