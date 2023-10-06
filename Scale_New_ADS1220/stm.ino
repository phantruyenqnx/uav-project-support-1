/***************************************************************************
  Example sketch for the ADS1220_WE library

  This sketch explains all settings. You can use the sketch as a basis for your
  own sketches.

  On my website you find:
     1) An article about the ADS1220 and the use of this library
        https://wolles-elektronikkiste.de/ads1220-4-kanal-24-bit-a-d-wandler  (German) 
        https://wolles-elektronikkiste.de/en/4-channel-24-bit-adc-ads1220     (English)
 
    2) An article how to use the ADS1220 for typical applications, such as thermocouples, 
       NTCs, RTDs or Wheatstone bridges:
       https://wolles-elektronikkiste.de/ads1220-teil-2-anwendungen          (German)
       https://wolles-elektronikkiste.de/en/ads1220-part-2-applications      (English)

***************************************************************************/

#include "src/define.hpp"

void setup() {
    serDebug.begin(115200);
    modbus_configure(&serScale, 115200, SERIAL_8N1, 1, PA8, HOLDING_REGS_SIZE, holdingRegs);

    // modbus_update_comms(baud, byteFormat, id) is not needed but allows for easy update of the
    // port variables and slave id dynamically in any function.
    modbus_update_comms(115200, SERIAL_8N1, 1);

    if (!ads1.init())
    {
        serDebug.println("ADS1 is not connected!");
        while (1);
    }
    if (!ads2.init())
    {
        serDebug.println("ADS2 is not connected!");
        while (1);
    }
    if (!ads3.init())
    {
        serDebug.println("ADS3 is not connected!");
        while (1);
    }
    if (!ads4.init())
    {
        serDebug.println("ADS4 is not connected!");
        while (1);
    }
  /* General settings / commands */
  //  ads.start(); // wake up from power down and start measurement
  //  ads.reset(); // resets the ADS1220; all settings will change to default
  //  ads.powerDown(); // send the ADS1220 to sleep
  //  ads.setSPIClockSpeed(8000000); // set SPI clock speed, default is 4 MHz

    ads1.start(); ads2.start(); ads3.start(); ads4.start();
    ads1.reset(); ads2.reset(); ads3.reset(); ads4.reset();
    ads1.powerDown(); ads2.powerDown(); ads3.powerDown(); ads4.powerDown();
    ads1.setSPIClockSpeed(8000000); ads2.setSPIClockSpeed(8000000); 
    ads3.setSPIClockSpeed(8000000); ads4.setSPIClockSpeed(8000000);

  /* You set the channels to be measured with setCompareChannels(); You
     can choose the following parameters:
     Parameter                  Pos. Input     Neg. Input         Comment
     ADS1220_MUX_0_1              AIN0           AIN1             default
     ADS1220_MUX_0_2              AIN0           AIN2
     ADS1220_MUX_0_3              AIN0           AIN3
     ADS1220_MUX_1_2              AIN1           AIN2
     ADS1220_MUX_1_3              AIN1           AIN3
     ADS1220_MUX_2_3              AIN2           AIN2
     ADS1220_MUX_1_0              AIN1           AIN0
     ADS1220_MUX_3_2              AIN3           AIN2
     ADS1220_MUX_0_AVSS           AIN0           AVSS (=AGND)   single-ended
     ADS1220_MUX_1_AVSS           AIN1           AVSS           single-ended
     ADS1220_MUX_2_AVSS           AIN2           AVSS           single-ended
     ADS1220_MUX_3_AVSS           AIN3           AVSS           single-ended
     ADS1220_MUX_REFPX_REFNX_4   REFP0/REFP1   REFN0/REFN1     (REFPX-REFNX)/4; PGA bypassed
     ADS1220_MUX_AVDD_M_AVSS_4    AVDD           AVSS          (AVDD-AVSS)/4; PGA bypassed
     ADS1220_MUX_AVDD_P_AVSS_2    AVDD           AVSS           AVDD+AVSS)/2
     The last three modes use the internal reference (2.048 V) and gain = 1, independent of
     your settings.
  */
  //  ads.setCompareChannels(ADS1220_MUX_0_3);
    ads1.setCompareChannels(ADS1220_MUX_1_2);
    ads2.setCompareChannels(ADS1220_MUX_1_2);
    ads3.setCompareChannels(ADS1220_MUX_1_2);
    ads4.setCompareChannels(ADS1220_MUX_1_2);
  /* You can choose a gain between 1 (default) and 128 using setGain() if PGA is enabled
     (default). If PGA is disabled you can still choose a gain factor up to 4. If PGA is
     enabled, the amplified voltage shall be between AVSS + 200mV and AVDD - 200mV. Outside
     this range linearity drops. For details check the data sheet, section 8.3.2.1.

     If you apply a single-ended mode (negative AINx = AVSS), PGA must be bypassed. Accordingly,
     the maximum gain is 4. The library does these settings automatically.

     For the measurement of reference voltages / supply voltage PGA will also be bypassed. In
     this case gain is 1.

     The parameters you can choose for setGain() are:
     ADS1220_GAIN_X with X = 1,2,4,8,16,32,64 or 128

     With getGainFactor() you can query the gain. The function returns the effective gain and
     not the gain set in the register. Under certian conditions thes are are different. For
     example, the effective gain is set to 1 when external references are measured.
  */
    ads1.setGain(ADS1220_GAIN_128);
    ads2.setGain(ADS1220_GAIN_128);
    ads3.setGain(ADS1220_GAIN_128);
    ads4.setGain(ADS1220_GAIN_128);
  //  ads.getGainFactor(); // returns the effective gain as a byte value
  //  ads.bypassPGA(true); // true disables PGA, false enables PGA
  //  ads.isPGABypassed(); // returns true, if PGA is bypassed

  /* The data rate level with setDataRate(). The data rate itself also depends on the operating
     mode and the clock. If the internal clock is used or an external clock with 4.096 MHz the data
     rates are as follows (per second):

      Level               Normal Mode      Duty-Cycle      Turbo Mode
     ADS1220_DR_LVL_0          20               5               40         (default)
     ADS1220_DR_LVL_1          45              11.25            90
     ADS1220_DR_LVL_2          90              22.5            180
     ADS1220_DR_LVL_3         175              44              350
     ADS1220_DR_LVL_4         330              82.5            660
     ADS1220_DR_LVL_5         600             150             1200
     ADS1220_DR_LVL_6        1000             250             2000

     The higher the data rate, the higher the noise (tables are provided in section 7.1 in the
     data sheet). In single-shot mode the conversion times equal the times in Normal Mode.
  */
    ads1.setDataRate(ADS1220_DR_LVL_6);
    ads2.setDataRate(ADS1220_DR_LVL_6);
    ads3.setDataRate(ADS1220_DR_LVL_6);
    ads4.setDataRate(ADS1220_DR_LVL_6);
  /* Using setOperatingMode() you choose the operating mode. Possible parameters are:
     ADS1220_NORMAL_MODE      ->  Normal Mode
     ADS1220_DUTY_CYCLE_MODE  ->  Duty cycle mode. Saves power, but noise is higher.
     ADS1220_TURBO_MODE       ->  Turbo Mode for fast measurements
  */
  //  ads.setOperatingMode(ADS1220_DUTY_CYCLE_MODE);
    ads1.setOperatingMode(ADS1220_TURBO_MODE);
    ads2.setOperatingMode(ADS1220_TURBO_MODE);
    ads3.setOperatingMode(ADS1220_TURBO_MODE);
    ads4.setOperatingMode(ADS1220_TURBO_MODE);
  /*  You can choose between a continuous and a single-shot (on demand) mode with
      setConversionMode(). Parameters are:
      ADS1220_SINGLE_SHOT (default)
      ADS1220_CONTINUOUS
  */
  // ads.setConversionMode(ADS1220_CONTINUOUS);

  /* In order to obtain temperature values, choose enableTemperatureSensor(true); false will
     disable the temperature sensor. As long as the temperature sensor is enabled the ADS1220
     is blocked for this task. To obtain voltage values, you have to switch the sensor off. The
     temperature is queried with getTemperature();
  */
  //  ads.enableTemperatureSensor(true);
  //  ads.getTemperature(); // returns temperature as float

  /*
     setVRefSource() sets the the reference voltage source. Parameters are:
     ADS1220_VREF_INT          int. reference 2.048 V (default)
     ADS1220_VREF_REFP0_REFN0  ext. reference = Vrefp0 - Vrefn0
     ADS1220_VREF_REFP1_REFN1  ext. reference = Vrefp1 - Vrefn1 (REFP1=AIN0, REFN1=AIN3)
     ADS1220_VREF_AVDD_AVSS    ext. reference = supply voltage

     If you use the above options you also have to set the value of vRef "manually":
     setVRefValue_V(vRef in volts);

     Alternatively, you can set the reference voltage source and let the ADS1220 measure
     the reference. Be aware that this is not a measurement with highest precision.
     "Calibration" might be a bit misleading. You should take the lowest data rate (default)
     for most accurate results. You can use the following functions:
     setRefp0Refn0AsVefAndCalibrate();
     setRefp1Refn1AsVefAndCalibrate();
     setAvddAvssAsVrefAndCalibrate();
     setIntVRef();
     The latter function sets the default settings.

     Be aware that VREFPx must be >= VREFNx + 0.75V.
  */
    ads1.setVRefSource(ADS1220_VREF_REFP1_REFN1);
    ads1.setVRefValue_V(5.75);  // just an example
    
    ads2.setVRefSource(ADS1220_VREF_REFP1_REFN1);
    ads2.setVRefValue_V(5.75);

    ads3.setVRefSource(ADS1220_VREF_REFP1_REFN1);
    ads3.setVRefValue_V(5.75);

    ads4.setVRefSource(ADS1220_VREF_REFP1_REFN1);
    ads4.setVRefValue_V(5.75);
  //  or:
  //  ads.setRefp0Refn0AsVefAndCalibrate(); //or:
  //  ads.setRefp1Refn1AsVefAndCalibrate(); //or:
  //  ads.setAvddAvssAsVrefAndCalibrate(); //or:
  //  ads.setIntVRef();
  //  to query VRef:
  //  ads.getVRef_V(); // returns VRef as float

  /* You can set a filter to reduce 50 and or 60 Hz noise with setFIRFilter(); Parameters:
     ADS1220_NONE       no filter (default)
     ADS1220_50HZ_60HZ  50Hz and 60Hz filter
     ADS1220_50HZ       50Hz filter
     ADS1220_60HZ       60Hz filter
  */
  //  ads.setFIRFilter(ADS1220_50HZ_60HZ);

  /* When data is ready the DRDY pin will turn from HIGH to LOW. In addition, also the DOUT pin
     can be set as a data ready pin. The function is setDrdyMode(), parameters are:
     ADS1220_DRDY        only DRDY pin is indicating data readiness  (default);
     ADS1220_DOUT_DRDY   DRDY and DOUT pin indicate data readiness
  */
  //  ads.setDrdyMode(ADS1220_DOUT_DRDY);


  /* There is a switch between AIN3/REFN1 and AVSS. You can use this option to save power in
     bridge sensor applications.
     ADS1220_ALWAYS_OPEN    The switch is always open.
     ADS1220_SWITCH         Switch automatically closes when the START/SYNC command is sent
                            and opens when the POWERDOWN command is issued.
  */
    ads1.setLowSidePowerSwitch(ADS1220_SWITCH);
    ads2.setLowSidePowerSwitch(ADS1220_SWITCH);   
    ads3.setLowSidePowerSwitch(ADS1220_SWITCH); 
    ads4.setLowSidePowerSwitch(ADS1220_SWITCH); 
  /* The ADS1220 can provide two excitation currents, IDAC1 and IDAC2. It takes up to 200µs
     until the current is set up. The library includes a delay, so you don't have to add one.
     You can switch IDAC1 and IDAC2 on and off individually but the current is the same for 
     both.
     The ADS1220 will try to provide the voltage needed for the current you have chosen. This 
     voltage shall not exceed AVDD - 0.9V.

     ADS1220_IDAC_OFF         // default
     ADS1220_IDAC_10_MU_A     // set IDAC1/IDAC2 to 10 µA
     ADS1220_IDAC_50_MU_A     // 50 µA
     ADS1220_IDAC_100_MU_A    // 100 µA
     ADS1220_IDAC_250_MU_A    // 250 µA
     ADS1220_IDAC_500_MU_A    // 500 µA
     ADS1220_IDAC_1000_MU_A   // 1000 µA
     ADS1220_IDAC_1500_MU_A   // 1500 µA
  */
  // ads.setIdacCurrent(ADS1220_IDAC_50_MU_A);

  /* You can choose to which pin IDAC1 and IDAC2 are directed. The parameters are self-explaining.
     ADS1220_IDAC_NONE
     ADS1220_IDAC_AIN0_REFP1
     ADS1220_IDAC_AIN1
     ADS1220_IDAC_AIN2
     ADS1220_IDAC_AIN3_REFN1
     ADS1220_IDAC_REFP0
     ADS1220_IDAC_REFN0
  */
  //  ads.setIdac1Routing(ADS1220_IDAC_AIN0_REFP1);
  //  ads.setIdac2Routing(ads1220IdacRouting route);
    eeproomVar.m = getDoubleFromEEPROM(ADDR_HESO_M);
    eeproomVar.n = getDoubleFromEEPROM(ADDR_HESO_N);
    tareVar.offset[0] = getDoubleFromEEPROM(ADDR_OFFSET0);
    tareVar.offset[1] = getDoubleFromEEPROM(ADDR_OFFSET1);
    tareVar.offset[2] = getDoubleFromEEPROM(ADDR_OFFSET2);
    tareVar.offset[3] = getDoubleFromEEPROM(ADDR_OFFSET3);
    //calibTare() ;  
} // end of setup()

