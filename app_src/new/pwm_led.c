#include "main_task.h"
#include "debug.h"
#include "timer.h"
#include "pwm.h"
#include "dma.h"
#include "add_status.h"
#include "breakpoint.h"
#include <string.h>
#include "led_effect.h"
#include "remind_sound.h"
#include "bt_manager.h"
#include "ctrlvars.h"

#ifdef BT_TWS_SUPPORT
#include "bt_tws_api.h"
#endif


#ifdef CFG_PWM_LED_EN

/**********LED��������********************************/

#if (SYS_CORE_DPLL_FREQ == 240*1000)

#define LED_T		150		//200		//LED���鵥��bitλ���ڣ��Ϊ1.25us��T=LED_T/120000000,ϵͳʱ��Ϊ120M
#define RE			0		//RES�ź�ռ�ձ�Ϊ0
#define HT			100		//128  		//�߼�bitΪ1��ռ�ձ�
#define LT			50		//64	  	//�߼�bitΪ0��ռ�ձ�

#elif (SYS_CORE_DPLL_FREQ == 288*1000)

#define LED_T		180		//200		//LED���鵥��bitλ���ڣ��Ϊ1.25us��T=LED_T/120000000,ϵͳʱ��Ϊ120M
#define RE			0		//RES�ź�ռ�ձ�Ϊ0
#define HT			120		//128  		//�߼�bitΪ1��ռ�ձ�
#define LT			60		//64	  	//�߼�bitΪ0��ռ�ձ�

#elif (SYS_CORE_DPLL_FREQ == 360*1000)

#define LED_T		225		//200		//LED���鵥��bitλ���ڣ��Ϊ1.25us��T=LED_T/120000000,ϵͳʱ��Ϊ120M
#define RE			0		//RES�ź�ռ�ձ�Ϊ0
#define HT			150		//128  		//�߼�bitΪ1��ռ�ձ�
#define LT			75		//64	  	//�߼�bitΪ0��ռ�ձ�

#endif


#ifdef PWM7_LED
#define BUF_LEN		LED_NUM*3*8+50	//ˢ�������ݵĳ��ȣ��ӵ�50Ϊǰ48���Լ����2������Ϊ0������ģ��RES�ź�
static uint8_t LED_DATA_1[LED_NUM * 3]	= {0x00};
static uint8_t LedBufA[BUF_LEN] 		= {0x0};
static uint8_t LedBufB[BUF_LEN] 		= {0x0};
#else
static uint8_t LED_DATA_1[LED_NUM * 3]		= {0x00};
#endif

#ifdef PWM6_LED
#define BUF_LEN1	LED_NUM1*3*8+50	//ˢ�������ݵĳ��ȣ��ӵ�50Ϊǰ48���Լ����2������Ϊ0������ģ��RES�ź�
static uint8_t LED_DATA_2[LED_NUM1 * 3]	= {0x00};
static uint8_t LedBufC[BUF_LEN1] 		= {0x0};
static uint8_t LedBufD[BUF_LEN1] 		= {0x0};
#else
static uint8_t LED_DATA_2[LED_NUM1 * 3]		= {0x00};
#endif

#ifdef PWM5_LED
#define BUF_LEN2	LED_NUM2*3*8+50	//ˢ�������ݵĳ��ȣ��ӵ�50Ϊǰ48���Լ����2������Ϊ0������ģ��RES�ź�
static uint8_t LED_DATA_3[LED_NUM2 * 3]	= {0x00};
static uint8_t LedBufE[BUF_LEN2] 		= {0x00};
static uint8_t LedBufF[BUF_LEN2] 		= {0x00};
#else
static uint8_t LED_DATA_3[LED_NUM2 * 3]		= {0x00};
#endif

#ifdef PWM8_LED
#define BUF_LEN3	LED_NUM3*3*8+320	//ˢ�������ݵĳ��ȣ��ӵ�50Ϊǰ48���Լ����2������Ϊ0������ģ��RES�ź�
static uint8_t LED_DATA_4[LED_NUM3 * 3]	= {0x00};
static uint8_t LedBufG[BUF_LEN3] 		= {0x00};
static uint8_t LedBufH[BUF_LEN3] 		= {0x00};
#else
static uint8_t LED_DATA_4[LED_NUM3 * 3]		= {0x00};
#endif

static void pwm7_led_ctrl(uint8_t *data_led);	
static void pwm6_led_ctrl(uint8_t *data_led);	
static void pwm5_led_ctrl(uint8_t *data_led);
static void pwm8_led_ctrl(uint8_t *data_led);	

//�޸�
// ����������̬��������Ҫ���ļ���ͷ�����ڶ���Ϊstatic��
//static int direction = 1;      // ��ˮ����1=����-1=����
//static int flow_count = 0;     // ��ˮ��������

// �����ں����ⶨ��ȫ�ֱ���
// int direction = 1;
// int flow_count = 0;


TIMER 	delay_led_on_timer;
TIMER 	delay_led_on_timer1;

//�� led_param_init �г�ʼ��
TIMER 	led_switch_timer;
TIMER 	led_switch_timer1;
TIMER 	led_switch_timer2;
TIMER 	led_switch_timer3;
TIMER 	led_switch_timer4;

uint8_t if_refresh_led_data = FALSE;

bool led_exchange_flag = FALSE;
bool led_exchange_flag1 = FALSE;
bool led_exchange_flag2 = FALSE;
bool led_exchange_flag3 = FALSE;

bool 	if_add = FALSE;   //�ƹ����ȿ���
bool 	if_add1 = FALSE;   //�ƹ����ȿ���

bool 	if_sub = FALSE;

uint8_t if_accelerate = FALSE;

uint8_t light_persent = 1;  //�ƹ�����

uint8_t light_persent_base = 30;  //�ƹ�����

uint16_t color_start_index = 0;
uint16_t color_start_index1 = 0;
uint16_t color_start_index2 = 0;

#define LED_AUDIO_LEVEL_MAX			48
#define LED_AUDIO_RAW_GATE			500
#define LED_AUDIO_RAW_LOW			1800
#define LED_AUDIO_RAW_MID			5000
#define LED_AUDIO_RAW_HIGH			12000
#define LED_AUDIO_RAW_FULL			22000
#define LED_AUDIO_FILTER_SHIFT		2
#define LED_AUDIO_RISE_STEP			3
#define LED_AUDIO_FALL_STEP			2

static uint32_t led_audio_filtered_music = 0;
static uint32_t led_audio_filtered_mic = 0;
static uint8_t led_audio_level_music = 0;
static uint8_t led_audio_level_mic = 0;

static uint8_t LedAudioLevelLimit(uint8_t current, uint8_t target)
{
	if(target > current)
	{
		uint8_t delta = target - current;
		return current + ((delta > LED_AUDIO_RISE_STEP) ? LED_AUDIO_RISE_STEP : delta);
	}
	else if(current > target)
	{
		uint8_t delta = current - target;
		return current - ((delta > LED_AUDIO_FALL_STEP) ? LED_AUDIO_FALL_STEP : delta);
	}

	return current;
}

static uint8_t LedAudioRawToLevel(uint32_t raw)
{
	if(raw <= LED_AUDIO_RAW_GATE)
	{
		return 0;
	}
	else if(raw <= LED_AUDIO_RAW_LOW)
	{
		return (uint8_t)(1 + ((raw - LED_AUDIO_RAW_GATE) * 11 / (LED_AUDIO_RAW_LOW - LED_AUDIO_RAW_GATE)));
	}
	else if(raw <= LED_AUDIO_RAW_MID)
	{
		return (uint8_t)(12 + ((raw - LED_AUDIO_RAW_LOW) * 16 / (LED_AUDIO_RAW_MID - LED_AUDIO_RAW_LOW)));
	}
	else if(raw <= LED_AUDIO_RAW_HIGH)
	{
		return (uint8_t)(28 + ((raw - LED_AUDIO_RAW_MID) * 14 / (LED_AUDIO_RAW_HIGH - LED_AUDIO_RAW_MID)));
	}
	else if(raw <= LED_AUDIO_RAW_FULL)
	{
		return (uint8_t)(42 + ((raw - LED_AUDIO_RAW_HIGH) * 5 / (LED_AUDIO_RAW_FULL - LED_AUDIO_RAW_HIGH)));
	}

	return LED_AUDIO_LEVEL_MAX;
}

static uint8_t LedAudioLevelGet(LED_TYPE channel)
{
	uint32_t raw = GetAudioSdct(channel);
	uint32_t *filtered = &led_audio_filtered_music;
	uint8_t *level = &led_audio_level_music;

	if(channel == MIC_VOL_TYPE)
	{
		filtered = &led_audio_filtered_mic;
		level = &led_audio_level_mic;
	}

	if(raw <= LED_AUDIO_RAW_GATE)
	{
		*filtered = 0;
		*level = 0;
		return 0;
	}

	*filtered = ((*filtered * 3) + raw) >> LED_AUDIO_FILTER_SHIFT;
	uint8_t target = LedAudioRawToLevel(*filtered);
	*level = LedAudioLevelLimit(*level, target);

	if(*level > LED_AUDIO_LEVEL_MAX)
	{
		*level = LED_AUDIO_LEVEL_MAX;
	}

	return *level;
}
short color_contral_param = 0;
short color_contral_param1 = 0;
short color_contral_param2 = 0;
short color_contral_param3 = 0;
short color_contral_param4 = 0;
short color_contral_param5 = 0;
short color_contral_param6 = 0;
short color_contral_param7 = 0;

short color_contral_tmp = 0;
short color_contral_tmp1 = 0;
short color_contral_tmp2 = 0;
short color_contral_tmp3 = 0;
short color_contral_tmp4 = 0;
short color_contral_tmp5 = 0;

bool led_exchange_flag_other = FALSE;
short color_contral_param_other1 = 0;
short color_contral_param_other2 = 0;
short color_contral_param_other3 = 0;
short color_contral_param_other4 = 0;

short color_contral_tmp_other1 = 0;
short color_contral_tmp_other2 = 0;
short color_contral_tmp_other3 = 0;
short color_contral_tmp_other4 = 0;

#define ClearLedDataPwm7() memset(LED_DATA_1, 0, sizeof(LED_DATA_1))
#define ClearLedDataPwm6() memset(LED_DATA_2, 0, sizeof(LED_DATA_2))
#define ClearLedDataPwm5() memset(LED_DATA_3, 0, sizeof(LED_DATA_3))
#define ClearLedDataPwm8() memset(LED_DATA_4, 0, sizeof(LED_DATA_4))

void ClearLedDataAll(void)
{
	#ifdef PWM7_LED
	ClearLedDataPwm7();
	#endif
	
	#ifdef PWM6_LED
	ClearLedDataPwm6();
	#endif
	
	#ifdef PWM5_LED
	ClearLedDataPwm5();
	#endif
	
	#ifdef PWM8_LED
	ClearLedDataPwm8();
	#endif
}

void led_param_init(void)
{
	led_exchange_flag = TRUE;
	led_exchange_flag1 = TRUE;
	led_exchange_flag2 = TRUE;
	
	if_add = FALSE;   //�ƹ����ȿ���
	if_add1 = FALSE;
	
	if_sub = FALSE;
	if_accelerate = FALSE;
	
	light_persent = 1;	//�ƹ�����
	
	light_persent_base = 30;  //�ƹ�����
	
	color_start_index = 0;
	color_start_index1 = 0;
	color_start_index2 = 0;
	
	color_contral_param = 0;
	color_contral_param1 = 0;
	color_contral_param2 = 0;
	color_contral_param3 = 0;
	color_contral_param4 = 0;
	color_contral_param5 = 0;
	color_contral_param6 = 0;
	color_contral_param7 = 0;
	
	color_contral_tmp = 0;
	color_contral_tmp1 = 0;
	color_contral_tmp2 = 0;
	color_contral_tmp3 = 0;
	color_contral_tmp4 = 0;	
	color_contral_tmp5 = 0;

	TimeOutSet(&led_switch_timer, 0);
	TimeOutSet(&led_switch_timer1, 0);
	TimeOutSet(&led_switch_timer2, 0);
	TimeOutSet(&led_switch_timer3, 0);
}

void LedEffectMusicVolum(uint8_t volum)
{
	if(led_exchange_flag_other)
	{
		ClearLedDataAll();
		led_exchange_flag_other = FALSE;
		color_contral_param_other1 = 0;
		color_contral_param_other2 = 0;
		color_contral_param_other3 = 0;
		color_contral_param_other4 = 5;

		color_contral_tmp_other1 = 0;
		color_contral_tmp_other2 = 0;
		color_contral_tmp_other3 = 0;
		color_contral_tmp_other4 = 0;

		TimeOutSet(&led_switch_timer4, 0);
		DBG("--------LedEffectMusicVolum--------%d\n", volum);
	}
	if(IsTimeOut(&led_switch_timer4))
	{
		ClearLedDataAll();
		if_refresh_led_data = 0xFF;
		switch (volum)
		{
		case 0:
			color_contral_param_other1 = 0;
			break;

		case 1 ... 3:
			color_contral_param_other1 = 1;
			break;

		case 4 ... 6:
			color_contral_param_other1 = 2;
			break;

		case 7 ... 9:
			color_contral_param_other1 = 3;
			break;

		case 10 ... 12:
			color_contral_param_other1 = 4;
			break;
			
		case 13 ... 15:
			color_contral_param_other1 = 5;
			break;
			
		case 16:
			color_contral_param_other1 = 6;
			break;
		}

		if(volum == 16)
		{
			for(uint8_t i = 0; i < color_contral_param_other1; i++)
			{
				LED_DATA_3[i * 3 + 0] = 0;
				LED_DATA_3[i * 3 + 1] = 0;
				LED_DATA_3[i * 3 + 2] = 255 * userVar.brightness / 100;
			}
		}
		else if(volum == 0)
		{
			ClearLedDataAll();
		}
		else
		{
			for(uint8_t i = 0; i < (color_contral_param_other1 - 1); i++)
			{
				LED_DATA_3[i * 3 + 0] = 0;
				LED_DATA_3[i * 3 + 1] = 0;
				LED_DATA_3[i * 3 + 2] = 255 * userVar.brightness / 100;
			}
			if(volum % 3 == 0)
			{
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 0] = 0;
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 1] = 0;
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 2] = 255 * userVar.brightness / 100;
			}
			else if(volum % 3 == 1)
			{
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 0] = 0;
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 1] = 255 * userVar.brightness / 100;
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 2] = 0;
			}
			else if(volum % 3 == 2)
			{
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 0] = 255 * userVar.brightness / 100;
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 1] = 0;
				LED_DATA_3[(color_contral_param_other1 - 1) * 3 + 2] = 0;
			}
		}
		
		TimeOutSet(&led_switch_timer4, LED_CONTRAL_TIME8);
	}
}

