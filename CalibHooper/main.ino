#include "./src/define.hpp"

UARTSensLevelClass sensLevel(2000000);
BLDCSpinnerClass spinner(PA6, 1000); 
ModuleScaleClass scale(115200); //scale
SERVOHopperClass hopper(PB6, 750); // servo 750 - 1500 - 2250

unsigned long timer1 = 0;


void debugPrintStr(String str, bool newline);


void dbug485Str(String data);
void hopperMode(uint8_t mode);
void selectMode();

//respone data calib
void receiceCalib();

//receive from master

void receive485();

//send data 485
void sendData485();

static uint16_t MODBUS_CRC16( const unsigned char *buf, unsigned int len);
void setup(){
    rs485.begin(115200);
    dbug.begin(115200);
    pinMode(RS485_Pin, OUTPUT);
    //intit scale
    scale.init();
    //init hopper
    hopper.init();
    //init level
    sensLevel.init();
    if(_sys.checkStatus == true)
    {
        while(sensLevel.startMotor(_sLv.setupMotor, &_sLv.sLvpercent) != 100)
        {
            _sLv.sLvpercent = sensLevel.startMotor(_sLv.setupMotor, &_sLv.sLvpercent);
            dbug485Str("sensor " + (String)_sLv.sLvpercent + "%");

        }
        dbug485Str("Setup Sensor done");

        //init spinner
        spinner.init();
        _spin.timSetup = HAL_GetTick();
        while((unsigned long) (HAL_GetTick() - _spin.timSetup) < _spin.timeSetupLimit)
        {
            switch (_spin.stepSetup)
            {
            case 0:
                if((unsigned long) HAL_GetTick() - _spin.timSetup > 3000)
                {
                    dbug485Str("Spinner 25%");
                    _spin.stepSetup = 1;
                }
                break;
            case 1:
                if((unsigned long) HAL_GetTick() - _spin.timSetup > 6000)
                {
                    dbug485Str("Spinner 50%");
                    _spin.stepSetup = 2;
                }
                break;
            case 2:
                if((unsigned long)HAL_GetTick() - _spin.timSetup > 9000)
                {
                    dbug485Str("Spinner 75%");
                    _spin.stepSetup = 3;
                }
                break;
            case 3: 
                if((unsigned long) HAL_GetTick() - _spin.timSetup > 11500)
                {
                    dbug485Str("Spinner 100%");
                    _spin.stepSetup = 4;
                }
                break;
            }

        }

        dbug485Str("Setup Spinner done");
        hopper.setOpenPercent(0);
        //hopper.setOpenPercent(50);
        //_sLv.volumeStatus="0";
        dbug485Str("Set up done");

    }
    dbug485Str("Setup Done");
    //debugPrintStr((String)hopper.readPWM());
    _sys.timer = HAL_GetTick();
    _sys.timePrint = HAL_GetTick();
    timer1 = HAL_GetTick();
   // hopper.setOpenPercent(_servoHop.pwmCalib[18]);
//    _scl.scaleValue = 8;
//    _sLv.volumeStatus = "0";
    //;

}
void loop(){

    //read value motor level
    sensLevel.readVolume(&_sLv.volumeStatus);
    receive485();
    selectMode();
    /************READ 485****************/
    scale.readScale();
    scale.readScaleValue(&_scl.scaleVal);
    if(_sys.checkScale == true)
    {
        _scl.scaleValue = (_scl.scaleVal).toDouble() / 100.0;
        if((unsigned long) HAL_GetTick() - timer1 > 101)
        {
            dbug485Str((String)_scl.scaleValue);
            timer1  = HAL_GetTick();
        }
    }
    
     //dbug485Str((String)_scl.scaleValue);
    /******************CALIB TARE********************/
    scale.readCalibTare(&_scl.calibTareVal);
    if(_scl.calibTareVal != ""){
        dbug485Str((String)_scl.calibTareVal);
        _scl.calibTareVal="";
    }
    /*******************CALIB EMPTY*****************/
    scale.readCalibEmptyTank(&_scl.calibEmptyVal);
    if(_scl.calibEmptyVal != ""){
        dbug485Str(_scl.calibEmptyVal);
        _scl.calibEmptyVal ="";
    }
    /*****************CALIB 20KG*******************/
    scale.readCalib20Kg(&_scl.calib20kgVal);
    if(_scl.calib20kgVal != ""){
        dbug485Str((String)_scl.calib20kgVal);
        _scl.calib20kgVal="";
    }
    
    //select calib hooper 
    hopperMode();
    
    
}
void dbug485Str(String data){
   digitalWrite(RS485_Pin, HIGH);
   rs485.print(data);
   rs485.write(0x0D);
   //rs485.println();
   rs485.flush();
   digitalWrite(RS485_Pin, LOW);
}

//select mode
void selectMode()
{
    switch (_scl.modeCalib)
    {
    case 1:
        dbug485Str("Calib Tare");
        scale.setCalibTare(FUNCTION_CALIBTARE);
        _scl.modeCalib=0;
        break;
    case 2: 
        dbug485Str("Calib empty");
        scale.setCalibEmptyTank(FUNCTION_CALIBEMPTY);
        _scl.modeCalib = 0;
        break;
    case 3:
        dbug485Str("Calib 20kg");
        scale.setCalib20Kg(FUNCTION_CALIB20KG);
        _scl.modeCalib = 0;
        break;
    }
}



