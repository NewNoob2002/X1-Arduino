#include "MP2762A.h"

//定义IC寄存器
#define MP2762A_REG_IC_CTL_1                  0x00  //IC控制寄1存器 默认1.5A, 最大6.35A
#define MP2762A_REG_IV_CTL                    0x01  //电流电压控制寄存器 默认4.5V, 0-25.5V
#define MP2762A_REG_CHARGE_Ic                 0x02  //充电电流控制寄存器 默认1A, 0-6A
#define MP2762A_REG_PRE_CHARGE_Ic             0x03  //预充电电流控制寄存器 预充电默认180mA，终止电流默认200mA
#define MP2762A_REG_BAT_FULL_CHARGE_V         0x04  //电池充满电压及自动重充阈值设置（默认8.4V，重充阈值200mV
#define MP2762A_REG_BAT_IC_TEMP               0x05  //电池阻抗补偿和结温调节,默认120°C
#define MP2762A_REG_OTG_Out_V                 0x06  //OTG输出电压设置,默认4.75V
#define MP2762A_REG_OTG_Out_I                 0x07  //预充电阈值和OTG输出电流限制（默认6.8V预充阈值，OTG电流1A）

//配置寄存器
#define MP2762A_REG_CTL_1                  0x08  //配置寄存器0（控制OTG/充电使能、BATTFET开关等）
#define MP2762A_REG_CTL_2                  0x09  //配置寄存器1 (安全计时器、看门狗定时器设置）
#define MP2762A_REG_CTL_3                  0x0A  //配置寄存器2（NTC温度保护类型和阈值）
#define MP2762A_REG_CTL_4                  0x0B  //配置寄存器3（切换频率、PROCHOT/PSYS功能使能）
#define MP2762A_REG_CTL_5                  0x0C  //配置寄存器4（PROCHOT触发条件、虚拟二极管模式)

#define MP2762A_REG_SYS_UV_OV                 0x0D  //系统/OTG欠压和过压保护阈值设置
#define MP2762A_REG_PROCHOT_CFG               0x0E  //电池欠压和过压保护阈值设置
#define MP2762A_REG_IC_CTL_2                  0x0F  //输入电流限制2设置（用于双级限流模式）
#define MP2762A_REG_IC_CTL_2_Time             0x10  //输入电流限制2持续时间设置

#define MP2762A_REG_STAT                      0x13  //状态寄存器（电池UVLO、系统UV、充电状态、输入电源状态等）
#define MP2762A_REG_FAULT                     0x14  //故障寄存器（看门狗超时、OTG故障、充电故障、NTC故障等）
//10位分辨率0-1023
#define MP2762A_REG_BAT_Volt_H                0x16  //电池电压高字节
#define MP2762A_REG_BAT_Volt_L                0x17  //电池电压低字节
#define MP2762A_REG_SYS_Volt_H                0x18  //系统电流高字节
#define MP2762A_REG_SYS_Volt_L                0x19  //系统电流低字节
#define MP2762A_REG_BAT_Charge_IC_H           0x1A  //电池充电电流高字节
#define MP2762A_REG_BAT_Charge_IC_L           0x1B  //电池充电电流低字节
#define MP2762A_REG_In_Volt_H                 0x1C  //输入电压高字节
#define MP2762A_REG_In_Volt_L                 0x1D  //输入电压低字节
#define MP2762A_REG_In_IA_H                   0x1E  //输入电流高字节
#define MP2762A_REG_In_IA_L                   0x1F  //输入电流低字节
#define MP2762A_REG_OTG_Volt_H                0x20  //OTG输出电压高字节
#define MP2762A_REG_OTG_Volt_L                0x21  //OTG输出电压低字节
#define MP2762A_REG_OTG_IC_H                  0x22  //OTG输出电流高字节
#define MP2762A_REG_OTG_IC_L                  0x23  //OTG输出电流低字节

#define MP2762A_REG_Jun_Temp_H                0x24  //芯片结温温度高字节
#define MP2762A_REG_Jun_Temp_L                0x25  //芯片结温温度低字节

