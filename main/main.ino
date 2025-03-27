// Environmental Monitoring System
//
// Team: EcoSense
// Authors: Connor Bremner, Zahid Conteh, Brock Drengenberg, Daniel Langdon, Anthony Lu, Gabe De Matte, Jorge Sierra, Timothy Strawn
//
// Function: This code controls the environmental monitoring system by defining sensors with structs and functions.
//           Then calculates a data point for each sensor and formats it into a table
//           This tabel is sent out to a public SQL server
//
// Use Instructions: In sensor setup define sensor, option count, pin count
//                   In sensor functions include function for specific sensor
//                   In setup and loop follow example sensors



// INCLUDES
#include <GravityTDS.h>
GravityTDS TDS;

// MQ-5 Parameters
#define RL 1000  // Load resistance in ohms (1kΩ)
#define RO_CLEAN_AIR_FACTOR 6.5  // Clean air factor for MQ-5
static float Ro = 0;
 

// MACROS
#define DELTA_MS	250

// NAMING VARIABLE TYPES
typedef int32_t EMS_Int;
typedef float EMS_Float;

typedef uint32_t EMS_Time;

typedef uint8_t EMS_Option;
typedef uint8_t EMS_OptionCount;

typedef uint8_t EMS_PinValue;
typedef uint8_t EMS_PinCount;

static EMS_Time clk = 0;

enum EMS_DATA_TYPE{
	EMS_DATA_TYPE_INT,
	EMS_DATA_TYPE_FLOAT
};

struct EMS_DataPoint{
	EMS_Time recordedTime;
	union{
		EMS_Int dataInt;
		EMS_Float dataFloat;
	};
};

struct EMS_DataSeries{
	const char* name;
	enum EMS_DATA_TYPE type;
	struct EMS_DataPoint* array;
	struct EMS_DataSeries* next;
	size_t arrayLen;
};

struct EMS_Pin{
	EMS_PinValue pin;
	bool isAnalog;
};
enum EMS_SENSOR_TYPE{
	EMS_SENSOR_TYPE_CO2,
	EMS_SENSOR_TYPE_TDS,
	EMS_SENSOR_TYPE_CO,
};
struct EMS_Sensor{
	enum EMS_SENSOR_TYPE type;
	EMS_Option* settings;
	EMS_Pin* pins;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              SENSOR SETUP
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

<<<<<<< Updated upstream
enum EMS_SENSOR_TYPE{
	EMS_SENSOR_TYPE_CO2,
	EMS_SENSOR_TYPE_TDS,
	EMS_SENSOR_TYPE_CO,
  EMS_SENSOR_TYPE_MQ5,
};

=======
>>>>>>> Stashed changes
enum EMS_SETTING{
	EMS_SETTING_NULL = 0,

	// EMS_SETTING_CO2_MODE = 1,
};

enum{
	// EMS_OPTION_PH_MODE_INT = 0,
	// EMS_OPTION_PH_MODE_FLOAT = 1
};

enum EMS_PIN{
	EMS_PIN_NULL = 0,

	EMS_PIN_CO2_MAIN = 1,
	
	EMS_PIN_TDS_MAIN = 1,

	EMS_PIN_CO_MAIN = 1,

