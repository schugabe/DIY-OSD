#define __STDC_LIMIT_MACROS 1
#include <stdint.h>
#include "gps.h"
#include "config.h"

#include "Arduino.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

#include <math.h>

unsigned char Buttonpin = Buttonpin_;

// Buffer for GPS string
char GPSbuffer[100]={0};

int iiii =0;


//==================================
// Decleration for GPS-variables
//==================================

// Has homeposition been set?
char homepos=0;
// Simple way to make a little delay before the homeposition is set. (It waits for GPS-fix, waits a couple of extra seconds and set homeposition)
int homeposcount=0;

// GPSfix - when '0' no satellite fix.
char GPSfix='0';

// Used to store all the GPS positions etc...
long lat1=0;
long lat2=0;
long lats=0;

long lon1=0;
long lon2=0;
long lons=0;

// Used to calculate line of sight.
long losy=0;
long losx=0;

// Variables to store home position:
long lathome = 0;
long lonhome = 0;
long los = 0;

// Direction home:
int homehead=0;
int homehead_r[3]={96,96,96};
int headi=0;
int arrow=0;
int arrowd;
unsigned char satellites[3]={3,3,3};
unsigned char satellitesr[3]={3,3,3};

unsigned char latitude_dir =0;
unsigned char longitude_dir =0;

float longitude_factor =1;
int lat_deg = 90;


// RSSI
#if (digital_rssi==1)
typedef enum { falling, rising } pwm_state_t;
static uint16_t rising_ticks;
static pwm_state_t state = rising;
static uint8_t current_min = 100;
static uint8_t rssi_measurements_count = 0;
unsigned long last_rssi_measurement = 0;

#define MAX_DURATION  (rssi_max-rssi_min)
#define SET_RISING()  TCCR1B |=  (1<<ICES1); TCNT1 = 0;	state = rising; TIFR1 = _BV(ICF1) | _BV(TOV1)
#define SET_FALLING() TCCR1B &= ~(1<<ICES1); state = falling; TIFR1 = _BV(ICF1) | _BV(TOV1)

extern volatile int rssi_reading;
extern volatile uint8_t rssi_read;
#endif

#if (USE_GPS==0)
extern volatile uint8_t software_timer;
#endif
//========================================
// Menu system
//========================================

/*
int menuon = 0;
unsigned char menupos = 0;
unsigned char move_arrow_count =0;

unsigned char show_mah_km = show_mah_km_;
unsigned char show_decimals = show_decimals_;
unsigned char altitude_offset_on = altitude_offset_on_;
unsigned char align_text = align_text_;
unsigned char show_plane_pos = show_plane_pos_;
*/

//unsigned char Buttonpin = Buttonpin_;

int menuon = 0;
unsigned char menupos = 0;
char move_arrow_count =0;
unsigned char menu = 1;
  
unsigned char show_mah_km = EEPROM.read(1);
unsigned char show_decimals = EEPROM.read(2);
unsigned char altitude_offset_on = EEPROM.read(3);
unsigned char align_text = EEPROM.read(4);
unsigned char show_plane_pos = EEPROM.read(5);

int alt_alarm = EEPROM.read(10)+(EEPROM.read(11)<<8);
int los_alarm = EEPROM.read(12)+(EEPROM.read(13)<<8);
int volt_alarm =EEPROM.read(14)+(EEPROM.read(15)<<8);
int mah_alarm = EEPROM.read(16)+(EEPROM.read(17)<<8);

//========================================
// Variables when analyzing serial data (GPS)
//========================================

// For GPS/Serial read
// bufnr counts the number of characters received. N used to when extracting 
// data from GPS strings.
int bufnr=0; // Bytes are stored in an array. bufnr "counts the characters" 
int n=0;
int count =0; 
 
