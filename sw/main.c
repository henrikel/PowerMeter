#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/* Pin arrangements as follows:
   PB7, PB2, PB3, PB4: Cathodes for LED indicators
   PB0, PD6, PD5, PD4, PD3, PD2, PD1, PD0: Indicator segments A-G and decimal point
   PB1: Photo transistor input (analog comparator)
*/

#define LED1_PORT	PORTB
#define LED2_PORT	PORTB
#define LED3_PORT	PORTB
#define LED4_PORT	PORTB
#define LED1_DDR	DDRB
#define LED2_DDR	DDRB
#define LED3_DDR	DDRB
#define LED4_DDR	DDRB
#define LED1_PIN	PB7
#define LED2_PIN	PB2
#define LED3_PIN	PB3
#define LED4_PIN	PB4

#define SEGA_PORT	PORTB
#define SEGB_PORT	PORTD
#define SEGC_PORT	PORTD
#define SEGD_PORT	PORTD
#define SEGE_PORT	PORTD
#define SEGF_PORT	PORTD
#define SEGG_PORT	PORTD
#define SEGDP_PORT	PORTD
#define SEGA_DDR	DDRB
#define SEGB_DDR	DDRD
#define SEGC_DDR	DDRD
#define SEGD_DDR	DDRD
#define SEGE_DDR	DDRD
#define SEGF_DDR	DDRD
#define SEGG_DDR	DDRD
#define SEGDP_DDR	DDRD
#define SEGA_PIN	PB0
#define SEGB_PIN	PD6
#define SEGC_PIN	PD5
#define SEGD_PIN	PD4
#define SEGE_PIN	PD3
#define SEGF_PIN	PD2
#define SEGG_PIN	PD1
#define SEGDP_PIN	PD0

#define MAX_COUNT		100

volatile uint8_t	*led_ports[] = {&LED1_PORT, &LED2_PORT, &LED3_PORT, &LED4_PORT};
volatile uint8_t	*seg_ports[] = {&SEGA_PORT, &SEGB_PORT, &SEGC_PORT, &SEGD_PORT, 
                                    &SEGE_PORT, &SEGF_PORT, &SEGG_PORT, &SEGDP_PORT};
                                    
volatile uint8_t	*led_ddr[] = {&LED1_DDR, &LED2_DDR, &LED3_DDR, &LED4_DDR};
volatile uint8_t	*seg_ddr[] = {&SEGA_DDR, &SEGB_DDR, &SEGC_DDR, &SEGD_DDR, 
                                  &SEGE_DDR, &SEGF_DDR, &SEGG_DDR, &SEGDP_DDR};
                                    
static uint8_t		led_pins[] = {7, 2, 3, 4};
//static uint8_t		seg_pins[] = {0, 6, 5, 4, 3, 2, 1, 0};

static uint8_t		codes[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 
                               0b01100110, 0b01101101, 0b01111101, 0b00000111, 
                               0b01111111, 0b01100111, 0b01110111, 0b01111100,
			                   0b00111001, 0b01011110, 0b01111001, 0b01110001,
			                   0b10000000};
			                   
volatile uint8_t	values[4] = {0,1,2,3}, dp_pos = 0, display_intensity = MAX_COUNT, 
					intensities[4] = {0,0,0,0}, update = 1;
volatile uint32_t	millisecs = 0;

#define setbit(port, bit)	(port) |= (1 << (bit))
#define clearbit(port, bit)	(port) &= ~(1 << (bit))

void set_segments(uint8_t values) {

	if(values & 1)
		setbit(SEGA_PORT,SEGA_PIN);
	else
		clearbit(SEGA_PORT,SEGA_PIN);
		
	if(values & 2)
		setbit(SEGB_PORT,SEGB_PIN);
	else
		clearbit(SEGB_PORT,SEGB_PIN);
		
	if(values & 4)
		setbit(SEGC_PORT,SEGC_PIN);
	else
		clearbit(SEGC_PORT,SEGC_PIN);
		
	if(values & 8)
		setbit(SEGD_PORT,SEGD_PIN);
	else
		clearbit(SEGD_PORT,SEGD_PIN);
		
	if(values & 0x10)
		setbit(SEGE_PORT,SEGE_PIN);
	else
		clearbit(SEGE_PORT,SEGE_PIN);
		
	if(values & 0x20)
		setbit(SEGF_PORT,SEGF_PIN);
	else
		clearbit(SEGF_PORT,SEGF_PIN);
		
	if(values & 0x40)
		setbit(SEGG_PORT,SEGG_PIN);
	else
		clearbit(SEGG_PORT,SEGG_PIN);
		
	if(values & 0x80)
		setbit(SEGDP_PORT,SEGDP_PIN);
	else
		clearbit(SEGDP_PORT,SEGDP_PIN);

}

void set_value(uint8_t val, int dp_flag) {
	
	if(val <= 16)
		set_segments(codes[val] | (dp_flag ? 0x80 : 0));
}

void show_value(int val, int decimal_pos, int show_zeros) {
	
	if(val <= 9999) {
		values[3] = val % 10;
		values[2] = val / 10 % 10;
		values[1] = val / 100 % 10;
		values[0] = val / 1000 % 10;
	}
	else
		values[3] = values[2] = values[1] = values[0] = 14;
	
	intensities[0] = intensities[1] = intensities[2] = intensities[3] = display_intensity;

	if(!show_zeros) {
		if(val < 1000 && decimal_pos != 1)
			intensities[0] = 0;
		if(val < 100 && decimal_pos != 2)
			intensities[1] = 0;
		if(val < 10 && decimal_pos != 3)
			intensities[2] = 0;
	}
	
	if(decimal_pos > 0 && decimal_pos <= 4)
		dp_pos = decimal_pos;
}