void LedEffectBtConnect(uint8_t flag)
{
	if(led_exchange_flag_other)
	{
		ClearLedDataAll();
		led_exchange_flag_other = FALSE;
		color_contral_param_other1 = 0;
		color_contral_param_other2 = 0;
		color_contral_param_other3 = 0;
		color_contral_param_other4 = 5;

		color_contral_tmp_other1 = 0;
		color_contral_tmp_other2 = 0;
		color_contral_tmp_other3 = 0;
		color_contral_tmp_other4 = 0;

		TimeOutSet(&led_switch_timer4, 0);
		DBG("--------LedEffectBtConnect--------\n");
	}
	if(IsTimeOut(&led_switch_timer4))
	{
		if(!color_contral_tmp_other3) ClearLedDataAll();
		if_refresh_led_data = 0xFF;
		
		if(color_contral_param_other1 < color_contral_param_other4)
		{
			color_contral_param_other1++;
		}
		else
		{
			color_contral_param_other4--;
			color_contral_param_other1 = -1;
		}

		for(uint8_t i = color_contral_param_other4 + 1; i < 6; i++)
		{
			color_contral_tmp_other1 = (5 - i) * 255;
			LED_DATA_3[i * 3 + 0] = grb[color_contral_tmp_other1 * 3 + 0] * userVar.brightness / 100;
			LED_DATA_3[i * 3 + 1] = grb[color_contral_tmp_other1 * 3 + 1] * userVar.brightness / 100;
			LED_DATA_3[i * 3 + 2] = grb[color_contral_tmp_other1 * 3 + 2] * userVar.brightness / 100;
		}

		if(color_contral_param_other4 == -1)
		{
			if(!color_contral_tmp_other3) 
			{
				TimeOutSet(&userVar.OtherLedTimer, LED_CONTRAL_TIME10);
				color_contral_tmp_other3 = 1;
			}
			TimeOutSet(&led_switch_timer4, LED_CONTRAL_TIME8);
			return;
		}

		color_contral_tmp_other1 = (5 - color_contral_param_other1) * 255;
		LED_DATA_3[color_contral_param_other1 * 3 + 0] = grb[color_contral_tmp_other1 * 3 + 0] * userVar.brightness / 100;
		LED_DATA_3[color_contral_param_other1 * 3 + 1] = grb[color_contral_tmp_other1 * 3 + 1] * userVar.brightness / 100;
		LED_DATA_3[color_contral_param_other1 * 3 + 2] = grb[color_contral_tmp_other1 * 3 + 2] * userVar.brightness / 100;
		
		TimeOutSet(&led_switch_timer4, LED_CONTRAL_TIME8);
	}
}

void LedEffectLedBrightness(uint8_t brightness)
{
	
	if(led_exchange_flag_other)
	{
		ClearLedDataAll();
		led_exchange_flag_other = FALSE;
		color_contral_param_other1 = 0;
		color_contral_param_other2 = 0;
		color_contral_param_other3 = 0;
		color_contral_param_other4 = 5;

		color_contral_tmp_other1 = 0;
		color_contral_tmp_other2 = 0;
		color_contral_tmp_other3 = 0;
		color_contral_tmp_other4 = 0;

		TimeOutSet(&led_switch_timer4, 0);
		DBG("--------LedEffectLedBrightness--------%d\n", brightness);
	}
	if(IsTimeOut(&led_switch_timer4))
	{
		ClearLedDataAll();
		if_refresh_led_data = 0xFF;

		// ��������ֵ����LED_DATA_3Ҫ������LED����
		switch (brightness)
		{
		case 0 ... 20:     // �������
			color_contral_param_other1 = 10;
			break;

		case 21 ... 40:
			color_contral_param_other1 = 19;
			break;

		case 41 ... 60:
			color_contral_param_other1 = 29;
			break;

		case 61 ... 80:
			color_contral_param_other1 = 38;
			break;

		case 81 ... 100:   // �������
			color_contral_param_other1 = 48;
			break;
		}

		// LED_DATA_3 ȫ�������48�����飩
		for(uint8_t i = 0; i < 48; i++)
		{
		    uint8_t r = 0, g = 0, b = 0;

		    if (i < 16)
		    {
		        // �� �� �� (0-15)
		        r = 255 - (i * 16);
		        g = i * 16;
		        b = 0;
		    }
		    else if (i < 32)
		    {
		        // �� �� �� (16-31)
		        r = 0;
		        g = 255 - ((i - 16) * 16);
		        b = (i - 16) * 16;
		    }
		    else
		    {
		        // �� �� �� (32-47)
		        r = (i - 32) * 16;
		        g = 0;
		        b = 255 - ((i - 32) * 16);
		    }

		    LED_DATA_3[i * 3 + 0] = r * userVar.brightness / 100;
		    LED_DATA_3[i * 3 + 1] = g * userVar.brightness / 100;
		    LED_DATA_3[i * 3 + 2] = b * userVar.brightness / 100;
		}


		// ========== LED_DATA_2 �������ȵ�λ������Ӧ�����ĵ��� ===========
		if(brightness == 100)
		{
		    // ֱ�Ӹ��� LED_DATA_3 ����ɫ�� LED_DATA_2
		    for(uint8_t i = 0; i < color_contral_param_other1; i++)
		    {
		        LED_DATA_2[i*3 + 0] = LED_DATA_3[i*3 + 0];
		        LED_DATA_2[i*3 + 1] = LED_DATA_3[i*3 + 1];
		        LED_DATA_2[i*3 + 2] = LED_DATA_3[i*3 + 2];
		    }
		}

		else if(brightness == 0)
		{
			// ����0%ʱ��LED_DATA_2ȫ��Ϩ�𣨵�LED_DATA_3����ȫ����
			// ����Ҫ�����������ΪClearLedDataAll�Ѿ����
		}
		else
		{
		    for(uint8_t i = 0; i < color_contral_param_other1; i++)
		    {
		        uint8_t r = 0, g = 0, b = 0;
		        uint8_t pos = i % 48;

		        if (pos < 16)
		        {
		            r = 255 - (pos * 16);
		            g = pos * 16;
		            b = 0;
		        }
		        else if (pos < 32)
		        {
		            r = 0;
		            g = 255 - ((pos - 16) * 16);
		            b = (pos - 16) * 16;
		        }
		        else
		        {
		            r = (pos - 32) * 16;
		            g = 0;
		            b = 255 - ((pos - 32) * 16);
		        }

		        LED_DATA_2[i * 3 + 0] = r * userVar.brightness / 100;
		        LED_DATA_2[i * 3 + 1] = g * userVar.brightness / 100;
		        LED_DATA_2[i * 3 + 2] = b * userVar.brightness / 100;
		    }
		}
		TimeOutSet(&led_switch_timer4, LED_CONTRAL_TIME8);
	}
}
void LedEffectPowerOn()
{
    static uint8_t flow_phase = 0;      // 0:�����׶�, 1:Ϩ��׶�
    static uint8_t led_index = 0;       // ��ǰ������LED����

    if (led_exchange_flag)
    {
        ClearLedDataAll();
        led_exchange_flag = FALSE;

        color_contral_param   = -1;
        color_contral_tmp     = 0;
        light_persent         = 0;
        flow_phase = 0;              // �����׶�
        led_index = 0;               // �ӵ�0�ſ�ʼ
        TimeOutSet(&led_switch_timer, 0);

        DBG("--------LedEffectPowerOn--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        if(flow_phase == 0)  // �����׶�
        {
            if(led_index < LED_NUM2)
            {
                // ������ǰLED
                color_contral_tmp = led_index * 1530 / LED_NUM2;

                LED_DATA_3[led_index * 3 + 0] = grb[color_contral_tmp * 3 + 0] * userVar.brightness / 100;
                LED_DATA_3[led_index * 3 + 1] = grb[color_contral_tmp * 3 + 1] * userVar.brightness / 100;
                LED_DATA_3[led_index * 3 + 2] = grb[color_contral_tmp * 3 + 2] * userVar.brightness / 100;
                LED_DATA_2[led_index * 3 + 0] = grb[color_contral_tmp * 3 + 0] * userVar.brightness / 100;
                LED_DATA_2[led_index * 3 + 1] = grb[color_contral_tmp * 3 + 1] * userVar.brightness / 100;
                LED_DATA_2[led_index * 3 + 2] = grb[color_contral_tmp * 3 + 2] * userVar.brightness / 100;

                led_index++;
            }
            else
            {
                // ����LED���ѵ������л���Ϩ��׶�
                flow_phase = 1;
                led_index = 0;
                TimeOutSet(&led_switch_timer, 15);
                return;
            }
        }
        else  // Ϩ��׶�
        {
            if(led_index < LED_NUM2)
            {
                // Ϩ��ǰLED
                LED_DATA_3[led_index * 3 + 0] = 0;
                LED_DATA_3[led_index * 3 + 1] = 0;
                LED_DATA_3[led_index * 3 + 2] = 0;
                LED_DATA_2[led_index * 3 + 0] = 0;
                LED_DATA_2[led_index * 3 + 1] = 0;
                LED_DATA_2[led_index * 3 + 2] = 0;

                led_index++;
            }
            else
            {
                // ����LED����Ϩ�𣬽�����Ч
                LedEffectSwitch(userVar.led_mode_bak, TRUE, TRUE);
                TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME10);
                return;
            }
        }

        TimeOutSet(&led_switch_timer, 15);
        if_refresh_led_data = 0xFF;
    }
}
void LedEffectPowerOff()
{
	if (led_exchange_flag)
	{
		ClearLedDataAll();
		led_exchange_flag = FALSE;
		
		color_contral_param 	= LED_NUM2;
		color_contral_tmp 		= 0;
		light_persent		 	= 0;
		TimeOutSet(&led_switch_timer, 0);

		for(uint8_t i = 0; i < LED_NUM2; i++)
		{
			color_contral_tmp = (11 - i) * 127;
			LED_DATA_3[i * 3 + 0] = grb[color_contral_tmp * 3 + 0]  * userVar.brightness / 100;
			LED_DATA_3[i * 3 + 1] = grb[color_contral_tmp * 3 + 0] * userVar.brightness / 100;
			LED_DATA_3[i * 3 + 2] = grb[color_contral_tmp * 3 + 0]  * userVar.brightness / 100;
		}

		RemindSoundServiceItemRequest(SOUND_REMIND_GUANJI, REMIND_PRIO_NORMAL);
		
		DBG("--------LedEffectPowerOff--------\n");
	}
	if (IsTimeOut(&led_switch_timer))
	{
		if(color_contral_param > 0)
		{
			color_contral_param--;
		}
		else
		{
			MainTaskMsgSend(MSG_POWERDOWN);//MSG_DEEPSLEEP
			TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME8);
			return;
		}

	
		if_refresh_led_data = 0xFF;

		TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME10 - 200);
	}
}

/**
 * LED��ƽ��Ч�� - ģʽһ�����������ƺ����ּ��٣�
 * Ч������ɫ(������) -> ��ɫ(������) -> ��ɫ(������) ����
 * ����Խ�󣬵����ĵ�������Խ��
 * ���ֲ���ʱ�������ٶȻ��������仯���ӿ�
 */

// ��ֵ���ֺ����ּ�����ؾ�̬����
static uint8_t peak_hold_value_led1 = 0;      // ��ǰ��ֵ���ֵĵ�ƽֵ
static uint8_t peak_decay_timer_led1 = 0;     // ��ֵ˥����ʱ��
static uint8_t if_accelerate_led1 = FALSE;    // ���ּ��ٱ�־
static uint8_t accelerate_counter_led1 = 0;   // ���ټ�����
static uint8_t volume_level_led1 = 0;         // ��ǰ������ƽֵ

static uint8_t peak_hold_value_led1_3 = 0;   // LED_DATA_3�ķ�ֵ
static uint8_t peak_hold_value_led1_2 = 0;   // LED_DATA_2�ķ�ֵ
static uint8_t peak_decay_timer_led1_3 = 0;  // LED_DATA_3��˥����ʱ��
static uint8_t peak_decay_timer_led1_2 = 0;  // LED_DATA_2��˥����ʱ��

