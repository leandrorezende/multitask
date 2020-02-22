float latitude;
float longitude;

int data_size;
char data[512];
char aux;
int x = 0;
int8_t answer;
char aux_str[100];
char indice[2];

const int sensorCalor = 0;
const int sensorAudio = 1;
int valorSensorCalor = 0;
int valorSensorAudio = 0;
const int motor = 12;
const int alarme = 13;


void start_GPS(){
     //Configuracion en Inicializacion GPS
    Serial.print("AT");
    delay(1000);
    Serial.println("AT+CGPSIPR=9600");// (set the baud rate)
    delay(1000);
    Serial.println("AT+CGPSPWR=1"); // （turn on GPS power supply）
    delay(1000);
    Serial.println("AT+CGPSRST=1"); //（reset GPS in autonomy mode）
    delay(10000); //delay para esperar señal del GPS
 }
 
void start_GSM(){
    Serial.println("AT");
    delay(2000);
    Serial.println("AT+CREG?");
    delay(2000);
    Serial.println("AT+SAPBR=3,1,\"APN\",\"zap.vivo.com.br\"");
    delay(2000);
    Serial.println("AT+SAPBR=3,1,\"USER\",\"vivo\"");
    delay(2000);
    Serial.println("AT+SAPBR=3,1,\"PWD\",\"vivo\"");
    delay(2000);
    Serial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    delay(2000);
    Serial.println("AT+SAPBR=1,1");
    delay(10000);
    Serial.println("AT+HTTPINIT");
    delay(2000);
    Serial.println("AT+HTTPPARA=\"CID\",1");
    delay(2000);
 }
 
void setup()
 {
    pinMode(motor, OUTPUT);
    pinMode(alarme, OUTPUT);
   //Init the driver pins for GSM function
    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
   //Output GSM Timing 
    digitalWrite(5,HIGH);
    delay(1500);
    digitalWrite(5,LOW);
    Serial.begin(9600);
        // Use these commands instead of the hardware switch 'UART select' in order to enable each mode
    // If you want to use both GMS and GPS. enable the required one in your code and disable the other one for each access.
    digitalWrite(3,LOW);//enable GSM TX、RX
    digitalWrite(4,HIGH);//disable GPS TX、RX
    
    delay(20000);
    
    start_GPS();
    
    delay(5000);
    
    start_GSM();
 }
 
 void loop(){   
  valorSensorCalor = analogRead(sensorCalor);
  valorSensorAudio = analogRead(sensorAudio);
  
  Serial.print("Valor do valorSensorCalor = ");
  Serial.println(valorSensorCalor);
   
  Serial.print("Valor do valorSensorAudio = ");
  Serial.println(valorSensorAudio);
  
    if (valorSensorCalor > 520)
        indice[0] = '1'; 
    else
        indice[0] = '0'; 
    if (valorSensorAudio > 480)
        indice[1] = '1';
    else
        indice[1] = '0'; 

    read_GPS();
    delay(5000);
    send_GPRS();
 }

 void send_GPRS(){
    digitalWrite(3,LOW);//Enable GSM mode
    digitalWrite(4,HIGH);//Disable GPS mode
    
    Serial.print("AT+HTTPPARA=\"URL\",\"labsoft.muz.ifsuldeminas.edu.br:8080/SRSVws/ocorrencia/cadastrarOcorrencia/");
    Serial.print(-latitude,5);
    Serial.print("/");
    Serial.print(-longitude,5);
    Serial.print("/");
    Serial.print("HHHHHHHHHH");
    Serial.print("/");
    Serial.print(indice[0]);
    Serial.print(indice[1]);
    Serial.println("\"");
    delay(2000);
    Serial.println("AT+HTTPACTION=0"); //now GET action 
    delay(5000);
    
    x=0;
    do{
        sprintf(aux_str, "AT+HTTPREAD=%d,100", x);
        if (sendATcommand2(aux_str, "+HTTPREAD:", "ERROR", 5000) == 1)
        {
            data_size = 0;
            while(Serial.available()==0);
            aux = Serial.read();
            do{
                data_size *= 10;
                data_size += (aux-0x30);
                while(Serial.available()==0);
                aux = Serial.read();        
            }while(aux != 0x0D);

            Serial.print("Data received: ");
            Serial.println(data_size);
            
            if (data_size > 0)
            {
              digitalWrite(alarme, HIGH);
              delay(500);
              digitalWrite(alarme, LOW);
                while(Serial.available() < data_size);
                Serial.read();

                for (int y = 0; y < data_size; y++)
                {
                    data[x] = Serial.read();
                    x++;
                }
                data[x] = '\0';
            }
            else
            {
                Serial.println("Download finished");    
            }
        }
        else if (answer == 2)
        {
            Serial.println("Error from HTTP");
        }
        else
        {
            Serial.println("No more data available");
            data_size = 0;
        }
        
        //sendATcommand("", "+HTTPACTION:0,200", 5000);
    }while (data_size > 0);    
    Serial.print("Data received: ");
    Serial.println(data);   
      
   if(data[1] == '1')
     digitalWrite(motor, HIGH);
   else 
     digitalWrite(motor, LOW);

   if(data[2] == '1')
     digitalWrite(alarme, HIGH);
   else 
     digitalWrite(alarme, LOW);
}
 void read_GPS(){
    //Serial.println("AT+CGPSINF=0");
    digitalWrite(4,LOW);//Enable GPS mode
    digitalWrite(3,HIGH);//Disable GSM mode
    latitude = getLatitude();
    longitude = getLongitude();
 }
 
 double Datatransfer(char *data_buf,char num){ //convert the data to the float type
  //*data_buf：the data array 
  double temp=0.0; //the number of the right of a decimal point
  unsigned char i,j;
  
  if(data_buf[0]=='-') {
    i=1;
    //process the data array
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    //convert the int type to the float type
    for(j=0;j<num;j++)
      temp=temp/10;
      //convert to the negative numbe
    temp=0-temp;
  }
  else{ //for the positive number
    i=0;
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    for(j=0;j<num;j++)
      temp=temp/10 ;
  }
  return temp;
}