//========================================
// Data read from GPS
//========================================
unsigned char time[14]={1,1,1,1,1,1,1,1,1,1,1,1};;
unsigned char speedkn[10]={1,1,1,1,1};
unsigned char speedkn2[10]={1,1,1,1,1};
unsigned char speedkmw[5]={1,1,1,1,1};
unsigned int speedkm =0;
unsigned int speedkm2 =0;
int latitude[15]={0};
int longitude[15]={0};
int head[10]={0};
unsigned char altitude[10]={1,1,1,1,1,1};
unsigned char altitude2[10]={1,1,1,1,1,1}; 
unsigned char altituder[10]={1,1,1,1,1,1,1,1};
unsigned char altituder2[10]={1,1,1,1,1,1,1,1};
long altitude_offset =0;
int altitude_negative=0; 
 
//========================================
// For flight summary
//========================================
long max_los =0;
unsigned char max_losr[] = {3,3,3,3};

int max_speed =0;
unsigned char max_speedr[] = {3,3,3,3};

long kmh_total =0;
unsigned char total_distancer[] = {3,3,3,3,3};

long max_alt=0;
unsigned char max_altr[] = {3,3,3,3,3,3};

#if (USE_GPS==1)
unsigned char landed = 1;
#else
unsigned char landed = 0;
#endif
long altitude_num=0;
long altitude_num2=0;
int altitude_int=0;

unsigned long flight_time =0;
unsigned char last_time=0;
unsigned char flight_timer[] = {3,3,3,3,3,3};

int avg_speed =0;
unsigned char avg_speedr[] = {3,3,3,3,3,3};

unsigned char loadbar[]={29,27,27,27,27,27,27,27,27,28};

unsigned char test=0;


unsigned char losr[]={1,1,1,1};     // Stores LOS characters (numbers) written to screen
unsigned long long_buf=0;

unsigned char arrowr[] ={3,3,3};
int current_num=0;

unsigned int mahkm =0;
unsigned char mahkmr[]={3,3,3,3,3,3};