void LedEffect1()
{
    if (led_exchange_flag)
    {
        ClearLedDataAll();

        led_exchange_flag = FALSE;

        color_contral_param   = 0;
        color_contral_tmp     = 0;
        light_persent         = 0;
        color_start_index     = 0;

        peak_hold_value_led1_3 = 0;
        peak_hold_value_led1_2 = 0;
        peak_decay_timer_led1_3 = 0;
        peak_decay_timer_led1_2 = 0;

        TimeOutSet(&led_switch_timer, 0);
        TimeOutSet(&led_switch_timer1, 0);

        DBG("--------LedEffect1--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        ClearLedDataAll();

        // LED3 = ��������
        uint8_t volume_level_led3 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MUSIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led3 = volume;
        }
        else
        {
            volume_level_led3 = 0;
        }

        // LED2 = ��˷�����
        uint8_t volume_level_led2 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led2 = volume;
        }
        else
        {
            volume_level_led2 = 0;
        }

        // ��������������ʹ��
        volume_level_led1 = volume_level_led3;

        // ====================== LED_DATA_3 �̶���ɫ���� ======================
        for(uint8_t i = 0; i < volume_level_led3; i++)
        {
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            r_val = 255 * userVar.brightness / 100;
            g_val = 255 * userVar.brightness / 100;
            b_val = 255 * userVar.brightness / 100;

            LED_DATA_3[i * 3 + 0] = r_val;
            LED_DATA_3[i * 3 + 1] = g_val;
            LED_DATA_3[i * 3 + 2] = b_val;
        }

        // ====================== LED_DATA_2 �̶���ɫ���� ======================
        for(uint8_t i = 0; i < volume_level_led2; i++)
        {
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            r_val = 255 * userVar.brightness / 100;
            g_val = 255 * userVar.brightness / 100;
            b_val = 255 * userVar.brightness / 100;

            LED_DATA_2[i * 3 + 0] = r_val;
            LED_DATA_2[i * 3 + 1] = g_val;
            LED_DATA_2[i * 3 + 2] = b_val;
        }

        // ====================== LED_DATA_3 peak hold ======================
        if(volume_level_led3 > peak_hold_value_led1_3)
        {
            peak_hold_value_led1_3 = volume_level_led3;
            peak_decay_timer_led1_3 = 2;
        }

        if(peak_decay_timer_led1_3 > 0)
        {
            peak_decay_timer_led1_3--;
            if(peak_decay_timer_led1_3 == 0 && peak_hold_value_led1_3 > 0)
            {
                peak_hold_value_led1_3--;
                peak_decay_timer_led1_3 = 2;
            }
        }

        // ====================== LED_DATA_2 peak hold ======================
        if(volume_level_led2 > peak_hold_value_led1_2)
        {
            peak_hold_value_led1_2 = volume_level_led2;
            peak_decay_timer_led1_2 = 2;
        }

        if(peak_decay_timer_led1_2 > 0)
        {
            peak_decay_timer_led1_2--;
            if(peak_decay_timer_led1_2 == 0 && peak_hold_value_led1_2 > 0)
            {
                peak_hold_value_led1_2--;
                peak_decay_timer_led1_2 = 2;
            }
        }

        // ====================== peak dot display ======================
        if(peak_hold_value_led1_3 > 0 && peak_hold_value_led1_3 <= 48)
        {
            uint8_t peak_idx = peak_hold_value_led1_3 - 1;
            uint8_t bright_val = 255 * userVar.brightness / 100;

            LED_DATA_3[peak_idx * 3 + 0] = bright_val;
            LED_DATA_3[peak_idx * 3 + 1] = bright_val;
            LED_DATA_3[peak_idx * 3 + 2] = bright_val;
        }

        if(peak_hold_value_led1_2 > 0 && peak_hold_value_led1_2 <= 48)
        {
            uint8_t peak_idx = peak_hold_value_led1_2 - 1;
            uint8_t bright_val = 255 * userVar.brightness / 100;

            LED_DATA_2[peak_idx * 3 + 0] = bright_val;
            LED_DATA_2[peak_idx * 3 + 1] = bright_val;
            LED_DATA_2[peak_idx * 3 + 2] = bright_val;
        }

        // ========== ���ּ��� ==========
        if(userVar.if_music_play)
        {
            if(!if_accelerate_led1 && IsTimeOut(&led_switch_timer1))
            {
                uint16_t volume_raw = GetAudioSdct(MUSIC_VOL_TYPE);
                uint8_t volume_strength = volume_raw / 1300;
                if(volume_strength > 4) volume_strength = 4;

                if(volume_strength == 0)
                    accelerate_counter_led1 = 7;
                else if(volume_strength == 1)
                    accelerate_counter_led1 = 9;
                else if(volume_strength == 2)
                    accelerate_counter_led1 = 12;
                else if(volume_strength == 3)
                    accelerate_counter_led1 = 17;
                else if(volume_strength == 4)
                    accelerate_counter_led1 = 35;

                if_accelerate_led1 = TRUE;
                TimeOutSet(&led_switch_timer1, ((70 - volume_strength * 20) * accelerate_counter_led1 + 500));
            }

            if(if_accelerate_led1)
            {
                if(accelerate_counter_led1 > 0)
                    accelerate_counter_led1--;
                else
                    if_accelerate_led1 = FALSE;

                TimeOutSet(&led_switch_timer, 70 - volume_level_led1 * 2);
            }
            else
            {
                TimeOutSet(&led_switch_timer, 80);
            }
        }
        else
        {
            if_accelerate_led1 = FALSE;
            accelerate_counter_led1 = 0;
            TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME8);
        }

        if_refresh_led_data = 0xFF;
    }
}
// ��ֵ������ؾ�̬����
static uint8_t peak_hold_value_led2 = 0;   // LED_DATA_2��ǰ��ֵ���ֵĵ�ƽֵ
static uint8_t peak_hold_value_led3 = 0;   // LED_DATA_3��ǰ��ֵ���ֵĵ�ƽֵ
static uint8_t peak_decay_timer = 0;       // ��ֵ˥����ʱ��
static uint8_t if_accelerate_led2 = FALSE;    // ���ּ��ٱ�־
static uint8_t accelerate_counter_led2 = 0;   // ���ټ�����
static uint8_t volume_strength_value = 0;     // ����ǿ��ֵ



void LedEffect2()
{
    if (led_exchange_flag)
    {
        ClearLedDataAll();

        led_exchange_flag = FALSE;

        color_contral_param   = 0;
        color_contral_tmp     = 0;
        light_persent         = 0;
        color_start_index     = 0;


        peak_hold_value_led1_3 = 0;
        peak_decay_timer_led1_3 = 0;

        TimeOutSet(&led_switch_timer, 0);
        TimeOutSet(&led_switch_timer1, 0);

        DBG("--------LedEffect1--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        ClearLedDataAll();

        // LED3 = ��������
        uint8_t volume_level_led3 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MUSIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led3 = volume;
        }
        else
        {
            volume_level_led3 = 0;
        }

        // LED2 = ��˷�����
        uint8_t volume_level_led2 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led2 = volume;
        }
        else
        {
            volume_level_led2 = 0;
        }

        // ��������������ʹ��
        volume_level_led1 = volume_level_led3;

        // ====================== LED_DATA_3 �̶���ɫ���� ======================
        for(uint8_t i = 0; i < volume_level_led3; i++)
        {
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            if(i <= 34)        // 1~35 �� ��ɫ
            {
                g_val = 255 * userVar.brightness / 100;
            }
            else if(i <= 44)   // 36~45 �� ��ɫ
            {
                r_val = 255 * userVar.brightness / 100;
                g_val = 255 * userVar.brightness / 100;
            }
            else               // 46~48 �� ��ɫ
            {
                r_val = 255 * userVar.brightness / 100;
            }

            LED_DATA_3[i * 3 + 0] = r_val;
            LED_DATA_3[i * 3 + 1] = g_val;
            LED_DATA_3[i * 3 + 2] = b_val;
        }

        // ====================== LED_DATA_2 �̶���ɫ���� ======================
        for(uint8_t i = 0; i < volume_level_led2; i++)
        {
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            if(i <= 34)
            {
                g_val = 255 * userVar.brightness / 100;
            }
            else if(i <= 44)
            {
                r_val = 255 * userVar.brightness / 100;
                g_val = 255 * userVar.brightness / 100;
            }
            else
            {
                r_val = 255 * userVar.brightness / 100;
            }

            LED_DATA_2[i * 3 + 0] = r_val;
            LED_DATA_2[i * 3 + 1] = g_val;
            LED_DATA_2[i * 3 + 2] = b_val;
        }

        //LED3��ֵ
        if(volume_level_led3 > peak_hold_value_led1_3)
        {
            peak_hold_value_led1_3 = volume_level_led3;
            peak_decay_timer_led1_3 = 2;
        }

        if(peak_decay_timer_led1_3 > 0)
        {
            peak_decay_timer_led1_3--;
            if(peak_decay_timer_led1_3 == 0 && peak_hold_value_led1_3 > 0)
            {
                peak_hold_value_led1_3--;
                peak_decay_timer_led1_3 = 2;
            }
        }

        // ====================== LED2 ��ֵ ======================
               if(volume_level_led2 > peak_hold_value_led1_2)
               {
                   peak_hold_value_led1_2 = volume_level_led2;
                   peak_decay_timer_led1_2 = 2;
               }

               if(peak_decay_timer_led1_2 > 0)
               {
                   peak_decay_timer_led1_2--;
                   if(peak_decay_timer_led1_2 == 0 && peak_hold_value_led1_2 > 0)
                   {
                       peak_hold_value_led1_2--;
                       peak_decay_timer_led1_2 = 2;
                   }
               }

        // ====================== ��ֵ��ɫ��λ�ñ仯 ======================
        if(peak_hold_value_led1_3 > 0 && peak_hold_value_led1_3 <= 48)
        {
            uint8_t peak_idx = peak_hold_value_led1_3 - 1;
            uint8_t r_val = 0, g_val = 0, b_val = 0;


            if(peak_idx <= 34)       // ��ɫ��
            {
                g_val = 255 * userVar.brightness / 100;
            }
            else if(peak_idx <= 44)  // ��ɫ��
            {
                r_val = 255 * userVar.brightness / 100;
                g_val = 255 * userVar.brightness / 100;
            }
            else                     // ��ɫ��
            {
                r_val = 255 * userVar.brightness / 100;
            }

            LED_DATA_3[peak_idx * 3 + 0] = r_val;
            LED_DATA_3[peak_idx * 3 + 1] = g_val;
            LED_DATA_3[peak_idx * 3 + 2] = b_val;
        }

        if(peak_hold_value_led1_2 > 0 && peak_hold_value_led1_2 <= 48)
                {
                    uint8_t peak_idx = peak_hold_value_led1_2 - 1;
                    uint8_t r_val = 0, g_val = 0, b_val = 0;


                    if(peak_idx <= 34)       // ��ɫ��
                    {
                        g_val = 255 * userVar.brightness / 100;
                    }
                    else if(peak_idx <= 44)  // ��ɫ��
                    {
                        r_val = 255 * userVar.brightness / 100;
                        g_val = 255 * userVar.brightness / 100;
                    }
                    else                     // ��ɫ��
                    {
                        r_val = 255 * userVar.brightness / 100;
                    }

                    LED_DATA_2[peak_idx * 3 + 0] = r_val;
                    LED_DATA_2[peak_idx * 3 + 1] = g_val;
                    LED_DATA_2[peak_idx * 3 + 2] = b_val;
                }


        // ========== ���ּ��� ==========
        if(userVar.if_music_play)
        {
            if(!if_accelerate_led1 && IsTimeOut(&led_switch_timer1))
            {
                uint16_t volume_raw = GetAudioSdct(MUSIC_VOL_TYPE);
                uint8_t volume_strength = volume_raw / 1300;
                if(volume_strength > 4) volume_strength = 4;

                if(volume_strength == 0)
                    accelerate_counter_led1 = 7;
                else if(volume_strength == 1)
                    accelerate_counter_led1 = 9;
                else if(volume_strength == 2)
                    accelerate_counter_led1 = 12;
                else if(volume_strength == 3)
                    accelerate_counter_led1 = 17;
                else if(volume_strength == 4)
                    accelerate_counter_led1 = 35;

                if_accelerate_led1 = TRUE;
                TimeOutSet(&led_switch_timer1, ((70 - volume_strength * 20) * accelerate_counter_led1 + 500));
            }

            if(if_accelerate_led1)
            {
                if(accelerate_counter_led1 > 0)
                    accelerate_counter_led1--;
                else
                    if_accelerate_led1 = FALSE;

                TimeOutSet(&led_switch_timer, 70 - volume_level_led1 * 2);
            }
            else
            {
                TimeOutSet(&led_switch_timer, 80);
            }
        }
        else
        {
            if_accelerate_led1 = FALSE;
            accelerate_counter_led1 = 0;
            TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME8);

            // ====================== û����ʱ��ֵ�������� ======================
            if(peak_hold_value_led1_3 > 0)
            {
                peak_decay_timer_led1_3--;
                if(peak_decay_timer_led1_3 == 0)
                {
                    peak_hold_value_led1_3--;
                    peak_decay_timer_led1_3 = 2;
                }
            }
        }

        if_refresh_led_data = 0xFF;
    }
}
void LedEffect3()
{
    if (led_exchange_flag)
    {
        led_exchange_flag = FALSE;
        ClearLedDataAll();

        color_start_index    = 0;
        color_contral_tmp    = 0;
        color_contral_tmp1   = 0;
        color_contral_param  = 0;
        color_contral_param1 = 0;
        color_contral_param2 = 0;

        if_accelerate        = FALSE;
        color_contral_param6 = 0;
        color_contral_param7 = 0;

        // ���ּ��ٱ�����ʼ��
        if_accelerate_led2 = FALSE;
        accelerate_counter_led2 = 0;
        volume_strength_value = 0;

        TimeOutSet(&led_switch_timer, 0);
        TimeOutSet(&led_switch_timer1, 0);

        DBG("--------LedEffect8--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        ClearLedDataAll();

        // ========== 1. LED3 ʹ�ã��������� ======================
        uint8_t volume_level_music = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MUSIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_music = volume;
        }
        else
        {
            volume_level_music = 0;
        }

        // ========== 2. LED2 ʹ�ã���˷������������ֲ�һ����======================
        uint8_t volume_level_mic = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MIC_VOL_TYPE); // ����ĳ� MIC
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_mic = volume;
        }
        else
        {
            volume_level_mic = 0;
        }

        // ========== ���� LED_DATA_3������������==========
        for(uint8_t i = 0; i < volume_level_music; i++)
        {
            uint8_t red_val = 255 * userVar.brightness / 100;
            LED_DATA_3[i * 3 + 0] = 0;
            LED_DATA_3[i * 3 + 1] = red_val;
            LED_DATA_3[i * 3 + 2] = 0;
        }

        // ========== ���� LED_DATA_2����˷���������ͬ����==========
        for(uint8_t i = 0; i < volume_level_mic; i++)
        {
            uint8_t red_val = 255 * userVar.brightness / 100;
            LED_DATA_2[i * 3 + 0] = 0;
            LED_DATA_2[i * 3 + 1] = red_val;
            LED_DATA_2[i * 3 + 2] = 0;
        }

        // ========== ���ּ����߼�
        if(userVar.if_music_play)
        {
            // ���ٴ����߼�
            if(!if_accelerate_led2 && IsTimeOut(&led_switch_timer1))
            {
                // ��������ǿ�ȼ�����ٲ���
                volume_strength_value = GetLedSwitchTime1(2) / 2;

                if(volume_strength_value == 0)
                    accelerate_counter_led2 = 7;
                else if(volume_strength_value == 1)
                    accelerate_counter_led2 = 9;
                else if(volume_strength_value == 2)
                    accelerate_counter_led2 = 12;
                else if(volume_strength_value == 3)
                    accelerate_counter_led2 = 17;
                else if(volume_strength_value == 4)
                    accelerate_counter_led2 = 35;
                else
                    accelerate_counter_led2 = 35;

                if_accelerate_led2 = TRUE;
                TimeOutSet(&led_switch_timer1, ((70 - volume_strength_value * 20) * accelerate_counter_led2 + 500));
            }

            // ����ִ���߼�
            if(if_accelerate_led2)
            {
                if(accelerate_counter_led2 > 0)
                {
                    accelerate_counter_led2--;
                }
                else
                {
                    if_accelerate_led2 = FALSE;
                }

                // ����Խ��ˢ�¼��Խ�̣�������������
                uint8_t refresh_speed = 70 - volume_strength_value * 10;
                if(refresh_speed < LED_CONTRAL_TIME4)
                {
                    refresh_speed = LED_CONTRAL_TIME4;
                }
                TimeOutSet(&led_switch_timer, refresh_speed);
            }
            else
            {
                // ����ˢ��ģʽ
                TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
            }
        }
        else
        {
            // ������ʱ�����ü��ٱ�־
            if_accelerate_led2 = FALSE;
            accelerate_counter_led2 = 0;
            volume_strength_value = 0;
            TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
        }

        if_refresh_led_data = 0xFF;
    }
}