void loop() {
    // float result = 0.0;

    /* The following functions query measured values from the ADS1220. If you request values in
        single-shot mode, the conversion will be initiated automatically. The value will be delivered
        once DRDY goes LOW. This ensures that you will receive "fresh" data. This is particularly
        important when you change channels.
    */
    
    // result = ads1.getVoltage_mV(); // get result in millivolts
    //  ads.getVoltage_muV(); // get result in microvolts
    //  ads.getRawData();  // get raw result (signed 24 bit as long int)
    //  ads.getTemperature();  // get temperature (you need to enable the T-sensor before);

    // 5ms
    // serDebug.print(normalize((double)ads1.getRawData(), -1000000, 1000000, 2000000));//ads1.getRawData());
    // serDebug.print("  ");
    // serDebug.print(normalize((double)ads2.getRawData(), -1000000, 1000000, 2000000));//ads2.getRawData());
    // serDebug.print("  ");
    // serDebug.print(normalize((double)ads3.getRawData(), -1000000, 1000000, 2000000));//ads3.getRawData());
    // serDebug.print("  ");
    // //serDebug.println(ads1.getRawData());
    // serDebug.println(normalize((double)ads4.getRawData(), -1000000, 1000000, 2000000));//ads4.getRawData()); 
    // long _r[4] = {0};
    // readAfterTare(_r);
    // serDebug.print(_r[0]); serDebug.print("  "); 
    // serDebug.print(_r[1]); serDebug.print("  ");
    // serDebug.print(_r[2]); serDebug.print("  ");
    // serDebug.println(_r[3]);

    // if (serDebug.available() > 0)
    // {
    //     char c = serDebug.read();
    //     switch (c)
    //     {
    //     case '0':
    //         serDebug.println("CALZ");
    //         zeroCalib();
    //         //serDebug.println(eeproomVar.a);
    //         serDebug.println("OK");
    //         break;
    //     case '1':
    //         serDebug.println("CAL2");
    //         //calib20kg(10.31);
    //         calib20kg(eeproomVar.kg_ref);
    //         // serDebug.println(eeproomVar.m);
    //         // serDebug.println(eeproomVar.n);
    //         serDebug.println("OK");
    //         break;
    //     case '2':
    //         serDebug.println("CALT");
    //         calibTare(); 
    //         serDebug.println("OK");
    //         break;    
    //     default:
    //         break;
    //     }
    // }
    modbus_update();

    double scaleValue = kalmanFilter.updateEstimate(x_raw()/1000.0)*eeproomVar.m + eeproomVar.n;
    //sendScaleValue(scaleValue);
    // if (holdingRegs[CALIB] != FUNCTION_SCALE_CALIB_TARE)
    // {
    //     if ((scaleValue < 100) && (scaleValue > - 100))
    //     {
    //         int16_t _value = (int16_t)(roundf((scaleValue)*100));
    //         holdingRegs[BYTE1] = _value / 100 ;
    //         holdingRegs[BYTE2] = (abs)(_value) % 100 ;

    //         // int16_t _v = holdingRegs[BYTE1] * 100 ;
    //         // int16_t _b = (_v > 0) ? holdingRegs[BYTE2] : - holdingRegs[BYTE2] ;

    //         // serDebug.print(holdingRegs[BYTE1]) ;
    //         // serDebug.print(" ");
    //         // serDebug.print(holdingRegs[BYTE2]);
    //         // serDebug.print(" ");
    //         // serDebug.print(_v + _b);
    //         // serDebug.print(" ");
    //         // serDebug.println(scaleValue);
    //     }
    // }
    


    //holdingRegs[CALIBREC] = FUNCTION_SCALE_CALIB_TARE_DONE;
    int16_t _value = 0 ;
    uint8_t regs_temp = holdingRegs[CALIB] & 0xFF ;
    switch (regs_temp)
    {
    case FUNCTION_SCALE_CALIB_TARE:
        //serDebug.println("TARE");
        //calibTare();
        _calibtare++;
        if (_calibtare == 1)
        {
            serDebug.println("TARE");
            _tareSuccessful = false;  
            _tareStartTime = HAL_GetTick(); 
            while (!_tareSuccessful && HAL_GetTick()<(_tareStartTime+10000)) 
            { _tareSuccessful = tare(500,5000, tareVar.offset);} 
            holdingRegs[BYTE1] = 0x00;
            holdingRegs[BYTE2] = FUNCTION_SCALE_CALIB_TARE_DONE;
        }
        break;
    case FUNCTION_SCALE_CALIB_EMPTY_TANK:
        //serDebug.println("ZERO");
        calibzero++;
        if (calibzero == 1)
        {
            serDebug.println("ZERO");
            sum_last = 0.00;
            for (int j = 0; j < 1000 ; j++)
            {
                sum_last += kalmanFilter.updateEstimate(x_raw()/1000.0) / 1000 ;  
                modbus_update();
            }    
            eeproomVar.a = sum_last;
            holdingRegs[BYTE1] = 0x00;
            holdingRegs[BYTE2] = FUNCTION_SCALE_CALIB_EMPTY_DONE;
        }    
        break;
    case FUNCTION_SCALE_CALIB_20KG:
        // serDebug.println("20KG");
        // calib20kg(holdingRegs[CALIB] >> 8);
        // serDebug.println(holdingRegs[CALIB] >> 8);
        // holdingRegs[BYTE1] = 0x00;
        // holdingRegs[BYTE2] = FUNCTION_SCALE_CALIB_20KG_DONE;
        _calib20kg++;
        if (_calib20kg == 1)
        {
            serDebug.println("20KG");
            serDebug.println(holdingRegs[CALIB] >> 8);
            eeproomVar.kg_ref = (double)(holdingRegs[CALIB] >> 8);
            _temp = 0.00;
            for (int j = 0; j < 1000 ; j++)
            {
                _temp += kalmanFilter.updateEstimate(x_raw()/1000.0) / 1000 ; 
            }    
            eeproomVar.b = _temp;   

            eeproomVar.m = eeproomVar.kg_ref / (eeproomVar.b - eeproomVar.a);
            eeproomVar.n = - eeproomVar.m * eeproomVar.a ; 

            saveDoubleToEEPROM(ADDR_HESO_M, eeproomVar.m);
            saveDoubleToEEPROM(ADDR_HESO_N, eeproomVar.n);
            holdingRegs[BYTE1] = 0x00;
            holdingRegs[BYTE2] = FUNCTION_SCALE_CALIB_20KG_DONE;
        }
        
        break;    
    default:
        if ((scaleValue < 100) && (scaleValue > - 100))
        {
            _value = (int16_t)(roundf((scaleValue)*100));
            holdingRegs[BYTE1] = _value / 100 ;
            holdingRegs[BYTE2] = (abs)(_value) % 100 ;
            serDebug.println(scaleValue);
            sum_last = 0.00;
            calibzero = 0;
            _calibtare = 0;
            _calib20kg = 0;
        }
        break;
    }

    // if ((calibzero == 1) && (regs_temp == FUNCTION_SCALE_CALIB_EMPTY_TANK))
    // {
    //     if(count < 1000)
    //     {
    //         sum_last += kalmanFilter.updateEstimate(x_raw()/1000.0) / 1000 ; 
    //         serDebug.println(sum_last);
    //         count++;
    //     }
    //     else 
    //     {
    //         eeproomVar.a = sum_last; 
    //         count = 0;
    //         calibzero = 2;
    //         sum_last = 0.00;
    //         holdingRegs[BYTE1] = 0x00;
    //         holdingRegs[BYTE2] = FUNCTION_SCALE_CALIB_EMPTY_DONE;
    //         serDebug.println("ZERO11");
    //     }
    // }

    // digitalWriteFast(digitalPinToPinName(PA8),HIGH);
    // serDebug.println(scaleValue);
    // serDebug.flush();
    // digitalWriteFast(digitalPinToPinName(PA8),LOW);
    // double temp = kalmanFilter.updateEstimate(x_raw()/1000.0);

    // serDebug.print(temp);
    // serDebug.print("   ");
    // serDebug.println(temp*eeproomVar.m + eeproomVar.n);
}

