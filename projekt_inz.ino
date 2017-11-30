#include <Math.h>
#define DEVICE_MAX_NUM 14
#define SLEEP_MS_STEP 10

struct device
{
   bool initialized;
   bool pwm;
   
   int pin1;
   int pin2;
   int pin3;
   
   double p1_val;
   double p2_val;
   double p3_val;
   
   int p1_targetVal;
   int p2_targetVal;
   int p3_targetVal;

   bool fade;
   double p1_adder;
   double p2_adder;
   double p3_adder;
};

typedef struct device Device;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  for(int pin = 2; pin < 14; pin++) {
    pinMode(pin, OUTPUT);
  }

  for(int pin = 22; pin < 54; pin++) {
    pinMode(pin, OUTPUT);
  }
}

String input;
Device devices[DEVICE_MAX_NUM];

void loop() {
  if (Serial.available() > 0) {
    input = Serial.readString();
    serialFunc(input);
  }
  checkDevices();
  delay(SLEEP_MS_STEP);
}

void serialFunc(String input)
{
  String funcName = getValue(input, '(', 0);
  String parameters = getValue(getValue(input, '(', 1), ')', 0);
  
  if(funcName == "deviceInit") {
    deviceInit(parameters);
  } else if(funcName == "led") {
    setLedRGB(parameters);
  } else if(funcName == "ledTime") {
    setLedRGBtime(parameters);
  } else if(funcName == "enable") {
    enableDevice(parameters);
  } else if(funcName == "showDevice") {
    showDevice(parameters);
  }
}

void checkDevices() {
  for(int i = 0; i < DEVICE_MAX_NUM; i++) {
    if(devices[i].initialized) {
      if(devices[i].fade == true) {
        if(devices[i].p1_val < (devices[i].p1_targetVal - 1) || devices[i].p1_val > (devices[i].p1_targetVal + 1))
          devices[i].p1_val = devices[i].p1_val + devices[i].p1_adder;
        if(devices[i].p2_val < (devices[i].p2_targetVal - 1) || devices[i].p2_val > (devices[i].p2_targetVal + 1))
          devices[i].p2_val = devices[i].p2_val + devices[i].p2_adder;
        if(devices[i].p3_val < (devices[i].p3_targetVal - 1) || devices[i].p3_val > (devices[i].p3_targetVal + 1))
          devices[i].p3_val = devices[i].p3_val + devices[i].p3_adder;
      }
      analogWrite(devices[i].pin1, devices[i].p1_val);
      analogWrite(devices[i].pin2, devices[i].p2_val);
      analogWrite(devices[i].pin3, devices[i].p3_val);
    }
  }
}

//int id, String r:g:b, int time
void setLedRGBtime(String param) {
  String devIdStr = getValue(param,',',0);
  String colorP = getValue(param,',',1);
  String timeP = getValue(param,',',2);

  int deviceId = devIdStr.toInt();
  double transTime = (double)(timeP.toInt() < SLEEP_MS_STEP ? SLEEP_MS_STEP : timeP.toInt());
  
  int rt = splitParams(colorP,':',0);
  int gt = splitParams(colorP,':',1);
  int bt = splitParams(colorP,':',2);

  double r_adder = ((double)colorDiff((int)devices[deviceId].p1_val, rt) / transTime) * SLEEP_MS_STEP;
  double g_adder = ((double)colorDiff((int)devices[deviceId].p2_val, gt) / transTime) * SLEEP_MS_STEP;
  double b_adder = ((double)colorDiff((int)devices[deviceId].p3_val, bt) / transTime) * SLEEP_MS_STEP;

  devices[deviceId].p1_targetVal = rt;
  devices[deviceId].p2_targetVal = gt;
  devices[deviceId].p3_targetVal = bt;

  devices[deviceId].p1_adder = r_adder;
  devices[deviceId].p2_adder = g_adder;
  devices[deviceId].p3_adder = b_adder;

  devices[deviceId].fade = true;

  printDevice(deviceId);
}

