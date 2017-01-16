#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>       // Required for the GPIO functions
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
#include <linux/kthread.h>    // Using kthreads for the flashing functionality
#include <linux/delay.h>      // Using this header for the msleep() function
#include <rtdm/rtdm_driver.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andy Yin");
MODULE_DESCRIPTION("FSK for TLCR5800");
MODULE_VERSION("0.1");


#define BUFFER_SIZE 2000
#define SYMBOL_PERIOD 6000
static uint8_t * tx_data = NULL;

// beaglebone pinout
static unsigned int buffer_ctrl = 51;
static unsigned int lpLED = 60;
static unsigned int lpLED_CAT = 50;


uint32_t  slot_ns;
static rtdm_timer_t phy_timer;

static uint8_t tx_buffer[BUFFER_SIZE] ;

static bool ledOn = 0;                      ///< Is the LED on or off? Used for flashing



/*******************  In this function, we implement FSK in the code to ********************/
static int curr_bit_pos =0;
static int symbol_count =0;
static int preamble =0;
static int bit=3;
static int tail=0;
void phy_timer_handler(rtdm_timer_t *timer)
{
    if(preamble < 30 )
    {
        preamble++;
        ledOn=true;
    }
    else
    {
        // sendDone
        if(curr_bit_pos==8*BUFFER_SIZE)
        {
            bit = 2;
            ledOn = false;
            symbol_count = 0;
            tail++;
        }
        // get bit from the buffer
        if(curr_bit_pos < 8*BUFFER_SIZE)
        {
            bit=tx_data[curr_bit_pos];
        }
        // when to blink
        if((bit==1 && symbol_count ==1)|| (bit==0 && symbol_count ==2))
        {
            ledOn = !ledOn;
        }
        symbol_count++;
        gpio_set_value(lpLED,ledOn);
        if((bit==1 && symbol_count >1) || (bit==0 && symbol_count >3))
        {
            symbol_count=0;
            ledOn=true;
            curr_bit_pos++;
        }
        // set led to be high once it is received.
        if(tail == 200)
        {
            curr_bit_pos++;
            ledOn=true;
            tail++;
        }	
    }
}


/***************** convert byte to bit ***********/
static uint8_t* byte_to_bit(uint8_t * data, int length)
{

    uint8_t * in = NULL;
    int i =0;
    int j=0;
    uint8_t mask = 0x80;

    in = (uint8_t*) kmalloc(length*8*sizeof(uint8_t), GFP_KERNEL);
    if (in == NULL)
    {
        printk(KERN_INFO "it's null don't go further");
    }

    for(i=0;i<length; i++)
    {
        for(j=0; j<8; j++)
        {
            in[i*8 + j] = (uint8_t)((data[i]&mask)>>(7-j));
            mask = mask >> 0x1;
        }
        mask = 0x80;
    }
    return in;

}


/***************************** init *****************************/
static int __init ebbLED_init(void){
    int ret = -ENOMEM;
    int j=0;
    printk(KERN_INFO "EBB LED: Initializing the EBB LED LKM\n");
    for(j=0;j<BUFFER_SIZE;j++)
    {
        tx_buffer[j] = 1;
    }
    tx_data = byte_to_bit(tx_buffer, BUFFER_SIZE);
    gpio_request(buffer_ctrl, "sysfs");
    gpio_request(lpLED, "sysfs");
    gpio_request(lpLED_CAT, "sysfs");
    gpio_direction_output(buffer_ctrl, false);
    gpio_direction_output(lpLED, false);   
    gpio_direction_output(lpLED_CAT, false);  
    gpio_export(buffer_ctrl, false);  
    gpio_export(lpLED, false);  
    gpio_export(lpLED_CAT, false);  
                                
    //timer
    slot_ns = 1000000000/SYMBOL_PERIOD;
    ret = rtdm_timer_init(&phy_timer, phy_timer_handler,"phy timer");
    if(ret)
    {
        rtdm_printk("PWM: error initializing up-timer: %i\n", ret);
        return  ret;
    }	 
    ret = rtdm_timer_start(&phy_timer, slot_ns, slot_ns, RTDM_TIMERMODE_RELATIVE);
    if(ret)
    {
        rtdm_printk("PWM: error starting up-timer: %i\n", ret);
        return ret;	
    }
    rtdm_printk("PWM: timers created\n");

    //thread
    //   task = kthread_run(flash, NULL, "LED_flash_thread");  // Start the LED flashing thread
    //   if(IS_ERR(task)){                                     // Kthread name is LED_flash_thread
    //      printk(KERN_ALERT "EBB LED: failed to create the task\n");
    //      return PTR_ERR(task);
    //   }
    return 0;
    }

static void __exit ebbLED_exit(void){
    kfree(tx_data);
    rtdm_timer_destroy(&phy_timer);
    gpio_unexport(buffer_ctrl);
    gpio_unexport(lpLED);
    gpio_unexport(lpLED_CAT);

    gpio_free(buffer_ctrl);                      // Free the LED GPIO
    gpio_free(lpLED);                      // Free the LED GPIO
    gpio_free(lpLED_CAT);                      // Free the LED GPIO

    printk(KERN_INFO "EBB LED: Goodbye from the EBB LED LKM!\n");
}

module_init(ebbLED_init);
module_exit(ebbLED_exit);