  EMS_PIN_MQ5_MAIN = 1,
};

enum EMS_READ_MODE{
	// EMS_READ_WATER_O2 = 0,
	// EMS_READ_WATER_N2,
};

EMS_OptionCount sensorOptionCount[] = {
	[EMS_SENSOR_TYPE_CO2] = 0,
	[EMS_SENSOR_TYPE_TDS] = 0,
	[EMS_SENSOR_TYPE_CO] = 0,
  [EMS_SENSOR_TYPE_MQ5] = 0,
};

EMS_PinCount sensorPinCount[] = {
	[EMS_SENSOR_TYPE_CO2] = 1,
	[EMS_SENSOR_TYPE_TDS] = 1,
	[EMS_SENSOR_TYPE_CO] = 1,
  [EMS_SENSOR_TYPE_MQ5] = 1,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              MAIN FUNCTIONS
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct EMS_Sensor createSensor(enum EMS_SENSOR_TYPE type){
	struct EMS_Sensor s = {.type = type};
	s.settings = (EMS_Option*)malloc(sizeof(EMS_Option) * sensorOptionCount[type]);
	memset(s.settings, 0, sizeof(EMS_Option) * sensorOptionCount[type]);
	s.pins = (EMS_Pin*)malloc(sizeof(struct EMS_Pin) * sensorPinCount[type]);
	memset(s.pins, 0, sizeof(struct EMS_Pin) * sensorPinCount[type]);
	return s;
}

void freeSensor(struct EMS_Sensor* sensor){
	if(sensor->settings){
		free(sensor->settings);
		sensor->settings = NULL;
	}
	if(sensor->pins){
		free(sensor->pins);
		sensor->pins = NULL;
	}
}

EMS_Option setSensorSetting(struct EMS_Sensor* sensor, enum EMS_SETTING setting, EMS_Option option){
	EMS_OptionCount c = sensorOptionCount[sensor->type];
	if(setting > c){
		return EMS_SETTING_NULL;
	}
	EMS_Option old = sensor->settings[setting - 1];
	sensor->settings[setting - 1] = option;
	return old;
}

EMS_Option getSensorSetting(const struct EMS_Sensor* sensor, enum EMS_SETTING setting){
	EMS_OptionCount c = sensorOptionCount[sensor->type];
	if(setting > c){
		return EMS_SETTING_NULL;
	}
	return sensor->settings[setting - 1];
}

EMS_PinValue setSensorPin(struct EMS_Sensor* sensor, enum EMS_PIN pin, EMS_PinValue v, bool isAnalog){
	EMS_PinCount c = sensorPinCount[sensor->type];
	if(pin > c){
		return EMS_PIN_NULL;
	}
	EMS_PinValue old = sensor->pins[pin - 1].pin;
	sensor->pins[pin - 1].pin = v;
	sensor->pins[pin - 1].isAnalog = isAnalog;
	return old;
}

static struct EMS_DataSeries* seriesHead = NULL;

bool registerDataSeries(const char* name, enum EMS_DATA_TYPE type){
	struct EMS_DataSeries** ptr = &seriesHead;
	while(*ptr != NULL){
		if(strcmp((*ptr)->name, name) == 0){
			return false;
		}
		ptr = &(*ptr)->next;
	}
	struct EMS_DataSeries* newSeries = (struct EMS_DataSeries*)malloc(sizeof(struct EMS_DataSeries));
	newSeries->name = name;
	newSeries->type = type;
	newSeries->array = NULL;
	newSeries->next = NULL;
	newSeries->arrayLen = 0;
	*ptr = newSeries;
	return true;
}

bool addDataToSeries(const char* series, void* data, EMS_Time clk){
	struct EMS_DataSeries* s = seriesHead;
	while(s){
		if(strcmp(s->name, series) == 0){
			break;
		}
		s = s->next;
	}
	if(!s){
		return false;
	}
	void* p = realloc(s->array, sizeof(struct EMS_DataPoint) * (++s->arrayLen));
	if(!p){
		puts("uhhhh");
		return false;
	}
	s->array = (struct EMS_DataPoint*)p;
	if(s->type == EMS_DATA_TYPE_INT){
		s->array[s->arrayLen - 1].dataInt = *((EMS_Int*)data);
	}else if(s->type == EMS_DATA_TYPE_FLOAT){
		s->array[s->arrayLen - 1].dataFloat = *((EMS_Float*)data);
	}
	s->array[s->arrayLen - 1].recordedTime = clk;
	return true;
}

bool addDataPointToSeries(const char* series, struct EMS_DataPoint* data){
	struct EMS_DataSeries* s = seriesHead;
	while(s){
		if(strcmp(s->name, series) == 0){
			break;
		}
		s = s->next;
	}
	if(!s){
		return false;
	}
	void* p = realloc(s->array, sizeof(struct EMS_DataPoint) * (++s->arrayLen));
	if(!p){
		puts("uhhhh");
		return false;
	}
	s->array = (struct EMS_DataPoint*)p;
	if(s->type == EMS_DATA_TYPE_INT){
		data->dataInt = data->dataFloat;
	}
	s->array[s->arrayLen - 1] = *data;
	return true;
}

void printAllData(void){
	struct EMS_DataSeries* s = seriesHead;
	while(s){
		puts(s->name);
		if(s->type == EMS_DATA_TYPE_INT){
			//puts("ints");
			for(size_t idx = 0; idx < s->arrayLen; ++idx){
				Serial.printf("%" PRId32 " @ %" PRIu32 "\n", ((struct EMS_DataPoint*)s->array)[idx].dataInt, ((struct EMS_DataPoint*)s->array)[idx].recordedTime);
			}
		}else if(s->type == EMS_DATA_TYPE_FLOAT){
			//puts("floats");
			for(size_t idx = 0; idx < s->arrayLen; ++idx){
				Serial.printf("%f @ %" PRIu32 "\n", ((struct EMS_DataPoint*)s->array)[idx].dataFloat, ((struct EMS_DataPoint*)s->array)[idx].recordedTime);
			}
		}
		s = s->next;
	}
}

static int readPin(const struct EMS_Sensor* s, enum EMS_PIN pin){
	if(s->pins[pin - 1].isAnalog){
		return analogRead(s->pins[pin - 1].pin);
	}
	  return digitalRead(s->pins[pin - 1].pin);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              SENSOR FUNCTIONS
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void CO2sensorread(const struct EMS_Sensor* s, struct EMS_DataPoint* d, enum EMS_READ_MODE){
	int ValorActual = readPin(s, EMS_PIN_CO2_MAIN);
	int ValorAnterior = LOW;
	long tiempoenHIGH, tiempoenLOW, h, l, ppm;
	long tt = millis();
	if(ValorActual == HIGH){
		if(ValorActual != ValorAnterior){
			h = tt;
			tiempoenLOW = h- l;
			ValorAnterior = ValorActual;
		}
	}else{
		if(ValorActual != ValorAnterior){
			l = tt;
			tiempoenHIGH = l - h;
			ValorAnterior = ValorActual;
			ppm = 5000 * (tiempoenHIGH - 2) / (tiempoenHIGH + tiempoenLOW - 4);
		}
	}
	Serial.println("here");
	d->dataInt = ppm;
}

static void TDSsensorread(const struct EMS_Sensor* s, struct EMS_DataPoint* d, enum EMS_READ_MODE){
	int temp = 25;
	TDS.setTemperature(temp);
	TDS.update();
	int tdsValue = TDS.getTdsValue(); // TDS levels in ppm
	d->dataInt = tdsValue;
}

static void COsensorread(const struct EMS_Sensor* s, struct EMS_DataPoint* d, enum EMS_READ_MODE){
	int val = readPin(s, EMS_PIN_CO_MAIN);
	d->dataInt = val;
}

// -------------------------------- MQ-5 sensor functions -----------------------------------
// MQ-5 Sensor Calibrate Function
float calibrateMQ5Sensor() {
   float val = 0;
   for (int i = 0; i < 100; i++) {
     val += getMQ5SensorResistance(analogRead(EMS_PIN_MQ5_MAIN));
     delay(50);
   }
   val = val / 100.0;
   return val / RO_CLEAN_AIR_FACTOR;
}
// Function to get the MQ-5 sensor resistance
float getMQ5SensorResistance(int raw_adc) {
  return ((1023.0 * RL) / raw_adc) - RL;
}

// Function to convert raw MQ-5 sensor value to ppm
float getMQ5PPM(int raw_adc, float Ro) {
  float Rs = getMQ5SensorResistance(raw_adc);
  float ratio = Rs / Ro;
  return 200 * pow(ratio, -1.5);  // Example calculation (adjust based on empirical data)
}
// MQ-5 Sensor Read 
static void MQ5sensorread(const struct EMS_Sensor* s, struct EMS_DataPoint* d, enum EMS_READ_MODE){
  // For MQ-5 sensor: Get raw adc and convert to ppm
  int raw_adc = analogRead(EMS_PIN_MQ5_MAIN);
  float ppm = getMQ5PPM(raw_adc, Ro);
  
	d->dataInt = ppm;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              DATA READING FUNCTIONS
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void (*readArray[])(const struct EMS_Sensor*, struct EMS_DataPoint*, enum EMS_READ_MODE) = {
	[EMS_SENSOR_TYPE_CO2] = &CO2sensorread,
  [EMS_SENSOR_TYPE_TDS] = &TDSsensorread,
	[EMS_SENSOR_TYPE_CO] = &COsensorread,
};

struct EMS_DataPoint readSensor(const struct EMS_Sensor* s){
	struct EMS_DataPoint d = {.recordedTime = ++clk};
	readArray[s->type](s, &d, (enum EMS_READ_MODE)0);
	return d;
}

struct EMS_DataPoint readSensorMode(const struct EMS_Sensor* s, enum EMS_READ_MODE m){
	struct EMS_DataPoint d = {.recordedTime = ++clk};
	readArray[s->type](s, &d, m);
	return d;
}

// SETUP STRUCT FOR SENSOR
struct EMS_Sensor CO2sensor;
struct EMS_Sensor TDSsensor;
struct EMS_Sensor COsensor;
struct EMS_SENSOR MQ5sensor;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              SETUP
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup(){
  	Serial.begin(9600);
  	while(!Serial){
  		Serial.println();
	}

	// CO2
	CO2sensor = createSensor(EMS_SENSOR_TYPE_CO2);
	setSensorPin(&CO2sensor, EMS_PIN_CO2_MAIN, 2, false);
	registerDataSeries("CO2ppm_int", EMS_DATA_TYPE_INT);

	// TDS
	TDSsensor = createSensor(EMS_SENSOR_TYPE_TDS);
	setSensorPin(&TDSsensor, EMS_PIN_TDS_MAIN, A0, true);
	registerDataSeries("TDSppm_int", EMS_DATA_TYPE_INT);
	TDS.setPin(A0);
	TDS.setAref(5.0);
	TDS.setAdcRange(1024);
	TDS.begin();
	
	// CO
	COsensor = createSensor(EMS_SENSOR_TYPE_CO);
	setSensorPin(&COsensor, EMS_PIN_CO_MAIN, A1, true);
	registerDataSeries("COppm_init", EMS_DATA_TYPE_INT);

  // MQ-5
  MQ5sensor = createSensor(EMS_SENSOR_TYPE_MQ5);
  setSensorPin(&MQ5sensor, EMS_PIN_MQ5_MAIN, A0, true);
  registerDataSeries("MQ5ppm_int", EMS_DATA_TYPE_INT);
  Ro = calibrateMQ5Sensor();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              LOOP
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop(){
	static struct EMS_DataPoint data;

	// CO2
	data = readSensor(&CO2sensor);
	addDataPointToSeries("CO2ppm_int", &data);

  // TDS
  data = readSensor(&TDSsensor);
  addDataPointToSeries("TDSppm_int", &data);
	
	// CO
	data = readSensor(&COsensor);
	addDataPointToSeries("COppm_int", &data);

  	// MQ-5
  	data = readSensor(&MQ5sensor):
  	addDataPointToSeries("MQ5ppm_int", &data);
	
	printAllData();
	
	delay(DELTA_MS);
}