void hopperMode(){
    //check scale
    if( (_scl.scaleValue > 6) && (_scl.scaleValue < 11) && (_sLv.volumeStatus == "0")){
        switch (_servoHop.servo_Mode)
        {
            //chuan bi calib
            case 1: //start spinner
                dbug485Str("Start calib");
                _spin.timeSet = HAL_GetTick();
                _sys.lastWeight = _scl.scaleValue;
                _servoHop.servo_Mode = 2;
                //not print scale value
                _sys.checkScale  = false;

            break;
            case 2:
                //set spinner value
                spinner.setRPM(800, &_spin.timeSet);
                dbug485Str("PWM: " + (String)spinner.readPWM());
                if(spinner.readPWM() == 1500)
                {
                    _servoHop.servo_Mode =3;
                }
                break;
            case 3:
                if(_sLv.volumeStatus == "0"){
                    hopper.setOpenPercent(_servoHop.pwmCalib[0]);
                    //read scale value
                    _scl.scaleValue = (_scl.scaleVal).toDouble() / 100.0;
                    if(_sys.lastWeight - _scl.scaleValue > 0.2)
                    {
                        _servoHop.servo_Mode =4;
                        _sys.timer  = HAL_GetTick();
                    }
                }
                break;
        }
    }
    switch (_servoHop.servo_Mode)
    {
        case 4:
            //check level
            if(_sLv.volumeStatus == "0")
            { 
                if((unsigned long)HAL_GetTick() - _sys.timer > 6000)
                {
                    hopper.setOpenPercent(_servoHop.pwmCalib[_servoHop.numCalib]);
                    //read value scale
                    _scl.scaleValue  = (_scl.scaleVal).toDouble() / 100.0;
                    dbug485Str("Scale Calib: " + (String)_scl.scaleValue);
                    _calibHop.weightCalib[_servoHop.numCalib] = _sys.lastWeight - _scl.scaleValue;
                    //scale value 
                    _sys.lastWeight = _scl.scaleValue;
                    _servoHop.numCalib++;  
                    _sys.timer = HAL_GetTick();          
                }
                if(_servoHop.numCalib == _sys.stepCalib)
                {
                    dbug485Str("Done hopper");
                    _servoHop.numCalib=0;
                    //write data
                    fitCurve(1, _sys.stepCalib, _servoHop.pwmCalib, _calibHop.weightCalib, 2, _sys.coefCalib);
                    dbug485Str("coef 1: ");
                    dbug485Str((String)_sys.coefCalib[0]);
                    dbug485Str("coef 2: ");
                    dbug485Str((String)_sys.coefCalib[1]);
                    _servoHop.servo_Mode = 5;
                    _sys.timeDelay = HAL_GetTick();
                    //level calib
                     //_sLv.volumeStatus = "1";
                }
            }
            
            break;

        //xa he giong
        case 5:
            if(_sLv.volumeStatus == "1")
            {
                if((unsigned long) HAL_GetTick() - _sys.timeDelay > 2000)
                {
                    _servoHop.servo_Mode = 6;
                    dbug485Str("Close");
                }
            }
            break;
        case 6:
            if(_sLv.volumeStatus == "1"){
                spinner.setRPM(250, &_spin.timeSet);
                if(spinner.readPWM() == 1000){
                    hopper.setOpenPercent(0);
                    dbug485Str((String)spinner.readPWM());
                    _servoHop.servo_Mode = 7;
                    dbug485Str("Print he so ");
                }   
            }
            break;
        case 7:
            // if((unsigned long) HAL_GetTick()  - _sys.timer > 500){
            //     dbug485Str((String)EEPROM.read(_eeprom.eepAddress));
            //     _eeprom.eepAddress++;
            //     _send485.senderValue++;
            //     if(_send485.senderValue==19){
            //     _servoHop.servo_Mode=0; 
            //     }
            //     _sys.timer=HAL_GetTick();
            // }
            
            if(_sLv.volumeStatus == "1")
            {    
                
                dbug485Str((String) _calibHop.weightCalib[_print.count]);
                _print.count ++;
            
                if(_print.count == _sys.stepCalib){
                    dbug485Str("Calib Successfull");
                    _print.count = 0;
                    _servoHop.servo_Mode = 0;
                }
                    
            }
            break;
    }
}

//modbus
static uint16_t MODBUS_CRC16( const unsigned char *buf, unsigned int len)
{
	uint16_t crc = 0xFFFF;
	char i = 0;

	while(len--)
	{
		crc ^= (*buf++);

		for(i = 0; i < 8; i++)
		{
			if( crc & 1 )
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}

	return crc;
}

//receive data from 485
void receive485()
{
    if(rs485.available()){ 
        if(rs485.peek() != 0x0D){
            _recei485.dataRecei += (char)rs485.read();
        }
        else
        {
            if((_recei485.dataRecei).indexOf('S')==0){
                _scl.modeCalib  = (_recei485.dataRecei.substring(1)).toInt();
            }
            else if((_recei485.dataRecei).indexOf('C')==0)
            {
                _servoHop.servo_Mode = (_recei485.dataRecei.substring(1)).toInt();
            }
            _recei485.dataRecei = "";
            byte tmp = rs485.read();
        }
    }
}

void debugPrintStr(String str, bool newline)
{
    digitalWriteFast(digitalPinToPinName(PA8),HIGH);
    rs485.print(str);
    rs485.flush();
    if (newline)
    {
        rs485.println();
        rs485.flush();
    }
    digitalWriteFast(digitalPinToPinName(PA8),LOW);
}