void LedEffect4()
{
    if (led_exchange_flag)
    {
        led_exchange_flag = FALSE;
        ClearLedDataAll();

        color_start_index    = 0;
        color_contral_tmp    = 0;
        color_contral_tmp1   = 0;
        color_contral_param  = 0;
        color_contral_param1 = 0;
        color_contral_param2 = 0;

        if_accelerate        = FALSE;
        color_contral_param6 = 0;
        color_contral_param7 = 0;

        // ��ֵ���ֱ�����ʼ��
        peak_hold_value_led2 = 0;
        peak_hold_value_led3 = 0;
        peak_decay_timer = 0;

        // ���ּ��ٱ�����ʼ��
        if_accelerate_led2 = FALSE;
        accelerate_counter_led2 = 0;
        volume_strength_value = 0;

        TimeOutSet(&led_switch_timer, 0);
        TimeOutSet(&led_switch_timer1, 0);

        DBG("--------LedEffect7--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        ClearLedDataAll();

        // LED3 ʹ�ã��������� MUSIC
        uint8_t volume_level_led3 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MUSIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led3 = volume;
        }
        else
        {
            volume_level_led3 = 0;
        }

        // LED2 ʹ�ã���˷����� MIC�������ֲ�һ����
        uint8_t volume_level_led2 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led2 = volume;
        }
        else
        {
            volume_level_led2 = 0;
        }

        // ====================== ��ɫ������������ ======================
        static uint8_t color_flow = 0;
        color_flow += 2;  // �����ٶ�

        // ========== 2. ���� LED_DATA_3��ʹ�ö���������==========
        for(uint8_t i = 0; i < volume_level_led3; i++)
        {
            LED_DATA_3[i * 3 + 0] =255 * userVar.brightness / 100;
            LED_DATA_3[i * 3 + 1] = 0;
            LED_DATA_3[i * 3 + 2] = 0;
        }

        // ========== 3. ���� LED_DATA_2��ʹ�ö���������==========
        for(uint8_t i = 0; i < volume_level_led2; i++)
        {
            LED_DATA_2[i * 3 + 0] = 255 * userVar.brightness / 100;
            LED_DATA_2[i * 3 + 1] = 0;
            LED_DATA_2[i * 3 + 2] = 0;
        }

        // ====================== LED3 ��ֵ�߼���ʹ�ö���������======================
        if(volume_level_led3 > peak_hold_value_led1_3)
        {
            peak_hold_value_led1_3 = volume_level_led3;
            peak_decay_timer_led1_3 = 2;
        }

        if(peak_decay_timer_led1_3 > 0)
        {
            peak_decay_timer_led1_3--;
            if(peak_decay_timer_led1_3 == 0 && peak_hold_value_led1_3 > 0)
            {
                peak_hold_value_led1_3--;
                peak_decay_timer_led1_3 = 2;
            }
        }

        // ====================== LED2 ��ֵ�߼���ʹ�ö���������======================
        if(volume_level_led2 > peak_hold_value_led1_2)
        {
            peak_hold_value_led1_2 = volume_level_led2;
            peak_decay_timer_led1_2 = 3;
        }

        if(peak_decay_timer_led1_2 > 0)
        {
            peak_decay_timer_led1_2--;
            if(peak_decay_timer_led1_2 == 0 && peak_hold_value_led1_2 > 0)
            {
                peak_hold_value_led1_2--;
                peak_decay_timer_led1_2 = 3;
            }
        }

        // ====================== ��ֵ �� ����ɫ ======================
        uint8_t r=0,g=0,b=0;
        uint8_t color = color_flow;
        if(color < 85)
        {
            r = 255 - color*3;
            g = color*3;
            b = 0;
        }
        else if(color < 170)
        {
            color -= 85;
            r = 0;
            g = 255 - color*3;
            b = color*3;
        }
        else
        {
            color -= 170;
            r = color*3;
            g = 0;
            b = 255 - color*3;
        }
        r = r * userVar.brightness / 100;
        g = g * userVar.brightness / 100;
        b = b * userVar.brightness / 100;

        // ========== ��ֵ����ʾ������ɫ�� ==========
        if(peak_hold_value_led1_3 > 0 && peak_hold_value_led1_3 <= 48)
        {
            uint8_t peak_idx = peak_hold_value_led1_3 - 1;
            LED_DATA_3[peak_idx * 3 + 0] = g;
            LED_DATA_3[peak_idx * 3 + 1] = r;
            LED_DATA_3[peak_idx * 3 + 2] = b;
        }

        if(peak_hold_value_led1_2 > 0 && peak_hold_value_led1_2 <= 48)
        {
            uint8_t peak_idx = peak_hold_value_led1_2 - 1;
            LED_DATA_2[peak_idx * 3 + 0] = g;
            LED_DATA_2[peak_idx * 3 + 1] = r;
            LED_DATA_2[peak_idx * 3 + 2] = b;
        }

        // ========== ���ּ����߼� ==========
        if(userVar.if_music_play)
        {
            // ���ٴ����߼�
            if(!if_accelerate_led2 && IsTimeOut(&led_switch_timer1))
            {
                // ��������ǿ�ȼ�����ٲ���
                volume_strength_value = GetLedSwitchTime1(2) / 2;

                if(volume_strength_value == 0)
                    accelerate_counter_led2 = 7;
                else if(volume_strength_value == 1)
                    accelerate_counter_led2 = 9;
                else if(volume_strength_value == 2)
                    accelerate_counter_led2 = 12;
                else if(volume_strength_value == 3)
                    accelerate_counter_led2 = 17;
                else if(volume_strength_value == 4)
                    accelerate_counter_led2 = 35;
                else
                    accelerate_counter_led2 = 35;

                if_accelerate_led2 = TRUE;
                TimeOutSet(&led_switch_timer1, ((70 - volume_strength_value * 20) * accelerate_counter_led2 + 500));
            }

            // ����ִ���߼�
            if(if_accelerate_led2)
            {
                if(accelerate_counter_led2 > 0)
                    accelerate_counter_led2--;
                else
                    if_accelerate_led2 = FALSE;
            }
            else
            {
                TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
            }
        }
        else
        {
            if_accelerate_led2 = FALSE;
            accelerate_counter_led2 = 0;
            volume_strength_value = 0;
            TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
        }

        if_refresh_led_data = 0xFF;
    }
}

void LedEffect5()
{
    if (led_exchange_flag)
    {
        led_exchange_flag = FALSE;
        ClearLedDataAll();

        color_start_index    = 0;
        color_contral_tmp    = 0;
        color_contral_tmp1   = 0;
        color_contral_param  = 0;
        color_contral_param1 = 0;
        color_contral_param2 = 0;

        if_accelerate        = FALSE;
        color_contral_param6 = 0;
        color_contral_param7 = 0;

        // ���ּ��ٱ�����ʼ��
        if_accelerate_led2 = FALSE;
        accelerate_counter_led2 = 0;
        volume_strength_value = 0;

        TimeOutSet(&led_switch_timer, 0);
        TimeOutSet(&led_switch_timer1, 0);

        DBG("--------LedEffect5--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        ClearLedDataAll();

        // LED3 ʹ�ã��������� MUSIC
        uint8_t volume_level_led3 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MUSIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led3 = volume;
        }
        else
        {
            volume_level_led3 = 0;
        }

        // LED2 ʹ�ã���˷����� MIC
        uint8_t volume_level_led2 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led2 = volume;
        }
        else
        {
            volume_level_led2 = 0;
        }

        // �̶����� 2 �����
        int16_t light_len = 2;
        int16_t end_pos = volume_level_led3;
        int16_t start_pos = end_pos - light_len;
        if(start_pos < 0) start_pos = 0;

        // LED2 ʹ���Լ���������������ͬ����
        int16_t end_pos_led2 = volume_level_led2 + 1;
        int16_t start_pos_led2 = (volume_level_led2 - light_len) + 1;
        if(end_pos_led2 > 48) end_pos_led2 = 48;
        if(start_pos_led2 < 0) start_pos_led2 = 0;

        // ========== ���� LED_DATA_3��ԭʼλ�ã� ==========
        for(uint8_t i = start_pos; i < end_pos; i++)
        {
            uint8_t red_val = 255 * userVar.brightness / 100;

            // fixed red: R=255, G=0, B=0
            LED_DATA_3[i * 3 + 0] = red_val;
            LED_DATA_3[i * 3 + 1] = 0;
            LED_DATA_3[i * 3 + 2] = 0;
        }

        // ========== 3. ���� LED_DATA_2��ƫ��+1������һλ�� ==========
        for(uint8_t i = start_pos_led2; i < end_pos_led2; i++)
        {
            uint8_t red_val = 255 * userVar.brightness / 100;

            // fixed red: R=255, G=0, B=0
            LED_DATA_2[i * 3 + 0] = red_val;
            LED_DATA_2[i * 3 + 1] = 0;
            LED_DATA_2[i * 3 + 2] = 0;
        }

        // ========== ���ּ����߼� ==========
        if(userVar.if_music_play)
        {
            if(!if_accelerate_led2 && IsTimeOut(&led_switch_timer1))
            {
                volume_strength_value = GetLedSwitchTime1(2) / 2;

                if(volume_strength_value == 0)      accelerate_counter_led2 = 7;
                else if(volume_strength_value == 1) accelerate_counter_led2 = 9;
                else if(volume_strength_value == 2) accelerate_counter_led2 = 12;
                else if(volume_strength_value == 3) accelerate_counter_led2 = 17;
                else                                accelerate_counter_led2 = 35;

                if_accelerate_led2 = TRUE;
                TimeOutSet(&led_switch_timer1, ((70 - volume_strength_value * 20) * accelerate_counter_led2 + 500));
            }

            if(if_accelerate_led2)
            {
                if(accelerate_counter_led2 > 0)
                    accelerate_counter_led2--;
                else
                    if_accelerate_led2 = FALSE;

                uint8_t refresh_speed = 70 - volume_strength_value * 10;
                if(refresh_speed < LED_CONTRAL_TIME4)
                    refresh_speed = LED_CONTRAL_TIME4;

                TimeOutSet(&led_switch_timer, refresh_speed);
            }
            else
            {
                TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
            }
        }
        else
        {
            if_accelerate_led2 = FALSE;
            accelerate_counter_led2 = 0;
            volume_strength_value = 0;
            TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
        }

        if_refresh_led_data = 0xFF;
    }
}

