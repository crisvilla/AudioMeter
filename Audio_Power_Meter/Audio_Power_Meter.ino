  /*--------------------------------|
   * This code will enable you to   |
   * determine maximum power output |
   * of an audio power amplifier    |
   * using any arduino board. Here  |
   * I am using arduino pro mini,   |
   * PD8554 or commonly known as    |
   * nokia 5110 lcd, resistor & cap.|
   *                                |
   *   TX  1         - RAW          |
   *   RX  0         - GND          |
   *   GND -         - RST          |
   *   GND -         - VCC          |
   *   N/A 2        A3 N/A          |
   *   RST 3        A2 N/A          |
   *   CE  4        A1 N/A          |
   *   DC  5        A0 AUDIO INPUT  |
   *   DIN 6        13 N/A          |
   *   CLK 7        12 N/A          |
   *   N/A 8        11 N/A          |
   *   N/A 9        10 N/A          |
   *       A4 A5 A6 A7              |
   *                                |
   * -------------------------------|
   */
  // this is a free software and can be used at ur own risk, Cris Villa
  //***************Libraries*****************
  #include <SPI.h>                       // you need to download this libraries from Adafruit github
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>


  // Software SPI (slower updates, more flexible pin options):
  // pin 7 - Serial clock out (SCLK)
  // pin 6 - Serial data out (DIN)
  // pin 5 - Data/Command select (D/C)
  // pin 4 - LCD chip select (CS)
  // pin 3 - LCD reset (RST)
  Adafruit_PCD8544 mylcd = Adafruit_PCD8544(7, 6, 5, 4, 3);


  char tmp[14];

  // ***************some variables********************

  long timer = 0;
  int half_sec, last_sec, sec;
  int scope[80];
  float out_vol, last_out_vol, last_out_cur, out_cur, Rload, max_pow=0.0;
  int timeDivision = 1;
  int load_impedance = 4;       // make it default to 4 ohms
  

  void setup() {
    Serial.begin(9600);         // serial monitor used to change parameters
    // Initialize device.
    mylcd.begin();              // initialize LCD
    mylcd.setContrast(25);      // set ur contrast here
    mylcd.clearDisplay();       // clear lcd
    // Initialize device.


  }

  void loop() {
  // Delay between measurements.
  static float max_data=0, min_data=2000, total_diff=0;

  if(millis()-timer >=500){   // check for timing 1000 = 1 second, half sec = 500, quarter of a sec = 250
    half_sec++;               // every (half/quarter)seconds increment ur timer
    timer = millis();         // initialize ur timer equals to arduino timing
  }
  if(half_sec>=120) half_sec=0;


  while(Serial.available()){                      // if serial data available
    switch(Serial.read()){                        // with this u have access to change
      case '1':timeDivision = 1;                  // some variables used in the device
               Serial.print("Time division = ");  // this time division approach were to change only
               Serial.print(timeDivision);        // delay on sampling and not exactly the same
               Serial.println(" ms");             // function as to what happened in real osci.
               break;                             // osci effect on the lcd was to mimick the real osci
      case '2':timeDivision = 5;                  // only and can be develop and improved in other apps
               Serial.print("Time division = ");  // it will show better curve on 880hz
               Serial.print(timeDivision);
               Serial.println(" ms");
               break;
      case '3':timeDivision = 10;                  // this delay may slowdown ur device
               Serial.print("Time division = ");
               Serial.print(timeDivision);
               Serial.println(" ms");
               break;
      case '4':load_impedance = 4;
               Serial.print("Load impedance = ");
               Serial.print(load_impedance);
               Serial.println(" ohms");
               break;
      case '8':load_impedance = 8;
               Serial.print("Load impedance = ");
               Serial.print(load_impedance);
               Serial.println(" ohms");
               break;
    }
  }  // en of serial com

  max_data=0;                                             // need to initialized data every cycle(it seems that my static declaration didn't work)
  min_data=2000;
  total_diff=0;
  for(int i=0;i<79;i++){                                  // sampling input signal, 80 samples taken
    // read analog signal
    scope[i] = analogRead(A0);                            // read one sample at a time
    out_vol = out_vol + scope[i];                         // just add it up(totalizer)
    max_data = max(max_data,scope[i]);                    // store maximum value
    min_data = min(min_data,scope[i]);                    // store minimum value
    delay(timeDivision);                                  // rest for a while
  }
  out_vol = out_vol / 80;                                 // average of total samples
  total_diff = (max_data - min_data) * 216 / 1023;        // analog to digital conversion, "216vmax" is the ratio of resistors, this is
                                                          // a convertion of analog to digital and is mapped to 1023
  
  if(last_sec!=half_sec){                                 // every event change
    mylcd.clearDisplay();                                 // always clear display in order to draw fresh display
    

    mylcd.drawRoundRect(0,0,83,22,3,BLACK);            // draw rectangle
    float vp = total_diff / 2;                         // try to get Vout peak
    float pp = (vp * vp) / (2 * load_impedance);       // why 2? look for Electronics and ckt theories by Boleystad chapter 16
    max_pow = max(max_pow,pp);                         // all theories and example were presented there. Also u may watch here: "https://www.youtube.com/watch?v=7Svlaaz8lrk&t"
    set_txt(dtostrf(pp,6,1,tmp),3,4,BLACK,2,2);        // display captured power in lcd
    set_txt("W",76,4,BLACK,1,1);
    mylcd.drawRoundRect(0,24,83,24,3,BLACK);           // draw second rectangle
    for(int i=0;i<79;i++){                             // this routine will display audio signal line on lcd
      if(i!=0)mylcd.drawLine(i,25 + map(scope[i-1],0,1023,0,22),i+1,25 + map(scope[i],0,1023,0,22),BLACK);
    }
    set_txt(dtostrf(max_pow,5,1,tmp),46,26,BLACK,1,1); // display maximum power output that the device detected
    set_txt("W",76,26,BLACK,1,1);
    set_txt(dtostrf(vp,5,1,tmp),46,39,BLACK,1,1); // display voltage output in peak rating
    set_txt("V",76,39,BLACK,1,1);
    mylcd.display();
    last_sec=half_sec;
  }  // end of one second event

  }   // loop ends here

void set_txt(char temp[14], int x, int y, int color, int textsizex, int textsizey){  // this sketch is to display ur characters on lcd
    mylcd.setTextSize(textsizex,textsizey);
    mylcd.setTextColor(color);
    mylcd.setCursor(x,y);
    mylcd.println(temp);
  }