// double readOutput() {
    
//     long _r[4] = {0};
//     readAfterTare(_r);
//     return ((double)_r[0] + (double)_r[1] + (double)_r[2] + (double)_r[3])/ 4.0 
// }

double x_raw()
{
    long _r[4] = {0};
    readAfterTare(_r);
    return ((double)_r[0] + (double)_r[1] + (double)_r[2] + (double)_r[3])/ 4.0 ;
}

void readAfterTare(long *result)
{
    readRaw(result);
    for (int j = 0; j < COUNT; ++j) {
		    result[j] = result[j] -  tareVar.offset[j];  
    }    
}

void readRaw(long *values)
{
    values[0] = -ads1.getRawData() + 1000000; 
    values[1] = -ads2.getRawData() + 1000000; 
    values[2] =  ads3.getRawData() + 1000000; 
    values[3] = -ads4.getRawData() + 1000000; 
}

/// calib tare

void calibTare() 
{ 
    bool tareSuccessful = false;  
    unsigned long tareStartTime = HAL_GetTick(); 
    while (!tareSuccessful && HAL_GetTick()<(tareStartTime+10000)) 
    { tareSuccessful = tare(500,5000, tareVar.offset); } 

    // serDebug.println("tare");
    // serDebug.println(tareVar.offset[0]);
    // serDebug.println(tareVar.offset[1]);
    // serDebug.println(tareVar.offset[2]);
    // serDebug.println(tareVar.offset[3]);
} 