int main(void) {
	
	// Initialize 
	
	cli();
	
	setbit(LED1_DDR, LED1_PIN);
	setbit(LED2_DDR, LED2_PIN);
	setbit(LED3_DDR, LED3_PIN);
	setbit(LED4_DDR, LED4_PIN);
	
	clearbit(LED1_PORT, LED1_PIN);
	clearbit(LED2_PORT, LED2_PIN);
	clearbit(LED3_PORT, LED3_PIN);
	clearbit(LED4_PORT, LED4_PIN);
		
	setbit(SEGA_DDR, SEGA_PIN);
	setbit(SEGB_DDR, SEGB_PIN);
	setbit(SEGC_DDR, SEGC_PIN);
	setbit(SEGD_DDR, SEGD_PIN);
	setbit(SEGE_DDR, SEGE_PIN);
	setbit(SEGF_DDR, SEGF_PIN);
	setbit(SEGG_DDR, SEGG_PIN);
	setbit(SEGDP_DDR, SEGDP_PIN);
	
	clearbit(SEGA_PORT, SEGA_PIN);
	clearbit(SEGB_PORT, SEGB_PIN);
	clearbit(SEGC_PORT, SEGC_PIN);
	clearbit(SEGD_PORT, SEGD_PIN);
	clearbit(SEGE_PORT, SEGE_PIN);
	clearbit(SEGF_PORT, SEGF_PIN);
	clearbit(SEGG_PORT, SEGG_PIN);
	clearbit(SEGDP_PORT, SEGDP_PIN);
	
	// Setup timer for display refresh
	OCR0A = 40;
    
    TCCR0A = 0; // CTC mode
    TCCR0A |= _BV(WGM01);
    
    TCCR0B = 0;
    // Start timer with a 1/8 prescaler. With a value in OCR0A
    // of 40, the frequency becomes 25 kHz.
    TCCR0B |= _BV(CS01);
    
    TCNT0 = 0;
    //counter = 0;
    
    // Initialize analog comparator
	ACSR &= ~_BV(ACD);
	ACSR |= _BV(ACBG);
	ACSR &= ~_BV(ACIC);
	ACSR &= ~_BV(ACIE);
	ACSR &= ~_BV(ACIS0);
	ACSR |= _BV(ACIS1);
	ACSR |= _BV(ACIE);
	DIDR = 0x03;
	
	// Initialize milliseconds timer
	OCR1A = 125;
    TCCR1A = 0; // CTC mode
    TCCR1B = _BV(WGM12);
    // Start timer with a 1/64 prescaler. The value in OCR1A is the
    // number of microseconds before interrupt is fired, provided
    // system clock is 8 MHz.
    TCCR1B |= _BV(CS11) | _BV(CS10);
    TCNT1 = 0;
    millisecs = 0;
    
    _delay_ms(10);
    
    sei();
    
    // Enable timer interrupt
    TIMSK |= _BV(OCIE0A) | _BV(OCIE1A);
    
    intensities[0] = intensities[1] = intensities[2] = intensities[3] = display_intensity;
    show_value(0, 1, 1);
	
	// Conversion between pulse period and power in W is 3600/period in seconds
	// 
	// For a frequency of 1000 impulses per kWh, the maximum power of 10 kW
	// corresponds to 2.8 impulses per second, or 360 ms
	// The lowest power of 1 W corresponds to 1 pulse per hour
	
	while(1) {
		
		if(update) {
			
			uint32_t	power;
			
			if(millisecs == 0)
				power = 0;
			else
				power = (uint32_t)(3600000/millisecs);
				
			if(power >= 999500)
				show_value((power+500)/1000, 0, 1);
			else if(power >= 99950)
				show_value((power+50)/100, 3, 1);
			else if(power >= 9995)
				show_value((power+5)/10, 2, 1);
			else if(power > 999)
				show_value(power, 1, 1);
			else if(power > 99)
				show_value(power, 1, 1);
			else if(power > 9)
				show_value(power, 2, 1);
			else
				show_value(power, 3, 1);
			update = 0;
			millisecs = 0;
			
		}
		
	}
}

ISR(TIMER0_COMPA_vect) {
	
	static uint8_t	cur_led;
	uint8_t			n = intensities[cur_led] > MAX_COUNT ? MAX_COUNT : intensities[cur_led];
	static uint8_t	counter = 0;
			
	counter += 1;
	
	if(counter == n)
		clearbit(*(led_ports[cur_led]), led_pins[cur_led]);
	else if(counter > MAX_COUNT) {
		cur_led = (cur_led+1) % 4;
		counter = 0;
		set_value(values[cur_led], dp_pos - 1 == cur_led);
		if(intensities[cur_led] != 0)
			setbit(*(led_ports[cur_led]), led_pins[cur_led]);
		//else
		//	clearbit(*(led_ports[cur_led]), led_pins[cur_led]);
	}

}

ISR(TIMER1_COMPA_vect) {
	cli();
	millisecs += 1;
	sei();
}

ISR(ANA_COMP_vect) {

	update = 1;

}