//int id, String r:g:b
void setLedRGB(String param) {
  int deviceId = splitParams(param, ',', 0);
  param = getValue(param, ',', 1);
  
  devices[deviceId].p1_val = splitParams(param,':',0);
  devices[deviceId].p2_val = splitParams(param,':',1);
  devices[deviceId].p3_val = splitParams(param,':',2);

  devices[deviceId].p1_targetVal = devices[deviceId].p1_val;
  devices[deviceId].p2_targetVal = devices[deviceId].p2_val;
  devices[deviceId].p3_targetVal = devices[deviceId].p3_val;

  devices[deviceId].p1_adder = 0;
  devices[deviceId].p2_adder = 0;
  devices[deviceId].p3_adder = 0;

  devices[deviceId].fade = false;
   
  analogWrite(devices[deviceId].pin1, devices[deviceId].p1_val);
  analogWrite(devices[deviceId].pin2, devices[deviceId].p2_val);
  analogWrite(devices[deviceId].pin3, devices[deviceId].p3_val);

  printDevice(deviceId);
}

//int id, bool pwm, int p1, int p2, int p3
void deviceInit(String params) {
  int deviceId = splitParams(params, ',', 0);
  devices[deviceId].initialized = true;
  devices[deviceId].pwm = (getValue(params, ',', 1) == "true") ? true : false;
  devices[deviceId].pin1 = splitParams(params, ',', 2);
  devices[deviceId].pin2 = splitParams(params, ',', 3);
  devices[deviceId].pin3 = splitParams(params, ',', 4);
  
  printDevice(deviceId);
}

//int id, bool status
void enableDevice(String params) {
  int deviceId = splitParams(params, ',', 0);
  bool stat = (getValue(params, ',', 1) == "true") ? true : false;

  if(stat) {
    devices[deviceId].p1_val = 1;
    devices[deviceId].p2_val = 1;
    devices[deviceId].p3_val = 1;
    devices[deviceId].p1_targetVal = devices[deviceId].p1_val;
    devices[deviceId].p2_targetVal = devices[deviceId].p2_val;
    devices[deviceId].p3_targetVal = devices[deviceId].p3_val;
    digitalWrite(devices[deviceId].pin1, HIGH);
    digitalWrite(devices[deviceId].pin2, HIGH);
    digitalWrite(devices[deviceId].pin3, HIGH);
  } else {
    devices[deviceId].p1_val = 0;
    devices[deviceId].p2_val = 0;
    devices[deviceId].p3_val = 0;
    devices[deviceId].p1_targetVal = devices[deviceId].p1_val;
    devices[deviceId].p2_targetVal = devices[deviceId].p2_val;
    devices[deviceId].p3_targetVal = devices[deviceId].p3_val;
    digitalWrite(devices[deviceId].pin1, LOW);
    digitalWrite(devices[deviceId].pin2, LOW);
    digitalWrite(devices[deviceId].pin3, LOW);
  }

  printDevice(deviceId);
}

//int id
void showDevice(String params) {
  int deviceId = splitParams(params, ',', 0);
  printDevice(deviceId);
}

void printDevice(int id) {
  String inited = (devices[id].initialized) ? "true" : "false";
  
  Serial.print("DEVICE Id: " + String(id) + "\n");
  Serial.print("Inited: " + inited + "\n\n");

  Serial.print("Pin1: " + String(devices[id].pin1) + "\n");
  Serial.print("Pin2: " + String(devices[id].pin2) + "\n");
  Serial.print("Pin3: " + String(devices[id].pin3) + "\n\n");
  
  Serial.print("Pin1_val: " + String(devices[id].p1_val) + "\n");
  Serial.print("Pin2_val: " + String(devices[id].p2_val) + "\n");
  Serial.print("Pin3_val: " + String(devices[id].p3_val) + "\n\n");
  
  Serial.print("Pin1_valTarget: " + String(devices[id].p1_targetVal) + "\n");
  Serial.print("Pin2_valTarget: " + String(devices[id].p2_targetVal) + "\n");
  Serial.print("Pin3_valTarget: " + String(devices[id].p3_targetVal) + "\n\n");

  Serial.print("p1_adder: " + String(devices[id].p1_adder) + "\n");
  Serial.print("p2_adder: " + String(devices[id].p2_adder) + "\n");
  Serial.print("p3_adder: " + String(devices[id].p3_adder) + "\n");
}

int colorDiff(int c, int tc){
  return tc - c;
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int splitParams(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]).toInt() : 0;
}