bool tare(int times, uint16_t tolerance, long *_OFFSETS) {
	//TODO: change this implementation to use a smarter read strategy. 
	//		right now samples are read 'times' times, but only the last is used (the multiple samples)
	//		are just being used to verify the spread is < tolerance.
	//		
	//		This should be done by creating a smarter multiple-reads function which returns a struct with values and metadata (number of good samples, standard deviation, etc.) 
	int i,j;

	long values[COUNT];

	long minValues[COUNT];
	long maxValues[COUNT];

	for (i=0; i<COUNT; ++i) {
		minValues[i]=0x7FFFFFFF;
		maxValues[i]=0x80000000;

		//OFFSETS[i]=0; //<--removed this line, so that a failed tare does not undo previous tare
	}

	for (i=0; i<times; ++i) {
		readRaw(values);
		for (j=0; j<COUNT; ++j) {
			if (values[j]<minValues[j]) {
				minValues[j]=values[j];
			}	
			if (values[j]>maxValues[j]) {
				maxValues[j]=values[j];
			} 
            modbus_update();
		}		
	}

	if (tolerance!=0 && times>1) {
		for (i=0; i<COUNT; ++i) {
			if (abs(maxValues[i]-minValues[i])>tolerance) {
				//one of the cells fluctuated more than the allowed tolerance, reject tare attempt;
				if (tareVar.debugEnabled) {
					serDebug.print("Rejecting tare: (");
					serDebug.print(i);
					serDebug.print(") ");
					serDebug.println(abs(maxValues[i]-minValues[i]));
				}
				return false;
			}
		}
	}

	//set the offsets
	for (i=0; i<COUNT; ++i) {
		_OFFSETS[i] = values[i];
	}
    saveDoubleToEEPROM(ADDR_OFFSET0, _OFFSETS[0]);
    saveDoubleToEEPROM(ADDR_OFFSET1, _OFFSETS[1]);
    saveDoubleToEEPROM(ADDR_OFFSET2, _OFFSETS[2]);
    saveDoubleToEEPROM(ADDR_OFFSET3, _OFFSETS[3]);
	return true;

}

