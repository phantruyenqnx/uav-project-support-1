#include <src/SensLevel/SensLevel.hpp>
#include <BLDCSpinner.hpp>
#include <SERVOHopper.hpp>
#include "src/define.hpp"

//Khai bao dung cac class
UARTSensLevelClass sensLevel(2000000);
BLDCSpinnerClass spinner(PA6, 1000); 
SERVOHopperClass hopper (PB6, 750); // servo 750 - 1500 - 2250
TT_MODBUS_SLAVE  MSpread(&SerialMain, baud, SerialMain_Pincontrol485, SLAVEID);

void setup()
{
    // ========================== KHAI BAO HE THONG ===================== //
    MSpread.init();
    // ========================== DOI 15S SETTUP HE THONG ===================== //
    bool check_operation = false;
    unsigned long time1 = 0;
    time1 = HAL_GetTick();
    pinMode(LED, OUTPUT);
    while (!check_operation)
    {       
        digitalWrite(LED, HIGH);
        unsigned long time2 = HAL_GetTick();
        bool _led = false;
        while ((unsigned long)(HAL_GetTick() - time1) < 15000)
        {
            if ((unsigned long)(HAL_GetTick() - time2) > 500)
            {
                _led = !_led;
                digitalWrite(LED, _led);      
                time2 = HAL_GetTick();          
            }
        }
        digitalWrite(LED, LOW);
        check_operation = true;
        break;    
    }

    uint8_t _sLvpercent = 0;
    uint8_t stepSetupSpinner = 0;
    unsigned long _coef;
    unsigned long *coef;
    unsigned long timeSetupSpinner = 0;
    bool Peripheral_checkSetup = true;
    uint8_t Peripheral_step = 0;
    uint16_t timeSetupSpinnerLimit = 12000;

    while (Peripheral_checkSetup)
    {
        switch (Peripheral_step)
        {
        case 0:  // check connect with module Common.
            while (!MSpread.check_connect_master(FR0, READ, WRITE));
            Peripheral_step = 1;
            break;
        case 1: // scale init
            scale_init();
            Peripheral_step = 2;   
            break;
        case 2: // level sensor init
            sensLevel.init();
            while(sensLevel.startMotor(sLv.control,&_sLvpercent) != 100)
            {
                _sLvpercent = sensLevel.startMotor(sLv.control,&_sLvpercent);
            }
            sensLevel.startMotor(true, &_sLvpercent);
            Peripheral_step = 3;
            break;
        case 3: // spinner init
            spinner.init();
            timeSetupSpinner = HAL_GetTick();
            while ((unsigned long)(HAL_GetTick() - timeSetupSpinner) < timeSetupSpinnerLimit);        
            Peripheral_step = 4; 
            break;            
        case 4: // hopper init
            hopper.init();
            Peripheral_checkSetup = false;
            break;
        default:
            break;
        }
    }
    while (!MSpread.check_connect_master(FR0, READ, WRITE));
    //Serial2.println("vaiz");
}

void loop()
{
    scale_update();
    uint16_t sPi_read = spinner.readRPM();
    sensLevel.readVolume(&sLv._haveVolume);
    uint8_t sLv_read = 0;
    if (sLv._haveVolume == "0")
    {
        sLv_read = FR4_STOP ; 
    }
    else if (sLv._haveVolume == "1")
    {
        sLv_read = FR4_RUN;
    }
    switch (MSpread.receive())
    {
    case FR1: // scale
        if (MSpread.recFr[2] == WRITE)  
        {
            if (MSpread.recFr[3] == FR1_CALIB_TARE)
            {
                scale_regs[FUNC] = FUNCTION_SCALE_CALIB_TARE;
            }
            else if (MSpread.recFr[3] == FR1_CALIB_ZERO)
            {
                scale_regs[FUNC] = FUNCTION_SCALE_CALIB_EMPTY_TANK;
            }
            else if (MSpread.recFr[3] < 21)
            {
                scale_regs[FUNC] = (MSpread.recFr[3] << 8) | FUNCTION_SCALE_CALIB_20KG;
            }            
        }
        MSpread.updateFrame(    FR1,
                                MSpread.recFr[2],
                                scale_valueH,
                                scale_valueL);
        break;
    case FR2: // spinner
        if (MSpread.recFr[2] == WRITE) sPi.val = (MSpread.recFr[3] << 8) | MSpread.recFr[4];
        MSpread.updateFrame(    FR2,
                                MSpread.recFr[2],
                                (uint8_t) (sPi_read >> 8),
                                (uint8_t) (sPi_read & 0xFF));
        break;
    case FR3: // hopper 
        if (MSpread.recFr[2] == WRITE) hopper.setOpenPercent(MSpread.recFr[3]);
        MSpread.updateFrame(    FR3,
                                MSpread.recFr[2],
                                (uint8_t) hopper.readPercent(),
                                30);
        break;
    case FR4: // level
        if (MSpread.recFr[2] == WRITE)  
        {
            if (MSpread.recFr[3] == FR4_RUN)
            {
                sLv.control = true;
            }
            else if (MSpread.recFr[3] == FR4_STOP)
            {
                sLv.control = false;
            }
        }
        MSpread.updateFrame(    FR4,
                                MSpread.recFr[2],
                                sLv_read,
                                2);
        break; 
    }
    spinner.setRPM(sPi.val, &sPi.time1); 
    if(sLv.last_control != sLv.control)
    {
        sensLevel.startMotor2(sLv.control);
        sLv.last_control = sLv.control;
    }
}

// ======================== FUNCTION -> SCALE =============================== //
void scale_init()
{
    // Initialize each packet
    modbus_construct(&scale_packets[SCALEVALUE],  1, READ_HOLDING_REGISTERS,  0, 2, 0);
    modbus_construct(&scale_packets[CALIB],       1, PRESET_SINGLE_REGISTER,  2, 1, 2);

    // Initialize the Modbus Finite State Machine
    modbus_configure(&SerialScale, 115200, SERIAL_8N1, timeout, polling, retry_count, scale_TxEnablePin, scale_packets, TOTAL_NO_OF_PACKETS, scale_regs);
    SerialScale.println(F_CPU);

    scale_regs[2] = FUNCTION_VALUE ;
}

void scale_update()
{
    modbus_update();
    //int16_t _v , _b ;
    uint8_t regs_temp = scale_regs[FUNC]& 0xFF ;
    switch (regs_temp)
    {
    case FUNCTION_SCALE_CALIB_TARE:
        if (scale_regs[BYTE2] == FUNCTION_SCALE_CALIB_TARE_DONE)
        {
            scale_regs[FUNC] = FUNCTION_VALUE ;
        }
        break;
    case FUNCTION_VALUE :
        //_v = scale_regs[BYTE1] * 100 ;
        //_b = (_v > 0) ? scale_regs[BYTE2] : - scale_regs[BYTE2] ;
        scale_valueH =  scale_regs[BYTE1];
        scale_valueL =  scale_regs[BYTE2];
        break;
    case FUNCTION_SCALE_CALIB_EMPTY_TANK:
        if (scale_regs[BYTE2] == FUNCTION_SCALE_CALIB_EMPTY_DONE)
        {
            scale_regs[FUNC] = FUNCTION_VALUE ;
        }
        break;   
    case FUNCTION_SCALE_CALIB_20KG:
        if (scale_regs[BYTE2] == FUNCTION_SCALE_CALIB_20KG_DONE)
        {
            scale_regs[FUNC] = FUNCTION_VALUE ;
        }
        break;             
    default:
        break;
    }
}

