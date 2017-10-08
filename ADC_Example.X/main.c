/*H**********************************************************************
* FILENAME :        main.c            
*
* DESCRIPTION :
*       ADC example for RexPIC-8 
*
* NOTES :
*       Designed for PIC16F1517 (RexPIC-8)
*       Copyright Swiss TI Lab 2017
*       https://www.swisstilab.com/en/
* 
* LICENSE:
*      CC-BY-NC-SA
* 
* AUTHOR :    Kevin Ceresa        START DATE :    08 Oct 2017
*
* CHANGES :
*
* REF NO    VERSION   DATE      WHO     DETAIL
* 1         V1I1      08Oct17   KC      Final Release
*
*H*/

#define _XTAL_FREQ 8000000 //8MHz crystal oscillator (necessary for the delay function)
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------- UART_Write ------
|  Function UART_Write
|
|  Purpose:  Send a character over the UART internal module 
|
|  Parameters:
|      txData:  -- IN --    Char input
|      
|  Returns:  -
*-------------------------------------------------------------------*/
void UART_Write(char txData)
{
    while(0 == PIR1bits.TXIF)
    {
    }

    TXREG = txData;    // Write the data byte to the USART.
}

/*------------------------------------------------- UART_Init ------
|  Function UART_Init
|
|  Purpose:  Initialize the internal UART module 
|
|  Parameters:
|      -
|      
|  Returns:  -
*-------------------------------------------------------------------*/
void UART_Init()
{
    //RC6 output
    TRISCbits.TRISC6 = 0;
    ANSELC = 0;
        
    // ABDOVF no_overflow; SCKP Non-Inverted; BRG16 16bit_generator; WUE disabled; ABDEN disabled; 
    BAUDCON = 0x08;

    // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled; 
    RCSTA = 0x90;

    // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN enabled; SYNC asynchronous; BRGH hi_speed; CSRC slave; 
    TXSTA = 0x24;

    // Baud Rate = 9600; SPBRGL 207; 
    SPBRGL = 0xCF;

    // Baud Rate = 9600; SPBRGH 0; 
    SPBRGH = 0x00;
}

/*------------------------------------------------- ADC_Init --------
|  Function ADC_Init
|
|  Purpose:  Initialize the ADC internal module
|
|  Parameters:
|      -
|      
|  Returns:  -
*-------------------------------------------------------------------*/
void ADC_Init()
{
    /*Port Configuration*/ //Change for your case
    TRISAbits.TRISA1 = 1; //Port RA1 Input
    ANSELAbits.ANSA1 = 1; //Port AN1 (RA1) analog
    
    /*ADC Configuration*/
    ADCON0bits.CHS = 1; //Set AN1 (RA1) as ADC input
    ADCON1bits.ADPREF = 0; //Set VDD as voltage reference
    ADCON1bits.ADCS = 0b001; //Set the input clock at Fosc/8
    ADCON1bits.ADFM = 1; //Format result 8 bits low 2 bits high
    ADCON0bits.ADON = 1; //Enable the ADC module
    __delay_us(10); //Wait the acquisition time
    
}

//Global variable
short unsigned int adc_value;

float temp_buf[10],temp;
int count;

char buffer[50];

/*------------------------------------------------- main ------
|  Function Main
|
|  Purpose:  Initialize all module and ports. 
|            Infinite loop:
|               Start conversion, data elaboration, sending the data.
|
|  Parameters:
|      -
|      
|  Returns:  -
*-------------------------------------------------------------------*/
void main()
{
    //Initialize the uart module
    UART_Init();
    
    //Initialize the ADC module
    ADC_Init();
    
    while(1) //Infinite cycle
    {
       
         /*START THE CONVERSION*/
        ADCON0bits.GO_nDONE = 1; //Start conversion
        while(ADCON0bits.GO_nDONE == 1); //Wait the hardware clear (conversion done)
        
        /*READING THE RESULT*/
        adc_value = (ADRESH<<8)|ADRESL; //Acquiring the value and shift the 2 MSB by 8 bit and add the ADRESL value
        
        //Calculate the temperature value and store it in the buffer
        temp_buf[count] = (double)(((adc_value/1024.0)*3.3-0.5)*100);
       
        /* THESE ELABORATIONS ARE NOT OPTIMIZED */ 
        /* THEY REQUIRE A LARGE AMOUNT OF DATA  */
        
        count++;
        
        //After the 10 values are stored, calculate the averange value and send it 
        if(count == 10)
        {
            count = 0;
            
            for(int c = 0; c <= 9; c++) temp = temp_buf[c]+temp; //Sum all value readed
            
            temp = temp/9; //Get the averange value

            sprintf(buffer,"%0.2f",temp); //Convert the float value in string
        
            //Sending all the char over the UART
            for(int i=0;buffer[i]!='\0';i++)
            UART_Write(buffer[i]);
           
            //Write the *C unit
            UART_Write(' ');
            UART_Write('*');
            UART_Write('C');
            UART_Write('\n');
        
            __delay_ms(400);
    
        }
    }
    
}
