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

static EMS_Time clock = 0;

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

enum EMS_SENSOR_TYPE{
	EMS_SENSOR_TYPE_CO2,
	EMS_SENSOR_TYPE_TDS,
	EMS_SENSOR_TYPE_CO,
};

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
};

enum EMS_READ_MODE{
	// EMS_READ_WATER_O2 = 0,
	// EMS_READ_WATER_N2,
};

EMS_OptionCount sensorOptionCount[] = {
	[EMS_SENSOR_TYPE_CO2] = 0,
	[EMS_SENSOR_TYPE_TDS] = 0,
	[EMS_SENSOR_TYPE_CO] = 0,
};

EMS_PinCount sensorPinCount[] = {
	[EMS_SENSOR_TYPE_CO2] = 1,
	[EMS_SENSOR_TYPE_TDS] = 1,
	[EMS_SENSOR_TYPE_CO] = 1,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              MAIN FUNCTIONS
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct EMS_Sensor createSensor(enum EMS_SENSOR_TYPE type){
	struct EMS_Sensor s = {.type = type};
	s.settings = malloc(sizeof(EMS_Option) * sensorOptionCount[type]);
	memset(s.settings, 0, sizeof(EMS_Option) * sensorOptionCount[type]);
	s.pins = malloc(sizeof(struct EMS_Pin) * sensorPinCount[type]);
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

EMS_PinValue setSensorPin(struct EMS_Sensor* sensor, enum EMS_PIN pin, EMS_PinValue v){
	EMS_PinCount c = sensorPinCount[sensor->type];
	if(pin > c){
		return EMS_PIN_NULL;
	}
	EMS_PinValue old = sensor->pins[pin - 1].pin;
	sensor->pins[pin - 1].pin = v;
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
	struct EMS_DataSeries* newSeries = malloc(sizeof(struct EMS_DataSeries));
	newSeries->name = name;
	newSeries->type = type;
	newSeries->array = malloc(0);
	newSeries->next = NULL;
	newSeries->arrayLen = 0;
	*ptr = newSeries;
	return true;
}

bool addDataToSeries(const char* series, void* data, EMS_Time clock){
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
	s->array = p;
	if(s->type == EMS_DATA_TYPE_INT){
		s->array[s->arrayLen - 1].dataInt = *((EMS_Int*)data);
	}else if(s->type == EMS_DATA_TYPE_FLOAT){
		s->array[s->arrayLen - 1].dataFloat = *((EMS_Float*)data);
	}
	s->array[s->arrayLen - 1].recordedTime = clock;
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
	s->array = p;
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
			puts("ints");
			for(size_t idx = 0; idx < s->arrayLen; ++idx){
				printf("%" PRId32 " @ %" PRIu32 "\n", ((struct EMS_DataPoint*)s->array)[idx].dataInt, ((struct EMS_DataPoint*)s->array)[idx].recordedTime);
			}
		}else if(s->type == EMS_DATA_TYPE_FLOAT){
			puts("floats");
			for(size_t idx = 0; idx < s->arrayLen; ++idx){
				printf("%f @ %" PRIu32 "\n", ((struct EMS_DataPoint*)s->array)[idx].dataFloat, ((struct EMS_DataPoint*)s->array)[idx].recordedTime);
			}
		}
		s = s->next;
	}
}

static int readPin(const struct EMS_Sensor* s, enum EMS_PIN pin){
	if(s->pins[pin - 1].isAnalog){
		return analogRead(s->pins[pin - 1].pin);
	}
	return digitalRead(s->pins[pin - 1.pin]);
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
	}
	else{
		if(ValorActual != ValorAnterior){
			l = tt;
			tiempoenHIGH = l - h;
			ValorAnterior = ValorActual;
			ppm = 5000 * (tiempoenHIGH - 2) / (tiempoenHIGH + tiempoenLOW - 4);
		}
	}
	d->dataInt = ppm;
}

static void COsensorread(const struct EMS_Sensor* s, struct EMS_DataPoint* d, enum EMS_READ_MODE){
	int val = readPin(s, EMS_PIN_CO_MAIN);
	d->dataInt = val;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              SENSOR READING FUNCTIONS
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void (*readArray[])(const struct EMS_Sensor*, struct EMS_DataPoint*, enum EMS_READ_MODE) = {
	[EMS_SENSOR_TYPE_CO2] = &CO2sensorread,
	[EMS_SENSOR_TYPE_CO] = &COsensorread,
};

struct EMS_DataPoint readSensor(const struct EMS_Sensor* s){
	struct EMS_DataPoint d = {.recordedTime = ++clock};
	readArray[s->type](s, &d, 0);
	return d;
}

struct EMS_DataPoint readSensorMode(const struct EMS_Sensor* s, enum EMS_READ_MODE m){
	struct EMS_DataPoint d = {.recordedTime = ++clock};
	readArray[s->type](s, &d, m);
	return d;
}

// SETUP STRUCT FOR SENSOR
struct EMS_Sensor CO2sensor;
struct EMS_Sensor TDSsensor;
struct EMS_Sensor COsensor;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              SETUP
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup(){
	// create sensor
	CO2sensor = createSensor(EMS_SENSOR_TYPE_CO2);
	TDSsensor = createSensor(EMS_SENSOR_TYPE_TDS);
	COsensor = createSensor(EMS_SENSOR_TYPE_CO);

	// set sensor pin
	setSensorPin(&CO2sensor, EMS_PIN_CO2_MAIN, 2);
	setSensorPin(&TDSsensor, EMS_PIN_TDS_MAIN, A0);
	setSensorPin(&COsensor, EMS_PIN_CO_MAIN, A1);

	// set data series
	registerDataSeries("CO2ppm_int", EMS_DATA_TYPE_INT);
	registerDataSeries("TDSppm_int", EMS_DATA_TYPE_INT);
	registerDataSeries("COppm_init", EMS_DATA_TYPE_INT);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//              LOOP
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop(){
	static struct EMS_DataPoint data;
	data = readSensor(&CO2sensor);
	addDataPointToSeries("CO2ppm_int", &data);
	data = readSensor(&TDSsensor);
	addDataPointToSeries("TDSppm_int", &data);
	data = readSensor(&COsensor);
	addDataPointToSeries("COppm_int", &data);
	delay(DELTA_MS);
}
