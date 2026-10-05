/* CAN protocol two nodes 22 and 21 id extended ID 
Input: Button P.0.21 PIN 
ouput : LED inbuilt 29 PIN 
CAN_21.c */

#include<LPC17xx.h>
void CAN_Init(void);
void CAN_ACC(void);
void looktable(void); 
void delay(unsigned long int z);
void can_tx(unsigned long int id,unsigned char msg);
void can_rx(void);
unsigned char rx=0, prev=0;
int main()
{
	SystemInit ();	
	LPC_PINCON->PINMODE1 |= (3<<10); //Pull downn if need change it 
	LPC_GPIO1->FIOMASK3 =0XDF;  //LED OUTPUT
	LPC_GPIO1->FIODIR3 =0X20;   //LED OUTPUT
	LPC_GPIO1->FIOSET3 =0X20;   
	LPC_GPIO0->FIOMASK = 0XFFDFFFFF; // INPUT BUTTON
	LPC_GPIO0->FIODIR = 0XFFDFFFFF; //INPUT BUTTON
		
	CAN_Init();
	CAN_ACC();
	looktable(); 	
	
	while(1){
		rx = (LPC_GPIO0 -> FIOPIN >> 21) & 0x01;
		if (prev!=rx){
			prev = rx;
			if (rx == 1)
				can_tx (0x00000022,'1');
			else {
				can_tx (0x00000022,'0');
			}
		}
	}
}

void CAN_Init(void)
{
	LPC_SC->PCONP|=0x00002000;
	LPC_SC->PCLKSEL0|=0X00000000;
	LPC_PINCON->PINSEL0|=0X00000005;
	LPC_CAN1->MOD=0x00000001;
	LPC_CAN1->CMR=0X00000000;
	LPC_CAN1->GSR=0x00000000;
	LPC_CAN1->IER=0x00000001;
	LPC_CAN1->BTR=0X001C0007;
	LPC_CAN1->MOD=0x00000000;	
	NVIC_EnableIRQ(CAN_IRQn);
}

void CAN_ACC(void) // need changes here if we add id s
{
	LPC_CANAF->AFMR=0x00000001;
	LPC_CANAF->SFF_sa=0x00000000;
	LPC_CANAF->EFF_sa=0x00000000;
	LPC_CANAF->SFF_GRP_sa=0x00000000;
	LPC_CANAF->EFF_GRP_sa=0x00000000C;
	LPC_CANAF->ENDofTable=0x00000000C;
	LPC_CANAF->AFMR=0x00000000;	
}

void looktable(void) 
{
	LPC_CANAF->AFMR=0x00000001;
	LPC_CANAF_RAM->mask[0]=0x00000020;
	LPC_CANAF_RAM->mask[1]=0x00000021;
	LPC_CANAF_RAM->mask[2]=0x00000022; // Add up if need ID's 
	LPC_CANAF->AFMR=0x00000000;
}

void delay(unsigned long int z)
{
	unsigned long int x;
	for(x=0;x<z;x++);
}

  
void can_tx(unsigned long int id,unsigned char msg) //Based on Node 
{
	while((LPC_CAN1->SR&0X00000004)!=0X00000004);
	LPC_CAN1->TFI1=0X80010000;
	LPC_CAN1->TID1=id;
	LPC_CAN1->TDA1=msg;
	LPC_CAN1->CMR=0X21;
}

void 	CAN_IRQHandler(void)
{
  can_rx();
}	

void can_rx(void) // Changes need 
{
  unsigned long int CA = 0,CB = 0, CC = 0, CD = 0;
	CA = LPC_CAN1->RFS; //std frame
	CA = CA & 0x20000000;
	if (CA == 0x00000000)
	{
		CB = LPC_CAN1->RID; //id
		CB = CB & 0x000007FF;
		if(CB == 0x00000021)
		{
			CC = LPC_CAN1->RFS; //dlc
			CC = CC & 0x000F0000;
			if(CC == 0x00010000)
			{
				CD = LPC_CAN1->RDA; //data
				CD = CD & 0x000000FF;
				if(CD == '1')
				{  
					LPC_GPIO1->FIOSET3=0x20;
			  }
				else{
					LPC_GPIO1->FIOCLR3=0x20;
				}
			}
		}
		LPC_CAN1->CMR = 0X04;		
	}
}