void LedEffect6()
{
    if (led_exchange_flag)
    {
        led_exchange_flag = FALSE;
        ClearLedDataAll();

        color_start_index    = 0;
        color_contral_tmp    = 0;
        color_contral_tmp1   = 0;
        color_contral_param  = 0;
        color_contral_param1 = 0;
        color_contral_param2 = 0;

        if_accelerate        = FALSE;
        color_contral_param6 = 0;
        color_contral_param7 = 0;

        // ��ֵ���ֱ�����ʼ��
        peak_hold_value_led2 = 0;
        peak_hold_value_led3 = 0;
        peak_decay_timer = 0;

        // ���ּ��ٱ�����ʼ��
        if_accelerate_led2 = FALSE;
        accelerate_counter_led2 = 0;
        volume_strength_value = 0;

        TimeOutSet(&led_switch_timer, 0);
        TimeOutSet(&led_switch_timer1, 0);

        DBG("--------LedEffect4--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        ClearLedDataAll();

        // LED3 ʹ�ã���������
        uint8_t volume_level_led3 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MUSIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led3 = volume;
        }
        else
        {
            volume_level_led3 = 0;
        }

        // LED2 ʹ�ã���˷������������ֲ�һ����
        uint8_t volume_level_led2 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            if(volume < 0) volume = 0;
            volume_level_led2 = volume;
        }
        else
        {
            volume_level_led2 = 0;
        }

        // �̶����� 2 ����� ���� LED3 ʹ���Լ�����
        int16_t light_len = 2;
        int16_t end_pos = volume_level_led3;
        int16_t start_pos = end_pos - light_len;
        if(start_pos < 0) start_pos = 0;

        // LED2 ʹ���Լ����� + ƫ��
        int16_t end_pos_led2 = volume_level_led2 + 1;
        int16_t start_pos_led2 = (volume_level_led2 - light_len) + 1;
        if(end_pos_led2 > 48) end_pos_led2 = 48;
        if(start_pos_led2 < 0) start_pos_led2 = 0;

        // ========== ���� LED_DATA_3 ==========
        for(uint8_t i = start_pos; i < end_pos; i++)
        {
            uint8_t green_val = 255 * userVar.brightness / 100;
            LED_DATA_3[i * 3 + 0] = 0;
            LED_DATA_3[i * 3 + 1] = green_val;
            LED_DATA_3[i * 3 + 2] = 0;
        }

        // ========== ���� LED_DATA_2 ==========
        for(uint8_t i = start_pos_led2; i < end_pos_led2; i++)
        {
            uint8_t green_val = 255 * userVar.brightness / 100;
            LED_DATA_2[i * 3 + 0] = 0;
            LED_DATA_2[i * 3 + 1] = green_val;
            LED_DATA_2[i * 3 + 2] = 0;
        }

        // ====================== LED3 ��ֵ
        if(volume_level_led3 > peak_hold_value_led1_3)
        {
            peak_hold_value_led1_3 = volume_level_led3;
            peak_decay_timer_led1_3 = 2;
        }

        if(peak_decay_timer_led1_3 > 0)
        {
            peak_decay_timer_led1_3--;
            if(peak_decay_timer_led1_3 == 0 && peak_hold_value_led1_3 > 0)
            {
                peak_hold_value_led1_3--;
                peak_decay_timer_led1_3 = 2;
            }
        }

        // ======================LED2 ��ֵ ======================
        if(volume_level_led2 > peak_hold_value_led1_2)
        {
            peak_hold_value_led1_2 = volume_level_led2;
            peak_decay_timer_led1_2 = 3;
        }

        if(peak_decay_timer_led1_2 > 0)
        {
            peak_decay_timer_led1_2--;
            if(peak_decay_timer_led1_2 == 0 && peak_hold_value_led1_2 > 0)
            {
                peak_hold_value_led1_2--;
                peak_decay_timer_led1_2 = 3;
            }
        }

        // ========== ��ֵС�Ƶ���ʾ ==========
        if(peak_hold_value_led1_3 > 0 && peak_hold_value_led1_3 <= 48)
        {
            uint8_t peak_idx = peak_hold_value_led1_3 - 1;
            uint8_t yellow_val = 255 * userVar.brightness / 100;

            LED_DATA_3[peak_idx * 3 + 0] = yellow_val;
            LED_DATA_3[peak_idx * 3 + 1] = yellow_val;
            LED_DATA_3[peak_idx * 3 + 2] = 0;
        }

        if(peak_hold_value_led1_2 > 0 && peak_hold_value_led1_2 <= 48)
        {
            uint8_t peak_idx = peak_hold_value_led1_2 - 1;
            uint8_t yellow_val = 255 * userVar.brightness / 100;

            LED_DATA_2[peak_idx * 3 + 0] = yellow_val;
            LED_DATA_2[peak_idx * 3 + 1] = yellow_val;
            LED_DATA_2[peak_idx * 3 + 2] = 0;
        }

        // ========== ���ּ����߼� ==========
        if(userVar.if_music_play)
        {
            if(!if_accelerate_led2 && IsTimeOut(&led_switch_timer1))
            {
                volume_strength_value = GetLedSwitchTime1(2) / 2;

                if(volume_strength_value == 0)      accelerate_counter_led2 = 7;
                else if(volume_strength_value == 1) accelerate_counter_led2 = 9;
                else if(volume_strength_value == 2) accelerate_counter_led2 = 12;
                else if(volume_strength_value == 3) accelerate_counter_led2 = 17;
                else                                accelerate_counter_led2 = 35;

                if_accelerate_led2 = TRUE;
                TimeOutSet(&led_switch_timer1, ((70 - volume_strength_value * 20) * accelerate_counter_led2 + 500));
            }

            if(if_accelerate_led2)
            {
                if(accelerate_counter_led2 > 0)
                    accelerate_counter_led2--;
                else
                    if_accelerate_led2 = FALSE;

                uint8_t refresh_speed = 70 - volume_strength_value * 10;
                if(refresh_speed < LED_CONTRAL_TIME4)
                    refresh_speed = LED_CONTRAL_TIME4;

                TimeOutSet(&led_switch_timer, refresh_speed);
            }
            else
            {
                TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
            }
        }
        else
        {
            if_accelerate_led2 = FALSE;
            accelerate_counter_led2 = 0;
            volume_strength_value = 0;
            TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME4);
        }

        if_refresh_led_data = 0xFF;
    }
}
static const color_index_2[] =
{
	0,		255, 	0,
	165,	255,	0,
	255, 	255, 	0,
	255, 	0, 		0,
	255, 	0, 		255,
	0, 		0, 		255,
	128, 	0, 		255,
	255,	255,	255,
};

void LedEffect7()
{
	#define LedEffect_Color	40
	if (led_exchange_flag)
	{
		led_exchange_flag = FALSE;
		ClearLedDataAll();

		color_start_index	= 0;
		color_contral_tmp 	= 0;
		color_contral_tmp1 	= 0;
		color_contral_param = 0;
		color_contral_param1 = 0;
		color_contral_param2 = 0;
		if_accelerate 		= FALSE;
		color_contral_param6 = 0;
		color_contral_param7 = 0;

		TimeOutSet(&led_switch_timer, 0);
		TimeOutSet(&led_switch_timer1, 0);

		DBG("--------LedEffect3--------\n");
	}

	if (IsTimeOut(&led_switch_timer))
	{
		if_refresh_led_data = 0xFF;
		ClearLedDataAll();

		// ====================== LED3 ʹ�� �������� MUSIC ======================
		uint8_t volume_level_led3 = 0;
		if(userVar.if_music_play)
		{
			uint16_t vol = LedAudioLevelGet(MUSIC_VOL_TYPE);
			if(vol > 48) vol = 48;
			volume_level_led3 = vol;
		}

		// ====================== LED2 ʹ�� ��˷����� MIC����ͬ����======================
		uint8_t volume_level_led2 = 0;
		if(userVar.if_music_play)
		{
			uint16_t vol = LedAudioLevelGet(MIC_VOL_TYPE);
			if(vol > 48) vol = 48;
			volume_level_led2 = vol;
		}

		// ���� 24��25 �ŵ�
		uint8_t center_left  = 23;
		uint8_t center_right = 24;

		// LED3 ��ɢ��Χ�����֣�
		int16_t expand_led3 = (volume_level_led3 * 23) / 48;
		int16_t start_led3 = center_left - expand_led3;
		int16_t end_led3   = center_right + expand_led3;
		if(start_led3 < 0)  start_led3 = 0;
		if(end_led3 >= 48)  end_led3 = 47;

		// LED2 ��ɢ��Χ����˷磬������
		int16_t expand_led2 = (volume_level_led2 * 23) / 48;
		int16_t start_led2 = center_left - expand_led2;
		int16_t end_led2   = center_right + expand_led2;
		if(start_led2 < 0)  start_led2 = 0;
		if(end_led2 >= 48)  end_led2 = 47;

		// ��ɫ���䣨�Զ�������
		static uint8_t color_flow = 0;
		color_flow += 3;

		// ====================== �����ƴ� ======================
		for (int16_t i = 0; i < 48; i++)
		{
			uint8_t r = 0, g = 0, b = 0;

			// ====================== LED3 ʹ���Լ��ķ�Χ ======================
			if(i >= start_led3 && i <= end_led3)
			{
				uint8_t color = color_flow + i*5;
				if(color < 85)
				{
					r = 255 - color*3;
					g = color*3;
					b = 0;
				}
				else if(color < 170)
				{
					color -= 85;
					r = 0;
					g = 255 - color*3;
					b = color*3;
				}
				else
				{
					color -= 170;
					r = color*3;
					g = 0;
					b = 255 - color*3;
				}
				r = r * userVar.brightness / 100;
				g = g * userVar.brightness / 100;
				b = b * userVar.brightness / 100;
			}
			// д�� LED3
			LED_DATA_3[i*3+0] = g;
			LED_DATA_3[i*3+1] = r;
			LED_DATA_3[i*3+2] = b;

			// ====================== LED2 ʹ���Լ��ķ�Χ��������ͬ����======================
			r = 0; g = 0; b = 0;
			if(i >= start_led2 && i <= end_led2)
			{
				uint8_t color = color_flow + i*5;
				if(color < 85)
				{
					r = 255 - color*3;
					g = color*3;
					b = 0;
				}
				else if(color < 170)
				{
					color -= 85;
					r = 0;
					g = 255 - color*3;
					b = color*3;
				}
				else
				{
					color -= 170;
					r = color*3;
					g = 0;
					b = 255 - color*3;
				}
				r = r * userVar.brightness / 100;
				g = g * userVar.brightness / 100;
				b = b * userVar.brightness / 100;
			}
			// д�� LED2
			LED_DATA_2[i*3+0] = g;
			LED_DATA_2[i*3+1] = r;
			LED_DATA_2[i*3+2] = b;
		}

		TimeOutSet(&led_switch_timer, 12);
	}
}

// ��ֵ����������LED2��LED3 �ֿ�����ͬ����
static uint8_t peak_left_led2  = 23;
static uint8_t peak_right_led2 = 24;
static uint8_t peak_left_led3  = 23;
static uint8_t peak_right_led3 = 24;
static uint8_t peak_decay = 0;

static uint8_t peak_run_value_led9_3 = 0;
static uint8_t peak_run_value_led9_2 = 0;
static uint8_t peak_run_timer_led9_3 = 0;
static uint8_t peak_run_timer_led9_2 = 0;
void LedEffect8()
{
	#define LedEffect_Color	40
	if (led_exchange_flag)
	{
		led_exchange_flag = FALSE;
		ClearLedDataAll();

		color_start_index	= 0;
		color_contral_tmp 	= 0;
		color_contral_tmp1 	= 0;
		color_contral_param = 0;
		color_contral_param1 = 0;
		color_contral_param2 = 0;
		if_accelerate 		= FALSE;
		color_contral_param6 = 0;
		color_contral_param7 = 0;

		// ��ֵ��ʼ��
		peak_left_led2  = 23;
		peak_right_led2 = 24;
		peak_left_led3  = 23;
		peak_right_led3 = 24;
		peak_decay = 0;

		TimeOutSet(&led_switch_timer, 0);
		TimeOutSet(&led_switch_timer1, 0);

		DBG("--------LedEffect6--------\n");
	}

	if (IsTimeOut(&led_switch_timer))
	{
		if_refresh_led_data = 0xFF;
		ClearLedDataAll();

		// ====================== LED2 ʹ�ã��������� ======================
		uint8_t volume_level = 0;
		if(userVar.if_music_play)
		{
			uint16_t vol = LedAudioLevelGet(MUSIC_VOL_TYPE);
			if(vol > 48) vol = 48;
			volume_level = vol;
		}

		// ====================== LED3 ʹ�ã���˷�������������ͬ����======================
		uint8_t mic_volume_level = 0;
		if(userVar.if_music_play)
		{
			uint16_t mic_vol = LedAudioLevelGet(MIC_VOL_TYPE);
			if(mic_vol > 48) mic_vol = 48;
			mic_volume_level = mic_vol;
		}

		// ���� 24��25 �ŵ�
		uint8_t center_left  = 23;
		uint8_t center_right = 24;

		// LED2 ��ɢ��Χ�����֣�
		int16_t expand = (volume_level * 23) / 48;
		int16_t start = center_left - expand;
		int16_t end   = center_right + expand;
		if(start < 0)  start = 0;
		if(end >= 48)  end = 47;

		// LED3 ��ɢ��Χ����˷磩
		int16_t mic_expand = (mic_volume_level * 23) / 48;
		int16_t mic_start = center_left - mic_expand;
		int16_t mic_end   = center_right + mic_expand;
		if(mic_start < 0)  mic_start = 0;
		if(mic_end >= 48)  mic_end = 47;

		// LED2 ��ֵ����
		if(start < peak_left_led2)  peak_left_led2 = start;
		if(end   > peak_right_led2) peak_right_led2 = end;

		// LED3 ��ֵ����
		if(mic_start < peak_left_led3)  peak_left_led3 = mic_start;
		if(mic_end   > peak_right_led3) peak_right_led3 = mic_end;

		// ��ֵ˥��
		peak_decay++;
		if(peak_decay >= 8)
		{
			peak_decay = 0;
			if(peak_left_led2  < 23) peak_left_led2++;
			if(peak_right_led2 > 24) peak_right_led2--;
			if(peak_left_led3  < 23) peak_left_led3++;
			if(peak_right_led3 > 24) peak_right_led3--;
		}

		// ��ɫ����
		static uint8_t color_flow = 0;
		color_flow += 3;

		// ====================== LED2 �� LED3 �������� ======================
		for (int16_t i = 0; i < 48; i++)
		{
			// ---------------- LED2 �����ַ�Χ ----------------
			uint8_t r = 0, g = 0, b = 0;
			if(i >= start && i <= end)
			{
				uint8_t color = color_flow + i*5;
				if(color < 85)      { r = 255-color*3; g = color*3; b = 0; }
				else if(color <170){ color-=85; r=0; g=255-color*3; b=color*3; }
				else               { color-=170; r=color*3; g=0; b=255-color*3; }
				r = r*userVar.brightness/100;
				g = g*userVar.brightness/100;
				b = b*userVar.brightness/100;
			}
			LED_DATA_2[i*3+0] = g;
			LED_DATA_2[i*3+1] = r;
			LED_DATA_2[i*3+2] = b;

			// ---------------- LED3 ����˷緶Χ����ȫ��һ����----------------
			r = 0; g = 0; b = 0;
			if(i >= mic_start && i <= mic_end)
			{
				uint8_t color = color_flow + i*5;
				if(color < 85)      { r = 255-color*3; g = color*3; b = 0; }
				else if(color <170){ color-=85; r=0; g=255-color*3; b=color*3; }
				else               { color-=170; r=color*3; g=0; b=255-color*3; }
				r = r*userVar.brightness/100;
				g = g*userVar.brightness/100;
				b = b*userVar.brightness/100;
			}
			LED_DATA_3[i*3+0] = g;
			LED_DATA_3[i*3+1] = r;
			LED_DATA_3[i*3+2] = b;
		}

		// ��ֵ�׹�
		if(peak_left_led2 >=0 && peak_left_led2 <48)
		{
			LED_DATA_2[peak_left_led2*3 +0] = 0;
			LED_DATA_2[peak_left_led2*3 +1] = 255;
			LED_DATA_2[peak_left_led2*3 +2] = 0;
		}
		if(peak_right_led2 >=0 && peak_right_led2 <48)
		{
			LED_DATA_2[peak_right_led2*3 +0] = 0;
			LED_DATA_2[peak_right_led2*3 +1] = 255;
			LED_DATA_2[peak_right_led2*3 +2] = 0;
		}
		if(peak_left_led3 >=0 && peak_left_led3 <48)
		{
			LED_DATA_3[peak_left_led3*3 +0] = 0;
			LED_DATA_3[peak_left_led3*3 +1] = 255;
			LED_DATA_3[peak_left_led3*3 +2] = 0;
		}
		if(peak_right_led3 >=0 && peak_right_led3 <48)
		{
			LED_DATA_3[peak_right_led3*3 +0] = 0;
			LED_DATA_3[peak_right_led3*3 +1] = 255;
			LED_DATA_3[peak_right_led3*3 +2] = 0;
		}

		TimeOutSet(&led_switch_timer, 12);
	}
}