void gps() {
#if (Usebutton == 1)
	if (digitalRead(Buttonpin)==LOW) {
    
		GPSbuffer[0]=((alt_alarm/1000)+3)<<3;
		GPSbuffer[1]=((alt_alarm%1000/100)+3)<<3;
		GPSbuffer[2]=((alt_alarm%1000%100/10)+3)<<3;
		GPSbuffer[3]=((alt_alarm%1000%100%10)+3)<<3;                  

		GPSbuffer[4]=((los_alarm/1000)+3)<<3;
		GPSbuffer[5]=((los_alarm%1000/100)+3)<<3;
		GPSbuffer[6]=((los_alarm%1000%100/10)+3)<<3;
		GPSbuffer[7]=((los_alarm%1000%100%10)+3)<<3;       

		GPSbuffer[8]=((volt_alarm/1000)+3)<<3;
		GPSbuffer[9]=((volt_alarm%1000/100)+3)<<3;
		GPSbuffer[10]=((volt_alarm%1000%100/10)+3)<<3;
		GPSbuffer[11]=((volt_alarm%1000%100%10)+3)<<3;            

		GPSbuffer[12]=((mah_alarm/1000)+3)<<3;
		GPSbuffer[13]=((mah_alarm%1000/100)+3)<<3;
		GPSbuffer[14]=((mah_alarm%1000%100/10)+3)<<3;
		GPSbuffer[15]=((mah_alarm%1000%100%10)+3)<<3;    
    
		menuon=1; 
  
		while (menuon==1) {
			if (digitalRead(Buttonpin)==LOW) {  
				move_arrow_count=-25;
				while(digitalRead(Buttonpin)==LOW) {
					if (move_arrow_count > -15) {
						break; 
					}
				}
      
				if (menupos == 7 && menu == 1) {
					menuon=0; 

					// Show_mah_km
					EEPROM.write(1,show_mah_km);
  
					// Show decimals
					EEPROM.write(2,show_decimals);
  
					// Altitude offset
					EEPROM.write(3,altitude_offset_on);
  
					// Align text
					EEPROM.write(4,align_text);
  
					// Show plane pos
					EEPROM.write(5,show_plane_pos);  
  
					// Alt alarm
					EEPROM.write(10,(unsigned char) alt_alarm);
					EEPROM.write(11,(unsigned char) (alt_alarm>>8));  
  
					EEPROM.write(12,(unsigned char) los_alarm);
					EEPROM.write(13,(unsigned char) (los_alarm>>8));  

					EEPROM.write(14,(unsigned char) volt_alarm);
					EEPROM.write(15,(unsigned char) (volt_alarm>>8));  

					EEPROM.write(16,(unsigned char) mah_alarm);
					EEPROM.write(17,(unsigned char) (mah_alarm>>8));     

  
				}
      
				if (menupos == 6 && menu == 1) {
					menu=2; 
        
				}
      
				else if (menupos == 1 && menu == 1) {
					if (show_mah_km== 1) {
						show_mah_km =0;
					}
					else {
						show_mah_km =1;
					} 
          
				}
        

				else if (menupos == 2 && menu == 1) {
					if (show_decimals== 1) {
						show_decimals =0;
					}
					else {
						show_decimals =1;
					} 
          
				}



				else if (menupos == 3 && menu == 1) {
					if (altitude_offset_on == 1) {
						altitude_offset_on  =0;
					}
					else {
						altitude_offset_on =1;
					} 
          
				}


				else if (menupos == 4 && menu == 1) {
					align_text++;
        
					if (align_text > 40) {
						align_text = 2; 
					}
          
				}
        

				else if (menupos == 5 && menu == 1) {
					if (show_plane_pos == 1) {
						show_plane_pos  =0;
					}
					else {
						show_plane_pos =1;
					} 
          
				}
        
        
        
        
        
				else if (menupos == 1 && menu == 2) {
					alt_alarm=alt_alarm+50;
            
					if (alt_alarm > 2500) {
              
						alt_alarm=0; 
					}
            
					GPSbuffer[0]=((alt_alarm/1000)+3)<<3;
					GPSbuffer[1]=((alt_alarm%1000/100)+3)<<3;
					GPSbuffer[2]=((alt_alarm%1000%100/10)+3)<<3;
					GPSbuffer[3]=((alt_alarm%1000%100%10)+3)<<3;                    
          
				}        
        
        
				else if (menupos == 2 && menu == 2) {
					los_alarm=los_alarm+50;
            
					if (los_alarm > 2500) {
              
						los_alarm=0; 
					}
					GPSbuffer[4]=((los_alarm/1000)+3)<<3;
					GPSbuffer[5]=((los_alarm%1000/100)+3)<<3;
					GPSbuffer[6]=((los_alarm%1000%100/10)+3)<<3;
					GPSbuffer[7]=((los_alarm%1000%100%10)+3)<<3;                    
          
				}         
        
        
				else if (menupos == 3 && menu == 2) {
					volt_alarm=volt_alarm+1;
            
					if (volt_alarm > 200) {
              
						volt_alarm=0; 
					}
					GPSbuffer[8]=((volt_alarm/1000)+3)<<3;
					GPSbuffer[9]=((volt_alarm%1000/100)+3)<<3;
					GPSbuffer[10]=((volt_alarm%1000%100/10)+3)<<3;
					GPSbuffer[11]=((volt_alarm%1000%100%10)+3)<<3;                     
          
				}    

				else if (menupos == 4 && menu == 2) {
					mah_alarm=mah_alarm+100;
            
					if (mah_alarm > 5000) {
              
						mah_alarm=500; 
					}
					GPSbuffer[12]=((mah_alarm/1000)+3)<<3;
					GPSbuffer[13]=((mah_alarm%1000/100)+3)<<3;
					GPSbuffer[14]=((mah_alarm%1000%100/10)+3)<<3;
					GPSbuffer[15]=((mah_alarm%1000%100%10)+3)<<3;                
          
				}            
        
				else if (menupos == 5 && menu == 2) {
					menu=1; 
				}        
			}
		}
	}
}
#endif

SET_RISING();
rising_ticks = 0;
    