/// calib zero

void zeroCalib()
{
    sum_last = 0.00;
    for (int j = 0; j < 1000 ; j++)
    {
        sum_last += kalmanFilter.updateEstimate(x_raw()/1000.0) / 1000 ;  
    }    
    eeproomVar.a = sum_last;
}

// calib 20kg

void calib20kg(double kg_ref)
{
    double temp = 0.00;
    for (int j = 0; j < 1000 ; j++)
    {
        temp += kalmanFilter.updateEstimate(x_raw()/1000.0) / 1000 ; 
    }    
    eeproomVar.b = temp;   

    eeproomVar.m = kg_ref / (eeproomVar.b - eeproomVar.a);
    eeproomVar.n = - eeproomVar.m * eeproomVar.a ; 

    saveDoubleToEEPROM(ADDR_HESO_M, eeproomVar.m);
    saveDoubleToEEPROM(ADDR_HESO_N, eeproomVar.n);
}

// normalize
double normalize(double x, double min_val, double max_val, double new_range) {
    return (x - min_val) * new_range / (max_val - min_val);
}

// eeproom

void saveDoubleToEEPROM(int address, double value) {
  uint8_t* p = (uint8_t*)(void*)&value; // Convert the double pointer to a byte pointer
  for (int i = 0; i < sizeof(value); i++) {
    EEPROM.write(address++, *p++); // Write each byte of the double to EEPROM
  }
}