char ID(){ //Match the ID commands
  char i=0;
  char value[6]={
    '$','G','P','G','G','A' };//match the gps protocol
  char val[6]={
    '0','0','0','0','0','0' };

  while(1){
    if(Serial.available()){
      val[i] = Serial.read();//get the data from the serial interface
      if(val[i]==value[i]){ //Match the protocol
        i++;
        if(i==6){
          i=0;
          return 1;//break out after get the command
        }
      }
      else
        i=0;
    }
  } 
}

void comma(char num){ //get ','
  char val;
  char count=0;//count the number of ','

  while(1){
    if(Serial.available()){
      val = Serial.read();
      if(val==',')
        count++;
    }
    if(count==num)//if the command is right, run return
      return;
  }
}

double decimalgps(double rawdata){
  int degrees = (int)(rawdata / 100);
  double minutes = rawdata - (degrees*100);
  double mindecimal = minutes / 60.0;
  double total = degrees + mindecimal;
  return total;
}

float getLatitude(){ //get latitude
  char i;
  char lat[10]={
    '0','0','0','0','0','0','0','0','0','0'
  };
  if( ID()){
    comma(2);
    while(1){
      if(Serial.available()){
        lat[i] = Serial.read();
        i++;
      }
      if(i==10){
        i=0;
        double newlat = Datatransfer(lat,5);
        float corrected = decimalgps(newlat);
        return corrected;
      } 
    }
  }
}

float getLongitude(){ //get longitude
  char i;
  char lon[11]={
    '0','0','0','0','0','0','0','0','0','0','0'
  };

  if( ID()){
    comma(4);
    while(1){
      if(Serial.available()){
        lon[i] = Serial.read();
        i++;
      }
      if(i==11){
        i=0;
        double newlon = Datatransfer(lon,5);
        float corrected = decimalgps(newlon);
        return corrected;
      } 
    }
  }
}

int8_t sendATcommand(char* ATcommand, char* expected_answer1, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial.available() > 0) Serial.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 


        x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(Serial.available() != 0){    
            response[x] = Serial.read();
            x++;
            // check if the desired answer is in the response of the module
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
        }
        // Waits for the asnwer with time out
    }
    while((answer == 0) && ((millis() - previous) < timeout));    

    return answer;
}

int8_t sendATcommand2(char* ATcommand, char* expected_answer1, 
char* expected_answer2, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial.available() > 0) Serial.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 


        x = 0;
    previous = millis();

    // this loop waits for the answer
    do{        
        if(Serial.available() != 0){    
            response[x] = Serial.read();
            x++;
            // check if the desired answer 1 is in the response of the module
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
        }
        // Waits for the asnwer with time out
    }while((answer == 0) && ((millis() - previous) < timeout));    

    return answer;
}