void LedEffect9()
{
    if (led_exchange_flag)
    {
        ClearLedDataAll();

        led_exchange_flag = FALSE;

        color_contral_param   = 0;
        color_contral_tmp     = 0;
        light_persent         = 0;
        color_start_index     = 0;

        peak_run_value_led9_3 = 0;
        peak_run_value_led9_2 = 0;
        peak_run_timer_led9_3 = 0;
        peak_run_timer_led9_2 = 0;

        TimeOutSet(&led_switch_timer, 0);
        TimeOutSet(&led_switch_timer1, 0);

        DBG("--------LedEffect9--------\n");
    }

    if (IsTimeOut(&led_switch_timer))
    {
        ClearLedDataAll();

        uint8_t volume_level_led3 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MUSIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            volume_level_led3 = volume;
        }

        uint8_t volume_level_led2 = 0;
        if(userVar.if_music_play)
        {
            uint16_t volume = LedAudioLevelGet(MIC_VOL_TYPE);
            if(volume > 48) volume = 48;
            volume_level_led2 = volume;
        }

        volume_level_led1 = volume_level_led3;

        for(uint8_t i = 0; i < volume_level_led3; i++)
        {
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            if(i <= 34)
            {
                g_val = 255 * userVar.brightness / 100;
            }
            else if(i <= 44)
            {
                r_val = 255 * userVar.brightness / 100;
                g_val = 255 * userVar.brightness / 100;
            }
            else
            {
                r_val = 255 * userVar.brightness / 100;
            }

            LED_DATA_3[i * 3 + 0] = r_val;
            LED_DATA_3[i * 3 + 1] = g_val;
            LED_DATA_3[i * 3 + 2] = b_val;
        }

        for(uint8_t i = 0; i < volume_level_led2; i++)
        {
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            if(i <= 34)
            {
                g_val = 255 * userVar.brightness / 100;
            }
            else if(i <= 44)
            {
                r_val = 255 * userVar.brightness / 100;
                g_val = 255 * userVar.brightness / 100;
            }
            else
            {
                r_val = 255 * userVar.brightness / 100;
            }

            LED_DATA_2[i * 3 + 0] = r_val;
            LED_DATA_2[i * 3 + 1] = g_val;
            LED_DATA_2[i * 3 + 2] = b_val;
        }

        if(volume_level_led3 > peak_run_value_led9_3)
        {
            peak_run_value_led9_3 = volume_level_led3;
            peak_run_timer_led9_3 = 2;
        }

        if(peak_run_timer_led9_3 > 0)
        {
            peak_run_timer_led9_3--;
            if(peak_run_timer_led9_3 == 0 && peak_run_value_led9_3 > 0)
            {
                if(peak_run_value_led9_3 < 48)
                {
                    peak_run_value_led9_3++;
                    peak_run_timer_led9_3 = 2;
                }
                else
                {
                    peak_run_value_led9_3 = 0;
                }
            }
        }

        if(volume_level_led2 > peak_run_value_led9_2)
        {
            peak_run_value_led9_2 = volume_level_led2;
            peak_run_timer_led9_2 = 2;
        }

        if(peak_run_timer_led9_2 > 0)
        {
            peak_run_timer_led9_2--;
            if(peak_run_timer_led9_2 == 0 && peak_run_value_led9_2 > 0)
            {
                if(peak_run_value_led9_2 < 48)
                {
                    peak_run_value_led9_2++;
                    peak_run_timer_led9_2 = 2;
                }
                else
                {
                    peak_run_value_led9_2 = 0;
                }
            }
        }

        if(peak_run_value_led9_3 > 0 && peak_run_value_led9_3 <= 48)
        {
            uint8_t peak_idx = peak_run_value_led9_3 - 1;
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            if(peak_idx <= 34)
            {
                g_val = 255 * userVar.brightness / 100;
            }
            else if(peak_idx <= 44)
            {
                r_val = 255 * userVar.brightness / 100;
                g_val = 255 * userVar.brightness / 100;
            }
            else
            {
                r_val = 255 * userVar.brightness / 100;
            }

            LED_DATA_3[peak_idx * 3 + 0] = r_val;
            LED_DATA_3[peak_idx * 3 + 1] = g_val;
            LED_DATA_3[peak_idx * 3 + 2] = b_val;
        }

        if(peak_run_value_led9_2 > 0 && peak_run_value_led9_2 <= 48)
        {
            uint8_t peak_idx = peak_run_value_led9_2 - 1;
            uint8_t r_val = 0, g_val = 0, b_val = 0;

            if(peak_idx <= 34)
            {
                g_val = 255 * userVar.brightness / 100;
            }
            else if(peak_idx <= 44)
            {
                r_val = 255 * userVar.brightness / 100;
                g_val = 255 * userVar.brightness / 100;
            }
            else
            {
                r_val = 255 * userVar.brightness / 100;
            }

            LED_DATA_2[peak_idx * 3 + 0] = r_val;
            LED_DATA_2[peak_idx * 3 + 1] = g_val;
            LED_DATA_2[peak_idx * 3 + 2] = b_val;
        }

        if(userVar.if_music_play)
        {
            if(!if_accelerate_led1 && IsTimeOut(&led_switch_timer1))
            {
                uint16_t volume_raw = GetAudioSdct(MUSIC_VOL_TYPE);
                uint8_t volume_strength = volume_raw / 1300;
                if(volume_strength > 4) volume_strength = 4;

                if(volume_strength == 0)
                    accelerate_counter_led1 = 7;
                else if(volume_strength == 1)
                    accelerate_counter_led1 = 9;
                else if(volume_strength == 2)
                    accelerate_counter_led1 = 12;
                else if(volume_strength == 3)
                    accelerate_counter_led1 = 17;
                else if(volume_strength == 4)
                    accelerate_counter_led1 = 35;

                if_accelerate_led1 = TRUE;
                TimeOutSet(&led_switch_timer1, ((70 - volume_strength * 20) * accelerate_counter_led1 + 500));
            }

            if(if_accelerate_led1)
            {
                if(accelerate_counter_led1 > 0)
                    accelerate_counter_led1--;
                else
                    if_accelerate_led1 = FALSE;

                TimeOutSet(&led_switch_timer, 70 - volume_level_led1 * 2);
            }
            else
            {
                TimeOutSet(&led_switch_timer, 80);
            }
        }
        else
        {
            if_accelerate_led1 = FALSE;
            accelerate_counter_led1 = 0;
            TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME8);
        }

        if_refresh_led_data = 0xFF;
    }
}
/*void LedEffect9()
{
	if (led_exchange_flag)
	{
		ClearLedDataAll();

		led_exchange_flag = FALSE;
		TimeOutSet(&led_switch_timer, 0);

		DBG("--------LedEffect9--------\n");
	}

	if (IsTimeOut(&led_switch_timer))
	{
		for (uint8_t i = 0; i < LED_NUM; i++)
		{
			LED_DATA_1[i * 3 + 0] = color_index_2[6 * 3 + 0];
			LED_DATA_1[i * 3 + 1] = color_index_2[6 * 3 + 1];
			LED_DATA_1[i * 3 + 2] = color_index_2[6 * 3 + 2];
		}

		if_refresh_led_data = 0xFF;

		TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME10);
	}
}*/

/*void LedEffect10()
{
	if (led_exchange_flag)
	{
		ClearLedDataAll();

		led_exchange_flag = FALSE;
		TimeOutSet(&led_switch_timer, 0);

		DBG("--------LedEffect10--------\n");
	}

	if (IsTimeOut(&led_switch_timer))
	{
		for (uint8_t i = 0; i < LED_NUM; i++)
		{
			LED_DATA_1[i * 3 + 0] = color_index_2[7 * 3 + 0];
			LED_DATA_1[i * 3 + 1] = color_index_2[7 * 3 + 1];
			LED_DATA_1[i * 3 + 2] = color_index_2[7 * 3 + 2];
		}

		if_refresh_led_data = 0xFF;

		TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME10);
	}
}*/

void LedEffectClearLed(void)
{
	if (led_exchange_flag)
	{
		ClearLedDataAll();
		led_exchange_flag = FALSE;
		
		TimeOutSet(&led_switch_timer, 0);
		
		DBG("--------LED_MODE_CLOSE--------\n");
	}
	if (IsTimeOut(&led_switch_timer))
	{
		ClearLedDataAll();
		
		if_refresh_led_data = 0xFF;

		TimeOutSet(&led_switch_timer, LED_CONTRAL_TIME9);
	}
}

void CheckRgbLedEffect(void)
{
    if(!userVar.LedInit || !IsTimeOut(&delay_led_on_timer) || userVar.ProgramSate == PROGRAM_POWERON_SLEEPING)
	{
		return;
	}

	#if defined(OTHER_LED_EFFECT)
	if (!IsTimeOut(&userVar.OtherLedTimer) && (userVar.OtherLedFlag) \
		&& (userVar.led_mode > LED_MODE_POWER_ON \
		&& userVar.led_mode < LED_MODE_CLEAR_LED))
	{
		switch(userVar.OtherLedFlag)
		{
		case MUSIC_VOL_TYPE:
			LedEffectMusicVolum(MusicVolume);
			break;
		
		case MIC_VOL_TYPE:
			LedEffectMusicVolum(MicVolume);
			break;

		case BT_CONNECTED_TYPE:
			LedEffectBtConnect(btManager.btLinkState);
			break;

		case LED_BRIGHTNESS_TYPE:
			LedEffectLedBrightness(userVar.brightness);
			break;
		}
			
		#ifdef PWM7_LED
		if(GetBit(if_refresh_led_data, LED_TIMER_7))
		{
			pwm7_led_ctrl(LED_DATA_1);
			CleanBit(if_refresh_led_data, LED_TIMER_7);
		}
		#endif

		#ifdef PWM6_LED
		if(GetBit(if_refresh_led_data, LED_TIMER_6))
		{
			pwm6_led_ctrl(LED_DATA_2);
			CleanBit(if_refresh_led_data, LED_TIMER_6);
		}
		#endif

		#ifdef PWM5_LED
		if(GetBit(if_refresh_led_data, LED_TIMER_5))
		{
			pwm5_led_ctrl(LED_DATA_3);
			CleanBit(if_refresh_led_data, LED_TIMER_5);
		}
		#endif

		#ifdef PWM8_LED
		if(GetBit(if_refresh_led_data, LED_TIMER_8))
		{
			pwm8_led_ctrl(LED_DATA_4);
			CleanBit(if_refresh_led_data, LED_TIMER_8);
		}
		#endif
		return;
	}
	#endif

	switch(userVar.led_mode)
	{
	//����
	case LED_MODE_POWER_ON:
		{
			LedEffectPowerOn();
		}	
		break;
	
	case LED_MODE_CHANGE1:
		{
			LedEffect1();
		}	
		break;
	
	case LED_MODE_CHANGE2:
		{
			LedEffect2();
		}	
		break;
	
	case LED_MODE_CHANGE3:
		{
			LedEffect3();
		}	
		break;

	case LED_MODE_CHANGE4:
		{
			LedEffect4();
		}	
		break;
	
	case LED_MODE_CHANGE5:
		{
			LedEffect5();
		}	
		break;
	
	case LED_MODE_CHANGE6:
		{
			LedEffect6();
		}	
		break;
	
	case LED_MODE_CHANGE7:
		{
			LedEffect7();
		}	
		break;
	
	case LED_MODE_CHANGE8:
		{
			LedEffect8();
		}	
		break;
	
	case LED_MODE_CHANGE9:
		{
			LedEffect9();
		}
		break;
	
	//case LED_MODE_CHANGE10:
	//	{
		//	LedEffect10();
		//}
		//break;
	
	case LED_MODE_CLEAR_LED:
		{
			LedEffectClearLed();
		}
		break;
	
	case LED_MODE_POWER_OFF:
		{
			LedEffectPowerOff();
		}
		break;
	}

#ifdef PWM7_LED
	if(GetBit(if_refresh_led_data, LED_TIMER_7))
	{
		pwm7_led_ctrl(LED_DATA_1);
		CleanBit(if_refresh_led_data, LED_TIMER_7);
	}
#endif

#ifdef PWM6_LED
	if(GetBit(if_refresh_led_data, LED_TIMER_6))
	{
		pwm6_led_ctrl(LED_DATA_2);
		CleanBit(if_refresh_led_data, LED_TIMER_6);
	}
#endif

#ifdef PWM5_LED
	if(GetBit(if_refresh_led_data, LED_TIMER_5))
	{
		pwm5_led_ctrl(LED_DATA_3);
		CleanBit(if_refresh_led_data, LED_TIMER_5);
	}
#endif

#ifdef PWM8_LED
	if(GetBit(if_refresh_led_data, LED_TIMER_8))
	{
		pwm8_led_ctrl(LED_DATA_4);
		CleanBit(if_refresh_led_data, LED_TIMER_8);
	}
#endif
}