double getDoubleFromEEPROM(int address) {
  double value = 0;
  uint8_t* p = (uint8_t*)(void*)&value; // Convert the double pointer to a byte pointer
  for (int i = 0; i < sizeof(value); i++) {
    *p++ = EEPROM.read(address++); // Read each byte of the double from EEPROM
  }
  return value;
}

// void sendScaleValue(double value)
// {
//     if ((value < 100) && (value > - 100))
//     {
//         frValue[2] = (value >= 0) ? 0 : 1 ;
//         uint16_t _value = (uint16_t)(roundf((abs)(value)*100));
//         frValue[3] = _value / 1000;
//         frValue[4] = (_value % 1000) / 100 ;
//         frValue[5] = ((_value % 1000) % 100) / 10;
//         frValue[6] = ((_value % 1000) % 100) % 10;
//         // digitalWriteFast(digitalPinToPinName(PA8),HIGH);
//         // serDebug.print((frValue[2] == 0) ? ' ' : '-'); serDebug.flush();
//         // serDebug.print(frValue[3]); serDebug.flush();
//         // serDebug.print(frValue[4]); serDebug.flush();
//         // serDebug.print(frValue[5]); serDebug.flush();
//         // serDebug.print(frValue[6]); serDebug.flush();
//         // serDebug.println(); serDebug.flush();
//         // digitalWriteFast(digitalPinToPinName(PA8),LOW);
//         // sendFr(frValue, ELE(frValue));
//     }
// }