#define MP2762A_REG_SYS_Power_H               0x26  //系统功率高字节
#define MP2762A_REG_SYS_Power_L               0x27  //系统功率低字节
#define MP2762A_REG_BAT_DisCharge_H           0x28  //电池放电电流高字节
#define MP2762A_REG_BAT_DisCharge_L           0x29  //电池放电电流低字节

#define MP2762A_REG_BAT_OVP_DEBOUNCE          0x2B  //电池过压保护去抖时间设置
#define MP2762A_REG_BAT_VOLTAGE_LOOP_EN       0x2D  //电池电压环路使能控制
#define MP2762A_REG_BAT_PRECHARGE_THRESHOLD   0x30  //电池预充阈值选项（选择不同预充电压范围）
#define MP2762A_REG_SYS_VOLTAGE_JUMP_THRESHOLD 0x31 //系统电压跳脉冲阈值设置
#define MP2762A_REG_HIZ_MODE_INT_MASK         0x33  //Hi-Z模式中断屏蔽控制
#define MP2762A_REG_ANALOG_FREQ_LOOP_EN       0x36  //模拟频率环路使能控制
#define MP2762A_REG_ADC_NTC_VOLTAGE_RESULT_H  0x40  //ADC NTC电压结果高字节
#define MP2762A_REG_ADC_NTC_VOLTAGE_RESULT_L  0x41  //ADC NTC电压结果低字节
#define MP2762A_REG_HIZ_MODE_STATUS           0x48  //Hi-Z模式状态指示


//MP2762A配置

int MP2762A_Configure(MP2762A_config_t config)
{
    ESP_LOGI("MP2762A", "MP2762A Start Configuring");

    esp_err_t ret;
    uint8_t cfg_val = 0;
    //检测I2c通讯
    ret = esp32.Read_BAT_Reg_Byte(MP2762A_REG_STAT, &cfg_val, 1);
    if(ret != ESP_OK)
    {
        ESP_LOGE("MP2762A", "MP2762A I2C Communication Error");
        return -1;
    }

    if(cfg_val != 0b00000000)
    {
        ESP_LOGE("MP2762A", "MP2762A I2C Communication Error");
        return -1;
    }

    //Start Configure
    //输入电流电压限制
    //输入电压档位	推荐工作模式	输入功率	适配器能力	优先级
    /*5V-3A	        Boost模式	    15W	        低功率	备用
    9V-2.22A	    Buck模式	    20W	        中功率	首选
    12V-1.67A	    Buck模式	    20W	        中功率	首选 */
    //2.22A ≈ 1600mA + 400mA ；
    uint8_t ic_ctl_1 = 0b00101110;
    ret = esp32.Write_BAT_Reg_Byte(MP2762A_REG_IC_CTL_1, ic_ctl_1);
    if(ret != ESP_OK)
    {
        ESP_LOGE("MP2762A", "ICC Configure Error");
        return -1;
    }
    //8.8V = 8800mV = 0b01011000
    uint8_t iv_ctl = 0b01011000;
    ret = esp32.Write_BAT_Reg_Byte(MP2762A_REG_IV_CTL, iv_ctl);
    if(ret != ESP_OK)
    {
        ESP_LOGE("MP2762A", "IVC Configure Error");
        return -1;
    }
    //2A = 1600mA + 400mA = 0b00011000
    uint8_t charge_ic = 0b00101000;
    ret = esp32.Write_BAT_Reg_Byte(MP2762A_REG_CHARGE_Ic, charge_ic);
    if(ret != ESP_OK)
    {
        ESP_LOGE("MP2762A", "Charge IC Configure Error");
        return -1;
    }
    //
    uint8_t pre_charge_ic = 0b00000000;
    ret = esp32.Write_BAT_Reg_Byte(MP2762A_REG_PRE_CHARGE_Ic, pre_charge_ic);
    if(ret != ESP_OK)
    {
        ESP_LOGE("MP2762A", "Pre Charge IC Configure Error");
        return -1;
    }
    return 0;
}
