#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include <string.h>
#include "pico/binary_info.h"
// I2C defines
////////////////////////////////////////////////
#define LCD_LIMPA_TELA     0x01
#define LCD_INICIA         0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET    0x20

#define LCD_INICIO_ESQUERDA 0x02

#define LCD_LIGA_DISPLAY 0x04

#define LCD_16x2 0x08

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE_BIT 0x04

// By default these LCD display drivers are on bus address 0x27
#define BUS_ADDR 0x27

// Modes for lcd_envia_byte
#define LCD_CARACTER  1
#define LCD_COMANDO   0

#define MAX_LINES      2
#define MAX_CHARS      16

#define DELAY_US 600

void init_i2c(){
    i2c_init(i2c_default,100*1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN,GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN,GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
}

void lcd_envia_comando(uint8_t val) {
   i2c_write_blocking(i2c_default, BUS_ADDR, &val,1, false);
   
}

void lcd_pulsa_enable(uint8_t val) {
    sleep_us(DELAY_US);
    lcd_envia_comando(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    lcd_envia_comando(val | ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
   
}

void lcd_envia_byte(uint8_t caractere, int modo) {
   uint8_t nible_alto = modo | (caractere & 0xF0) | LCD_BACKLIGHT;
   uint8_t nible_baixo = modo | ((caractere << 4) & 0xF0) | LCD_BACKLIGHT;
   lcd_envia_comando(nible_alto);
   lcd_pulsa_enable(nible_alto);
   lcd_envia_comando(nible_baixo);
   lcd_pulsa_enable(nible_baixo);
}

void lcd_limpa_tela(void) {
    lcd_envia_byte(LCD_LIMPA_TELA, LCD_COMANDO);  
}

void lcd_posiciona_cursor(int linha, int coluna) {
   uint8_t aux = (linha == 0)? 0x80 + coluna : 0xC0 + coluna;
   lcd_envia_byte(aux, LCD_COMANDO);
}

void lcd_envia_caracter(char caractere) {
    lcd_envia_byte(caractere,LCD_CARACTER);
    
}

void lcd_envia_string(const char *s) {
    while(*s){
        lcd_envia_caracter(*s++);
    }
    
}

void lcd_init() {
    lcd_envia_byte(LCD_INICIA,LCD_COMANDO);
    lcd_envia_byte(LCD_INICIA | LCD_LIMPA_TELA, LCD_COMANDO);
    lcd_envia_byte(LCD_ENTRYMODESET | LCD_INICIO_ESQUERDA, LCD_COMANDO);
    lcd_envia_byte(LCD_FUNCTIONSET | LCD_16x2, LCD_COMANDO);
    lcd_envia_byte(LCD_DISPLAYCONTROL | LCD_LIGA_DISPLAY, LCD_COMANDO);
    lcd_limpa_tela();
}
/////////////////////////////////////////////////

volatile bool bouncing = false;
float count = 0;
int k = 0;

long long int debouncer(long int a, void *p){
    bool state = gpio_get(2); 
    //printf("%d",state);
    if (state == 0){
        k ++;
    }
    return 0;
}


void gpio_callback(uint gpio, uint32_t events) {
    add_alarm_in_ms(10,debouncer, NULL, false);
    if (k > 8){
        count = count +1;
        float resultado = count*0.2 ;
        printf(" %.1f  \n",resultado);
        k = 0;
    }  
}



void imprime (){
    
    float resultado = count * 0.2 ;
    //printf(" %.1f  \n",resultado);
    
    lcd_posiciona_cursor(1,0);
    char convertido[16];
    sprintf(convertido,"%.1f",resultado);
    int i;
    for (i=0; convertido[i] != '\0'; i++){
        lcd_envia_caracter(convertido[i]);
        }
    lcd_posiciona_cursor(1,6);
    lcd_envia_string("mm");

}


int main()
{
    stdio_init_all();
    //cyw43_arch_init();
    //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,false);
    init_i2c();
    sleep_ms(2000);
    printf("\nContagem de chuva\n");
    int i = 2;
    gpio_init(i);
    gpio_set_dir(i,GPIO_IN);
    gpio_set_pulls(i, true, false);
    gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true, &gpio_callback);

    lcd_init();
    lcd_posiciona_cursor(0,0);
    lcd_envia_string("BASCULADAS:");
    lcd_posiciona_cursor(1,0);
    lcd_envia_string("--------------");
    sleep_ms(1000);
    lcd_limpa_tela(); 
    lcd_posiciona_cursor(0,0);
    lcd_envia_string("BASCULADAS:");
     

    while (true) {
        imprime();       
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,false);
        //sleep_ms(200);
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,true);
        //sleep_ms(200);
    }
}