void LedEffectInit(void)
{		
	ClearLedDataAll();
	pwm7_led_ctrl(LED_DATA_1);
	pwm6_led_ctrl(LED_DATA_2);
	pwm5_led_ctrl(LED_DATA_3);
	pwm8_led_ctrl(LED_DATA_4);

	led_param_init();
	
	userVar.LedInit = 1;
	TimeOutSet(&delay_led_on_timer, 10);
	TimeOutSet(&led_switch_timer, 5);
	TimeOutSet(&led_switch_timer1, 5);
	TimeOutSet(&led_switch_timer2, 5);
	TimeOutSet(&led_switch_timer3, 5);

	userVar.OtherLedFlag = NORMAL_TYPE;
	TimeOutSet(&userVar.OtherLedTimer, 0);

	led_exchange_flag 	= TRUE;
	led_exchange_flag1 	= TRUE;
	led_exchange_flag2 	= TRUE;

	userVar.brightness = 80;

	#if 1
	userVar.led_mode = LED_MODE_POWER_ON; 	
	#else
	userVar.led_mode = userVar.led_mode_bak; 	
	#endif
	
	ClearLedDataAll();
	pwm7_led_ctrl(LED_DATA_1);
	pwm6_led_ctrl(LED_DATA_2);
	pwm5_led_ctrl(LED_DATA_3);
	pwm8_led_ctrl(LED_DATA_4);

	vTaskDelay(10);
	ClearLedDataAll();
	pwm7_led_ctrl(LED_DATA_1);
	pwm6_led_ctrl(LED_DATA_2);
	pwm5_led_ctrl(LED_DATA_3);
	pwm8_led_ctrl(LED_DATA_4);
	
	DBG("---LedEffectInit--- %d - %d\n", userVar.led_mode, userVar.led_mode_bak);
}


void LedEffectIODeInit(void)	//GPIO����͵�ƽ
{		
	DBG("--------LedEffectIODeInit--------\n");
	
#ifdef PWM7_LED
	PWM_GpioConfig(TIMER7_PWM_A3_A5_A20_B4, PWM7_PIN_SEL, PWM_IO_MODE_NONE);
	STRING_CONNECT(GPIO_PORT, PWM7_LED, MODESET)(PWM7_LED_PIN, 0x0);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM7_LED, GPIE), 	PWM7_LED_PIN);				
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM7_LED, GPOE), 	PWM7_LED_PIN);
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM7_LED, GPPU), 	PWM7_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM7_LED, GPPD), 	PWM7_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM7_LED, GPOUT), 	PWM7_LED_PIN);
#endif	

#ifdef PWM6_LED
	PWM_GpioConfig(TIMER6_PWM_A1_A9_A10_A23_A24_A28, PWM6_PIN_SEL, PWM_IO_MODE_NONE);
	STRING_CONNECT(GPIO_PORT, PWM6_LED, MODESET)(PWM6_LED_PIN, 0x0);		
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM6_LED, GPIE), 	PWM6_LED_PIN);				
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM6_LED, GPOE), 	PWM6_LED_PIN);
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM6_LED, GPPU), 	PWM6_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM6_LED, GPPD), 	PWM6_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM6_LED, GPOUT), 	PWM6_LED_PIN);
#endif	

#ifdef PWM5_LED
	PWM_GpioConfig(TIMER5_PWM_A0_A7_A10_A22_A24 , PWM5_PIN_SEL, PWM_IO_MODE_NONE);
	STRING_CONNECT(GPIO_PORT, PWM5_LED, MODESET)(PWM5_LED_PIN, 0x0);		
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM5_LED, GPIE), 	PWM5_LED_PIN);				
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM5_LED, GPOE), 	PWM5_LED_PIN);
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM5_LED, GPPU), 	PWM5_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM5_LED, GPPD), 	PWM5_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM5_LED, GPOUT), 	PWM5_LED_PIN);
#endif

#ifdef PWM8_LED
	PWM_GpioConfig(TIMER8_PWM_A4_A6_A21_B5 , PWM8_PIN_SEL, PWM_IO_MODE_NONE);
	STRING_CONNECT(GPIO_PORT, PWM8_LED, MODESET)(PWM8_LED_PIN, 0x0);		
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM8_LED, GPIE), 	PWM8_LED_PIN);				
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM8_LED, GPOE), 	PWM8_LED_PIN);
	GPIO_RegOneBitSet(  STRING_CONNECT(GPIO, PWM8_LED, GPPU), 	PWM8_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM8_LED, GPPD), 	PWM8_LED_PIN);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, PWM8_LED, GPOUT), 	PWM8_LED_PIN);
#endif
}

void LedEffectOff(uint8_t tws)	//�л����ص�
{		
	if(userVar.led_mode == LED_MODE_POWER_OFF)
	{
		return;
	}
	
	//�ػ���Ч���ã������棬�ǹػ���Ч����
	if(userVar.led_mode > LED_MODE_POWER_ON && userVar.led_mode < LED_MODE_CLEAR_LED)
	{
		userVar.led_mode_bak = userVar.led_mode;
	}
	else if(userVar.led_mode_bak > LED_MODE_POWER_ON && userVar.led_mode_bak < LED_MODE_CLEAR_LED)
	{
		userVar.led_mode_bak = userVar.led_mode_bak;
	}
	else
	{
		userVar.led_mode_bak = 1;//LED_MODE_CLEAR_LED
	}
	
	userVar.led_mode = LED_MODE_CLEAR_LED;
	
	led_exchange_flag = TRUE;
	led_exchange_flag1 = TRUE;
	led_exchange_flag2 = TRUE;
	led_exchange_flag3 = TRUE;
	
	DBG("--------LedEffectOff--------\n");
	
	#ifdef BT_TWS_SUPPORT
	if(tws)
	{
		tws_rgb_led_send(userVar.led_mode, FALSE);
	}
	#endif

	ClearLedDataAll();
	pwm7_led_ctrl(LED_DATA_1);
	pwm6_led_ctrl(LED_DATA_2);
	pwm5_led_ctrl(LED_DATA_3);
	pwm8_led_ctrl(LED_DATA_4);

	vTaskDelay(20);
	ClearLedDataAll();
	pwm7_led_ctrl(LED_DATA_1);
	pwm6_led_ctrl(LED_DATA_2);
	pwm5_led_ctrl(LED_DATA_3);
	pwm8_led_ctrl(LED_DATA_4);

	vTaskDelay(20);
	ClearLedDataAll();
	pwm7_led_ctrl(LED_DATA_1);
	pwm6_led_ctrl(LED_DATA_2);
	pwm5_led_ctrl(LED_DATA_3);
	pwm8_led_ctrl(LED_DATA_4);

}

void LedEffectNext(void)	//��Ч�л�
{
	//���ػ���Ч�У��ػ��У��������л�
	if(userVar.led_mode == LED_MODE_POWER_OFF || userVar.led_mode == LED_MODE_POWER_ON || !IsTimeOut(&userVar.OtherLedTimer))
	{
		return;
	}

	//��һ����Ч�Ƿ�����Ч��Ч
	if((userVar.led_mode > LED_MODE_POWER_ON) && (userVar.led_mode < LED_MODE_CLEAR_LED))
	{
		userVar.led_mode_bak = userVar.led_mode;
	}
	else if(userVar.led_mode_bak > LED_MODE_POWER_ON && userVar.led_mode_bak < LED_MODE_CLEAR_LED)
	{
		userVar.led_mode_bak = userVar.led_mode_bak;
	}
	else
	{	//��һ����Ч����Ч��Ч��Ĭ�ϱ����һ����Ч
		userVar.led_mode_bak = 1;//userVar.led_mode;
	}

	//Ĭ������һ����Ч
	++userVar.led_mode;

	//�л����Ƿ�Ϊ��Ч��Ч
	if((userVar.led_mode <= LED_MODE_POWER_ON) ||  (userVar.led_mode > LED_MODE_CLEAR_LED))
	{
		userVar.led_mode = LED_MODE_CHANGE1;
	}
	
	APP_DBG("LedEffectNext %d -- %d\n", userVar.led_mode, userVar.led_mode_bak);
	
	#ifdef BT_TWS_SUPPORT
	tws_rgb_led_send(userVar.led_mode, TRUE);
	#endif
	
	#ifdef CFG_FUNC_DISPLAY_EN
	DisplayTaskMsgSend(MSG_DISPLAY_SERVICE_LED_MODE);
	#endif

	//led_param_init();

	//��Ч��ʼ����־
	led_exchange_flag 	= TRUE;
	led_exchange_flag1 	= TRUE;
	led_exchange_flag2 	= TRUE;
	led_exchange_flag3 	= TRUE;

	#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
	#endif
}

void LedEffectSwitch(LED_MODE led_mode, uint8_t display, uint8_t tws)//ѡ���л�
{
	//���ػ���Ч�У��ػ��У��������л�
	if(userVar.led_mode == LED_MODE_POWER_OFF)
	{
		return;
	}
	else if((led_mode == LED_MODE_CLEAR_LED) && (userVar.led_mode == LED_MODE_CLEAR_LED))
	{
		led_mode = userVar.led_mode_bak;
	}

	//��һ����Ч�Ƿ�����Ч��Ч
	if((userVar.led_mode > LED_MODE_POWER_ON) && (userVar.led_mode < LED_MODE_CLEAR_LED))
	{
		userVar.led_mode_bak = userVar.led_mode;
	}
	else if(userVar.led_mode_bak > LED_MODE_POWER_ON && userVar.led_mode_bak < LED_MODE_CLEAR_LED)
	{
		userVar.led_mode_bak = userVar.led_mode_bak;
	}
	else
	{	//��һ����Ч����Ч��Ч��Ĭ�ϱ����һ����Ч
		userVar.led_mode_bak = 1;//userVar.led_mode;
	}
	
	//�л��ĵ�Ч�Ƿ�Ϊ��Ч��Ч
	if((led_mode > LED_MODE_POWER_ON) && (led_mode <= LED_MODE_CLEAR_LED))
	{
		userVar.led_mode = led_mode;
	}
	else if(led_mode == LED_MODE_POWER_OFF)
	{	//�л����ػ���Ч
		userVar.led_mode = LED_MODE_POWER_OFF;
	}
	else if(led_mode == LED_MODE_POWER_ON && GetSystemMode() == ModeIdle)
	{	//�л����ػ���Ч
		userVar.led_mode = LED_MODE_POWER_ON;
	}
	else
	{	//�л�����Ч��Ч��Ĭ���л�����һ����Ч
		userVar.led_mode = 1;//LED_MODE_CHANGE1
	}
	
	APP_DBG("LedEffectSwitch %d - %d - %d\n", led_mode, userVar.led_mode, userVar.led_mode_bak);
	
	#ifdef CFG_FUNC_DISPLAY_EN
	if(display)
	{
		DisplayTaskMsgSend(MSG_DISPLAY_SERVICE_LED_MODE);
	}
	#endif
	
	#ifdef BT_TWS_SUPPORT
	if(tws)
	{
		tws_rgb_led_send(userVar.led_mode, display);
	}
	#endif

	led_param_init();
	
	//��Ч��ʼ����־
	led_exchange_flag 	= TRUE;
	led_exchange_flag1 	= TRUE;
	led_exchange_flag2 	= TRUE;
	led_exchange_flag3	= TRUE;

	#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
	#endif
}

void LedEffectSwitchOther(LED_TYPE led_mode)	//�л���ĳЩָʾ��
{
	if(led_mode <= 0) return;

	userVar.OtherLedFlag = led_mode;
	if(userVar.OtherLedFlag == MUSIC_VOL_TYPE)
		TimeOutSet(&userVar.OtherLedTimer, 2000);		//ʱ��Ϊ��Чʱ��
	else if(userVar.OtherLedFlag == BT_CONNECTED_TYPE)
		TimeOutSet(&userVar.OtherLedTimer, 6000);
	else if(userVar.OtherLedFlag == LED_BRIGHTNESS_TYPE)
		TimeOutSet(&userVar.OtherLedTimer, 2000);

	led_exchange_flag_other = TRUE;
	DBG("LedEffectSwitchOther: %d\n", userVar.OtherLedFlag);
}

/**
 * @brief     ������ת��
 *
 * @param[in] *src  Դ���ݴ����
 * @param[in] *dst  ת��������ݴ����
 * @param[in] num   �������
 *
 * @return    ��
 */
static void Data_Conversion(uint8_t *src,uint8_t *dst,uint16_t num)
{
	uint16_t i=0;
	uint8_t j=0;
	uint16_t len;
	uint8_t *temp;
	temp = dst+48;
	len=3*num;
	for(i=0;i<len;i++)
	{
		for(j=0;j<8;j++)
		{
			if((*(src+i)<<j)&0x80)
			{
				*(temp + 8*i+j) =HT;
			}
			else
			{
				*(temp + 8*i+j) =LT;
			}
		}
	}
}

/**
 * @brief     ��Ҫ��TIMER���PWM��ÿһ��PWM������ɺ�ᴥ��DMA����
 * 			     ͨ��DMA����PWM��ռ�ձ�
 */
static void pwm7_led_Init(void)
{
#ifdef PWM7_LED
	//TIMER7_PWM_A3_A5_A20_B4
	#if (PWM7_PIN_SEL == 0)
	printf("[pwm_led_Init]	TIM7_A3\n");
	#elif  (PWM7_PIN_SEL == 1)
	printf("[pwm_led_Init]	TIM7_A5\n");
	#elif (PWM7_PIN_SEL == 2)
	printf("[pwm_led_Init]	TIM7_A20\n");
	#elif (PWM7_PIN_SEL == 3)
	printf("[pwm_led_Init]	TIM7_B4\n");
	#endif
	
	/**********************DMA����**************************/
	DMA_CONFIG	  DMAParam;
	DMAParam.Dir   				= DMA_CHANNEL_DIR_MEM2PERI;
	DMAParam.Mode  				= DMA_BLOCK_MODE;//DMA_BLOCK_MODE;
	DMAParam.ThresholdLen 		= 0 ;
	DMAParam.SrcAddress 		= (uint32_t)LedBufA;
	DMAParam.SrcAddrIncremental = DMA_SRC_AINCR_SRC_WIDTH;

	DMAParam.DstAddress 		= 0x4002E024;// ռ�ձȼĴ��� 0x4002E024
	DMAParam.DataWidth 			= DMA_DWIDTH_BYTE;
	DMAParam.DstAddrIncremental = DMA_DST_AINCR_NO;
	DMAParam.BufferLen			= BUF_LEN;
	
	DMA_TimerConfig(PERIPHERAL_ID_TIMER7, &DMAParam);
	DMA_BlockBufSet(PERIPHERAL_ID_TIMER7, LedBufA, BUF_LEN);	
	DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER7, DMA_DONE_INT);//���DMA��������ж�
	DMA_ChannelEnable(PERIPHERAL_ID_TIMER7);
	