while (1) {
	SPDR =0b00000000;
	
#if (USE_GPS==1)
	if (UCSR0A & _BV(RXC0)) {
		GPSbuffer[bufnr] = UDR0;

		bufnr++;
        
		// This code is not very efficient, but as it will only be executed 1-10 
		// times a second (1-10 hz update rate from GPS) it's not as critical as the video-timing.
     
		// The major problem by doing it this way is, that the text-string has to be
		// "analyzed" before the next character is received.  
    
		// If the last character is 13 = carriage return, an entire line has been received.
		if (GPSbuffer[bufnr-1] == 13){
                 
			// Check if it's the GPRMC-string that has been received;
			// If it's the GPRMC-string, the characher at index 4 = R
      
      
			// ====================================================================================
			// Second GPS String      
			// ====================================================================================
      
			if (GPSbuffer[4]=='R') {
        
				// Resets the speed to avoid "hanging numbers". 
				speedkn[0]=14;
				speedkn[1]=14;
				speedkn[2]=14;
				speedkn[3]=14;
				speedkn[4]=14;        

				// Goes through every character in the line. As the GPS-data is ","-seperated
				// we can look for the "," and find the data needed. 
				for (int i = 0; i<bufnr; i++) {
					if (GPSbuffer[i]==',') {
						n=0;
						count++;
					}
        
					else {
						switch (count) {
                
							case 0: break;
              
							case 1: 
							time[n]=GPSbuffer[i]-45;
							break; 
              
							case 2: break;
              
							case 3: 
							latitude[n]=GPSbuffer[i];
							break; 
              
							case 4:
							latitude_dir=GPSbuffer[i];
							break;
              
							case 5:
							longitude[n]=GPSbuffer[i];
							break; 
              
							case 6: 
							longitude_dir=GPSbuffer[i];
							break;
              
							case 7:
							speedkn[n]=GPSbuffer[i]-45;
							break; 
              
							case 8:
							head[n]=GPSbuffer[i];
							break;
              
							default: break;
              
						}
						// Don't need more data efter the 9. "," - so we will just break.
						if (count == 9) {
							break; 
						}
            
						n++;
					}
				}
      
				speedkn2[0]=speedkn[0];
				speedkn2[1]=speedkn[1];
				speedkn2[2]=speedkn[2];
				speedkn2[3]=speedkn[3];
				speedkn2[4]=speedkn[4];
              
  
				// So, when this last string has been received we got "plenty of time" to do some math....
				// okay, at 9600 baud rate the 2 strings takes around 80 % of the bandwidth (5 hz update rate),
				// but at least it gives us around 0,04 seconds. 
      
				// If you need the micro-controller for further calculations you might want to
				// improve this code a bit. 
      


				//===================================================      
				// GPS coordinates
				//===================================================   
				// Coordinates from MKT is like; 5559.5395
				// 55 as degrees and 59 as minutes (or something that ends with 60 ;)) and 5395 as "normal number/decimal system"

				lat1 = (latitude[8]-48)+(latitude[7]-48)*10+(latitude[6]-48)*100+(latitude[5]-48)*1000;
				lat2 = (latitude[3]-48)+(latitude[2]-48)*10+(latitude[1]-48)*60+(latitude[0]-48)*600;
				lats = lat1+(lat2*10000);
				lon1 = (longitude[9]-48)+(longitude[8]-48)*10+(longitude[7]-48)*100+(longitude[6]-48)*1000;
				lon2 = (longitude[4]-48)+(longitude[3]-48)*10+(longitude[2]-48)*60+(longitude[1]-48)*600+(longitude[0]-48)*6000;
				lons = lon1+(lon2*10000);
      
				//===================================================      
				// End of GPS coordinates
				//===================================================

				//===================================================      
				// Calculate LOS and heading
				//===================================================  
				if (homepos ==1) {
					// Calculate the distance in y-direction. 1 nautical mile = 1.85 km.
					losy = ((lathome - lats)*185)/1000;
					losx = ((lonhome - lons)*185*longitude_factor)/1000;
					los=sqrt((losy*losy)+(losx*losx));
      
					// Resets LOS to avoid "hanging numbers"
					losr[0]=14;
					losr[1]=14;
					losr[2]=14;
					losr[3]=14;      
      
					// If units is defined as US.
					if (units== 0) {
						los = los * 3.28; 
					}
      
					losr[0]=(  los/1000)+3;
					losr[1]=(( los%1000)/100)+3;      
					losr[2]=(((los%1000)%100)/10)+3; 
					losr[3]=(((los%1000)%100)%10)+3;
      
					// Calculate heading home.
      
					// This piece of code could probably be more efficient, but for now it gets the job done.  
					if (latitude_dir == 'S') {
						losy=losy*(-1);
					}
      
					if (longitude_dir == 'W') {
						losx=losx*(-1);
					}      

					if (losy > 0) {
						if (losx > 0) {
							// Arrow right up
							// (57 = from radians to degrees)
							homehead = 90-asin(losy/(double(los)))*57;
						}
						if (losx <= 0) {
							// Arrow left up
							homehead = 270+asin(losy/(double(los)))*57;
						}
					}
          
					if (losy <= 0) {
						if (losx > 0) {
							// Arrow right down
							homehead = 90+acos(losx/(double(los)))*57;
						}
						if (losx <= 0) {
							// Arrow left down
							homehead = 270-acos(abs(losx)/(double(los)))*57;
						}
					}       
                    
					if (show_plane_pos == 1) {
						homehead_r[0] = ((homehead/100)+3)<<5;
						homehead_r[1] = ((homehead%100/10)+3)<<5;
						homehead_r[2] = ((homehead%100%10)+3)<<5;
					}
						
					//Last part.. Need to calculate which way 
					// the arrow needs to point. 
      
					// First we need to convert the heading to an integer
					headi=0;
					for (int iii=0;iii<3;iii++) {
						if (head[iii]<48) {
							break;
						}
						headi=headi*10+(head[iii]-48);
					}
					arrow = homehead - headi; 
					if (arrow < 0) {
						arrow = arrow + 360;
					}     
      
					arrowr[0]=(  arrow/100)+3;
					arrowr[1]=(( arrow%100)/10)+3;      
					arrowr[2]=(( arrow%100)%10)+3; 
				}
  
  
				//===================================================      
				// END los calculation
				//=================================================== 



				//===================================================      
				// Calculate speed - to km/h
				//===================================================      

				if (speedkn2[0] < 14) {
					speedkm=0;
					for (iiii=0;iiii<3;iiii++) {
						if (speedkn2[iiii]<3) {
							break;
						}
						speedkm=speedkm*10+(speedkn2[iiii]-3);
					}
					speedkm = speedkm*10+(speedkn2[iiii+1]-3);
					
					if (units==0) {
						speedkm=(speedkm*11.51)/10; 
					}
					else {
						speedkm=(speedkm*18.52)/10;
					}
					speedkmw[0]=(  speedkm/1000)+3;
					speedkmw[1]=(( speedkm%1000)/100)+3;      
					speedkmw[2]=(((speedkm%1000)%100)/10)+3; 
					speedkmw[3]=(((speedkm%1000)%100)%10)+3; 
				}
				speedkm2=speedkm;
				//===================================================      
				// End speed calculation
				//===================================================    
      
      
				//===================================================      
				// Flight summary
				//===================================================  
				if (speedkm > (unsigned int)max_speed) {
					max_speed = speedkm;

					max_speedr[0]=(  max_speed/1000)+3;
					max_speedr[1]=(( max_speed%1000)/100)+3;      
					max_speedr[2]=(((max_speed%1000)%100)/10)+3; 
					max_speedr[3]=(((max_speed%1000)%100)%10)+3;
				}
      
				if (los > max_los) {
					max_los = los;
					
					max_losr[0]=(  max_los/1000)+3;
					max_losr[1]=(( max_los%1000)/100)+3;      
					max_losr[2]=(((max_los%1000)%100)/10)+3; 
					max_losr[3]=(((max_los%1000)%100)%10)+3;
				}      
      

				if (altitude[0] < 14) {

					altitude_num=0;
					for (iiii=0;iiii<6;iiii++) {
						if (altitude[iiii] < 3) {
							break;
						}
						altitude_num=altitude_num*10+(altitude[iiii]-3);
					}
          
					altitude_num=altitude_num*10+(altitude[iiii+1]-3);
    
					if (units==0) {
						altitude_num = altitude_num * 3.28; 
					}
    
    
					if (altitude_offset_on == 1) {
						altitude_num=altitude_num-altitude_offset;
						if (altitude_num<0) {
							altitude_num=abs(altitude_num);
							altitude_negative=1; 
						}
						else{
							altitude_negative=0;
						}
            
					}
    
					altitude_num2=altitude_num;
					altituder[0]=(  (altitude_num/10000)+3);  
					altituder[1]=(  (altitude_num%10000)/1000)+3;
					altituder[2]=(( (altitude_num%10000)%1000)/100)+3;      
					altituder[3]=((((altitude_num%10000)%1000)%100)/10)+3; 
					altituder[4]=((((altitude_num%10000)%1000)%100)%10)+3;
             
          
					if (altitude_num > max_alt) {
						max_alt = altitude_num; 
    
						max_altr[0]=altituder[0];
						max_altr[1]=altituder[1];
						max_altr[2]=altituder[2];
						max_altr[3]=altituder[3];
						max_altr[4]=altituder[4];
         
					}
      
				}

      
				// Add time
				if (time[5] !=last_time) {
					last_time=time[5];
					flight_time++;
      
					flight_timer[0]=(( flight_time/600))+3;     
					flight_timer[1]=(((flight_time%600)/60))+3;      
					flight_timer[2]=((flight_time%600)%60)/10+3; 
					flight_timer[3]=((flight_time%600)%60)%10+3; 
      
					kmh_total=kmh_total+speedkm;

					total_distancer[0]=(   (kmh_total/36)/10000)+3;      
					total_distancer[1]=(  ((kmh_total/36)%10000)/1000)+3;
					total_distancer[2]=(( ((kmh_total/36)%10000)%1000)/100)+3;      
					total_distancer[3]=(((((kmh_total/36)%10000)%1000)%100)/10)+3; 
					total_distancer[4]=(((((kmh_total/36)%10000)%1000)%100)%10)+3;     
     
					// Avg speed...
					avg_speed = kmh_total / flight_time;
      
					avg_speedr[0]=(  avg_speed/1000)+3;
					avg_speedr[1]=(( avg_speed%1000)/100)+3;      
					avg_speedr[2]=(((avg_speed%1000)%100)/10)+3; 
					avg_speedr[3]=(((avg_speed%1000)%100)%10)+3;           
      
				}
					
				//===================================================      
				// Flight summary END
				//===================================================        
          
				//===================================================      
				// Set homeposition
				//===================================================    
      
				// This is used to set homeposition. 
				if (homepos == 0) {
					// If we have GPS-fix;
					if (GPSfix > '0') {
						homeposcount++;
						if (homeposcount > set_home_delay) {
							lathome = lats;
							lonhome = lons;
							homepos = 1;
           
							max_los =0;
							max_speed=0;
							kmh_total=0;
							max_alt=0;
							flight_time=0;
							altitude_offset = altitude_num;
            
							lat_deg= (latitude[1]-48)+ (latitude[0]-48)*10;
							longitude_factor = cos((lat_deg*2*3.14)/360);

						}
					}
				}
     
     
				//===================================================      
				// End set homeposition
				//===================================================
      
      
				if (speedkm < summary_speed*10 && los < summary_los && current_num < summary_current*10 && flight_time > summary_time && altitude_num < summary_altitude*10) {
					landed = 1; 
				}
				else {
					landed = 0; 
				}

				long_buf=current_num;
				if (show_mah_km ==1) {
					if (speedkm2 > 10) {
						mahkm=((long_buf*1000)/(speedkm2));
					}
					
					if (mahkm > 9999) {
						mahkm = 9999; 
					}
					
					mahkmr[0]=(  mahkm/1000)+3;
					mahkmr[1]=(( mahkm%1000)/100)+3;      
					mahkmr[2]=(((mahkm%1000)%100)/10)+3; 
					mahkmr[3]=(((mahkm%1000)%100)%10)+3;
				}
			}
				
			// ====================================================================================
			// Second GPS String END     
			// ====================================================================================    

			// Check if the line is the GGA nmea GPS-line:
    
			if (GPSbuffer[6]=='A' && GPSbuffer[5] =='G') {
            
				// Again, reset altitude to avoid "hanging characters". 
				// Can maybe be done smarter. 

				altitude[0]=14;
				altitude[1]=14;         
				altitude[2]=14;         
				altitude[3]=14;
				altitude[4]=14;
				altitude[5]=14;
                 
				satellites[0]=14;
				satellites[1]=14;                 
                 
				for (int i = 0; i<bufnr; i++) {
					if (GPSbuffer[i]==',') {
						n=0;
						count++;
					}
        
					else {
						switch (count) {
              
							case 6:
							GPSfix = GPSbuffer[i];  
							break;  

							case 7:
							satellites[n] = GPSbuffer[i]-45;
							break;
              
							case 9:
							altitude[n]=GPSbuffer[i]-45;
							break; 

							default: break;
              
						}
            
						if (count == 10) {
							break; 
						}            
						n++;
					}
				}      
      
				altitude2[0]=altitude[0];
				altitude2[1]=altitude[1];         
				altitude2[2]=altitude[2];         
				altitude2[3]=altitude[3];
				altitude2[4]=altitude[4];
				altitude2[5]=altitude[5];  
           
				satellitesr[0]=satellites[0];
				satellitesr[1]=satellites[1];            
			}
        
			// No matter what has been received, count and bufnr is reset
			// to be ready to receive the next GPS string.
			count=0;
			bufnr=0;

		}        
		if (bufnr > 98) {
			bufnr=0;
		}
	}
#else
	if (software_timer >= VIDEO_FPS) {
		software_timer = 0;
		flight_time++;
		flight_timer[0]=(( flight_time/600))+3;     
		flight_timer[1]=(((flight_time%600)/60))+3;      
		flight_timer[2]=((flight_time%600)%60)/10+3; 
		flight_timer[3]=((flight_time%600)%60)%10+3;
        if (flight_time > 10)
            homepos = 1;
	}
#endif	
#if (digital_rssi==1)  
		
	if (rssi_read) {
		rssi_read = 0;
		rssi_measurements_count = 0;
		current_min = 100;
		SET_RISING();
	}
		
	if (rssi_measurements_count < 1) {
		// overflow occured => invalid result
		if (TIFR1 & _BV(TOV1)) {
			SET_RISING();
		}
		// new timer value captured
		else if (TIFR1 & _BV(ICF1)) {
			uint32_t timer = ICR1;
            
			if (state == rising) {
				SET_FALLING();
				rising_ticks = timer;
			}
			else {
				SET_RISING();         
				timer = timer - rising_ticks;                
                
				if (timer >= (rssi_min-10) && timer <= (rssi_max+10) ) {
					last_rssi_measurement = flight_time;
                    
#if (show_raw_rssi == 0)
					timer = (timer - rssi_min)*100;
					timer /= MAX_DURATION;
					if (timer > current_min)
						timer = current_min;
					else
						current_min = timer;
#endif
					rssi_measurements_count++;
					rssi_reading = timer;
				}
				else if (timer < (rssi_min-10)) {
					rssi_reading = 1;
				}
				else if (timer > (rssi_max+10)) {
					rssi_reading = 100;
				}
				
#if (show_raw_rssi == 1)
				rssi_reading = timer;
				rssi_measurements_count++;
#endif
			}
		}
	}
		
	if (last_rssi_measurement+2 < flight_time) {
		rssi_reading = 0;
	}
#endif
} 
}