/**********************PWM����**************************/
	
	PWM_StructInit	PWMParam;
	PWMParam.CounterMode			= PWM_COUNTER_MODE_UP;
	PWMParam.OutputType 			= PWM_OUTPUT_SINGLE_1;
	PWMParam.DMAReqEnable			= PWM_REQ_DMA_MODE;
	PWMParam.FreqDiv				= LED_T; //120MϵͳƵ���� 12000 = 100us
	PWMParam.Duty					= 0;
	//GPIO Config
	PWM_GpioConfig(TIMER7_PWM_A3_A5_A20_B4, PWM7_PIN_SEL, PWM_IO_MODE_OUT);
	//PWM Config
	PWM_Config(TIMER7, &PWMParam);
	//PWM Start
	PWM_Enable(TIMER7);
#endif
}

static void pwm6_led_Init(void)
{
#ifdef PWM6_LED
	//TIMER6_PWM_A1_A9_A10_A23_A24_A28
	#if (PWM6_PIN_SEL == 0)
	printf("[pwm_led_Init]	TIM6_A1\n");
	#elif  (PWM6_PIN_SEL == 1)
	printf("[pwm_led_Init]	TIM6_A9\n");
	#elif (PWM6_PIN_SEL == 2)
	printf("[pwm_led_Init]	TIM6_A10\n");
	#elif (PWM6_PIN_SEL == 3)
	printf("[pwm_led_Init]	TIM6_A23\n");
	#elif  (PWM6_PIN_SEL == 4)
	printf("[pwm_led_Init]	TIM6_A24\n");
	#elif (PWM6_PIN_SEL == 5)
	printf("[pwm_led_Init]	TIM6_A28\n");
	#endif

	/**********************DMA����**************************/
	DMA_CONFIG	  DMAParam;

	DMAParam.Dir   				= DMA_CHANNEL_DIR_MEM2PERI;
	DMAParam.Mode  				= DMA_BLOCK_MODE;//DMA_BLOCK_MODE;
	DMAParam.ThresholdLen 		= 0;
	DMAParam.SrcAddress 		= (uint32_t)LedBufC;
	DMAParam.SrcAddrIncremental = DMA_SRC_AINCR_SRC_WIDTH;

	DMAParam.DstAddress 		= 0x4002C824;// ռ�ձȼĴ���
	DMAParam.DataWidth 			= DMA_DWIDTH_BYTE;
	DMAParam.DstAddrIncremental = DMA_DST_AINCR_NO;
	DMAParam.BufferLen			= BUF_LEN1;

	DMA_TimerConfig(PERIPHERAL_ID_TIMER6, &DMAParam);
	DMA_BlockBufSet(PERIPHERAL_ID_TIMER6, LedBufC, BUF_LEN1); 
	DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER6, DMA_DONE_INT);//���DMA��������ж�
	DMA_ChannelEnable(PERIPHERAL_ID_TIMER6);

/**********************PWM����**************************/
	PWM_StructInit	PWMParam;

	PWMParam.CounterMode			= PWM_COUNTER_MODE_UP;
	PWMParam.OutputType 			= PWM_OUTPUT_SINGLE_1;
	PWMParam.DMAReqEnable			= PWM_REQ_DMA_MODE;
	PWMParam.FreqDiv				= LED_T; //120MϵͳƵ���� 12000 = 1.25us
	PWMParam.Duty					= 0;
	//GPIO Config
	PWM_GpioConfig(TIMER6_PWM_A1_A9_A10_A23_A24_A28, PWM6_PIN_SEL, PWM_IO_MODE_OUT);
	//PWM Config
	PWM_Config(TIMER6, &PWMParam);
	//PWM Start
	PWM_Enable(TIMER6);
#endif
}

static void pwm5_led_Init(void)
{
#ifdef PWM5_LED
	//TIMER5_PWM_A0_A7_A10_A22_A24
	#if (PWM5_PIN_SEL == 0)
	printf("[pwm_led_Init]	TIM5_A0\n");
	#elif  (PWM5_PIN_SEL == 1)
	printf("[pwm_led_Init]	TIM5_A7\n");
	#elif (PWM5_PIN_SEL == 2)
	printf("[pwm_led_Init]	TIM5_A10\n");
	#elif (PWM5_PIN_SEL == 3)
	printf("[pwm_led_Init]	TIM5_A22\n");
	#elif (PWM5_PIN_SEL == 4)
	printf("[pwm_led_Init]	TIM5_A24\n");
	#endif
	
	/**********************DMA����**************************/
	DMA_CONFIG	  DMAParam;
	DMAParam.Dir   				= DMA_CHANNEL_DIR_MEM2PERI;
	DMAParam.Mode  				= DMA_BLOCK_MODE;//DMA_BLOCK_MODE;
	DMAParam.ThresholdLen 		= 0 ;
	DMAParam.SrcAddress 		= (uint32_t)LedBufE;
	DMAParam.SrcAddrIncremental = DMA_SRC_AINCR_SRC_WIDTH;

	DMAParam.DstAddress 		= 0x4002C024;// ռ�ձȼĴ���
	DMAParam.DataWidth 			= DMA_DWIDTH_BYTE;
	DMAParam.DstAddrIncremental = DMA_DST_AINCR_NO;
	DMAParam.BufferLen			= BUF_LEN2;

	DMA_TimerConfig(PERIPHERAL_ID_TIMER5, &DMAParam);
	DMA_BlockBufSet(PERIPHERAL_ID_TIMER5, LedBufE, BUF_LEN2); 
	DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER5, DMA_DONE_INT);//���DMA��������ж�
	DMA_ChannelEnable(PERIPHERAL_ID_TIMER5);

/**********************PWM����**************************/
	PWM_StructInit	PWMParam;

	PWMParam.CounterMode			= PWM_COUNTER_MODE_UP;
	PWMParam.OutputType 			= PWM_OUTPUT_SINGLE_1;
	PWMParam.DMAReqEnable			= PWM_REQ_DMA_MODE;
	PWMParam.FreqDiv				= LED_T; //120MϵͳƵ���� 12000 = 100us
	PWMParam.Duty					= 0;
	//GPIO Config
	PWM_GpioConfig(TIMER5_PWM_A0_A7_A10_A22_A24, PWM5_PIN_SEL, PWM_IO_MODE_OUT);
	//PWM Config
	PWM_Config(TIMER5, &PWMParam);
	//PWM Start
	PWM_Enable(TIMER5);
#endif
}

static void pwm8_led_Init(void)
{
#ifdef PWM8_LED
	//TIMER8_PWM_A4_A6_A21_B5
	#if (PWM8_PIN_SEL == 0)
	printf("[pwm_led_Init]	TIM8_A4\n");
	#elif  (PWM8_PIN_SEL == 1)
	printf("[pwm_led_Init]	TIM8_A6\n");
	#elif (PWM8_PIN_SEL == 2)
	printf("[pwm_led_Init]	TIM8_A21\n");
	#elif (PWM8_PIN_SEL == 3)
	printf("[pwm_led_Init]	TIM8_B5\n");
	#endif
	
	/**********************DMA����**************************/
	DMA_CONFIG	  DMAParam;
	DMAParam.Dir   				= DMA_CHANNEL_DIR_MEM2PERI;
	DMAParam.Mode  				= DMA_BLOCK_MODE;//DMA_BLOCK_MODE;
	DMAParam.ThresholdLen 		= 0 ;
	DMAParam.SrcAddress 		= (uint32_t)LedBufG;
	DMAParam.SrcAddrIncremental = DMA_SRC_AINCR_SRC_WIDTH;

	DMAParam.DstAddress 		= 0x4002E824;// ռ�ձȼĴ���
	DMAParam.DataWidth 			= DMA_DWIDTH_BYTE;
	DMAParam.DstAddrIncremental = DMA_DST_AINCR_NO;
	DMAParam.BufferLen			= BUF_LEN3;

	DMA_TimerConfig(PERIPHERAL_ID_TIMER8, &DMAParam);
	DMA_BlockBufSet(PERIPHERAL_ID_TIMER8, LedBufG, BUF_LEN3); 
	DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER8, DMA_DONE_INT);//���DMA��������ж�
	DMA_ChannelEnable(PERIPHERAL_ID_TIMER8);

/**********************PWM����**************************/
	PWM_StructInit	PWMParam;

	PWMParam.CounterMode			= PWM_COUNTER_MODE_UP;
	PWMParam.OutputType 			= PWM_OUTPUT_SINGLE_1;
	PWMParam.DMAReqEnable			= PWM_REQ_DMA_MODE;
	PWMParam.FreqDiv				= LED_T; //120MϵͳƵ���� 12000 = 100us
	PWMParam.Duty					= 0;
	//GPIO Config
	PWM_GpioConfig(TIMER8_PWM_A4_A6_A21_B5, PWM8_PIN_SEL, PWM_IO_MODE_OUT);
	//PWM Config
	PWM_Config(TIMER8, &PWMParam);
	//PWM Start
	PWM_Enable(TIMER8);
#endif
}

void pwm_led_Init(void)
{
	pwm7_led_Init();
	pwm6_led_Init();
	pwm5_led_Init();
	pwm8_led_Init();

	LedEffectInit();
}

static void pwm7_led_ctrl(uint8_t *data_led)
{
#ifdef PWM7_LED
	static uint8_t send_flag=0; //����ˢ������״̬
	static uint8_t state_flag=0;//����׼��״̬
	uint16_t i=0,j=0;

	if(state_flag==0)//׼����һ����ʾ����
	{
				
		if(send_flag)
		{
			Data_Conversion(data_led,LedBufA,LED_NUM);
		}
		else
		{
			Data_Conversion(data_led,LedBufB,LED_NUM);
		}
		state_flag=1;
	}

	if(DMA_InterruptFlagGet(PERIPHERAL_ID_TIMER7, DMA_DONE_INT))
	{
		if(state_flag)
		{
			DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER7, DMA_DONE_INT);

			if(send_flag)
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER7,LedBufA,BUF_LEN);
				send_flag = 0;
			}
			else
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER7,LedBufB,BUF_LEN);
				send_flag = 1;
			}
			DMA_ChannelEnable(PERIPHERAL_ID_TIMER7);
			state_flag=0;
		}
	}
#endif
}

static void pwm6_led_ctrl(uint8_t *data_led)
{
#ifdef PWM6_LED
	static uint8_t send_flag=0; //����ˢ������״̬
	static uint8_t state_flag=0;//����׼��״̬
	uint16_t i=0,j=0;

	if(state_flag==0)//׼����һ����ʾ����
	{
				
		if(send_flag)
		{
			Data_Conversion(data_led,LedBufC,LED_NUM1);
		}
		else
		{
			Data_Conversion(data_led,LedBufD,LED_NUM1);
		}
		state_flag=1;
	}

	if(DMA_InterruptFlagGet(PERIPHERAL_ID_TIMER6, DMA_DONE_INT))
	{
		if(state_flag)
		{
			DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER6, DMA_DONE_INT);

			if(send_flag)
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER6,LedBufC,BUF_LEN1);
				send_flag = 0;
			}
			else
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER6,LedBufD,BUF_LEN1);
				send_flag = 1;
			}
			DMA_ChannelEnable(PERIPHERAL_ID_TIMER6);
			state_flag=0;
		}
	}
#endif
}

static void pwm5_led_ctrl(uint8_t *data_led)
{
#ifdef PWM5_LED
	static uint8_t send_flag=0; //����ˢ������״̬
	static uint8_t state_flag=0;//����׼��״̬
	uint16_t i=0,j=0;

	if(state_flag==0)//׼����һ����ʾ����
	{
				
		if(send_flag)
		{
			Data_Conversion(data_led,LedBufE,LED_NUM2);
		}
		else
		{
			Data_Conversion(data_led,LedBufF,LED_NUM2);
		}
		state_flag=1;
	}

	if(DMA_InterruptFlagGet(PERIPHERAL_ID_TIMER5, DMA_DONE_INT))
	{
		if(state_flag)
		{
			DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER5, DMA_DONE_INT);

			if(send_flag)
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER5,LedBufE,BUF_LEN2);
				send_flag = 0;
			}
			else
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER5,LedBufF,BUF_LEN2);
				send_flag = 1;
			}
			DMA_ChannelEnable(PERIPHERAL_ID_TIMER5);
			state_flag=0;
		}
	}
#endif
}

static void pwm8_led_ctrl(uint8_t *data_led)
{
#ifdef PWM8_LED
	static uint8_t send_flag=0; //����ˢ������״̬
	static uint8_t state_flag=0;//����׼��״̬
	uint16_t i=0,j=0;

	if(state_flag==0)//׼����һ����ʾ����
	{
				
		if(send_flag)
		{
			Data_Conversion(data_led,LedBufG,LED_NUM3);
		}
		else
		{
			Data_Conversion(data_led,LedBufH,LED_NUM3);
		}
		state_flag=1;
	}

	if(DMA_InterruptFlagGet(PERIPHERAL_ID_TIMER8, DMA_DONE_INT))
	{
		if(state_flag)
		{
			DMA_InterruptFlagClear(PERIPHERAL_ID_TIMER8, DMA_DONE_INT);

			if(send_flag)
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER8,LedBufG,BUF_LEN3);
				send_flag = 0;
			}
			else
			{
				DMA_BlockBufSet(PERIPHERAL_ID_TIMER8,LedBufH,BUF_LEN3);
				send_flag = 1;
			}
			DMA_ChannelEnable(PERIPHERAL_ID_TIMER8);
			state_flag=0;
		}
	}
#endif
}

#endif

