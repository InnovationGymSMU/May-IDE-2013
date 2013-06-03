#include <VirtualWire.h>
#include <SimpleTimer.h>
#include <string.h>

#undef int
#undef abs
#undef double
#undef float
#undef round

//Packet book keeping
uint8_t node_type = 3;// = 1;        //Type of node running this code: 1=TxRx, 2=Tx, 3=Rx
uint8_t nodeID_field = 3;// = 5;     //Field indicating which node gathered the data
uint8_t next_hop = 0;         //node id of next hop in network

uint8_t rx_pin = 2;
uint8_t tx_pin = 3;

SimpleTimer timer;

const int middleValue = 512; //the middle of the range of analog values
const int numberOfSamples = 128; //how many readings will be taken each time
int sample; //the value read from microphone each time
long signal; //the reading once you have removed DC offset
long averageReading; //the average of that loop of readings
long runningAverage=0; //the running average of calculated values
const int averagedOver= 16; //how quickly new values affect running average
//bigger numbers mean slower
int counter =0;

void gather()
{
    if (counter == 255)
    {
      counter = 1;
    } else {
      counter++;
    }  
  
    //Transmit Data from Sensors      
    uint8_t data_packet[27];
    //Gather sensor dataand format packet
    //Fill in Sender ID and Node ID
    data_packet[0] = next_hop;
    data_packet[1] = nodeID_field;
      
    //Microphones
    for (int j = 0; j<3; j++)
    {
      long sumOfSquares = 0;
      for (int i=0; i<numberOfSamples; i++) //take many readings and average them
      { 
        sample = analogRead(j+1); //take a reading
        signal = (sample - middleValue); //work out its offset from the center
        signal *= signal; //square it to make all values positive
        sumOfSquares += signal; //add to the total
      }
      averageReading = sumOfSquares/numberOfSamples; //calculate running average
      runningAverage=(((averagedOver-1)*runningAverage)+averageReading)/averagedOver;
      //Serial.println(runningAverage); //print the value so you can check it
      data_packet[2+2*j] = runningAverage;
      data_packet[3+2*j] = runningAverage >> 8;
    }
    
    data_packet[8] = analogRead(4); //Light Sensor
    data_packet[9] = 0;
    data_packet[10] = 0;
    data_packet[11] = 0;
    data_packet[12] = 0;
    data_packet[13] = 0;
    data_packet[14] = 0;
    data_packet[15] = 0;
    data_packet[16] = 0;
    data_packet[17] = 0;
    data_packet[18] = 0;
    data_packet[19] = 0;
    data_packet[20] = 0;
    data_packet[21] = 0;
    data_packet[22] = 0;
    data_packet[23] = 0;
    data_packet[24] = 0;
    data_packet[25] = 0;
    data_packet[26] = counter;
      
    if (node_type == 2 || node_type == 1)
    {  
      transmit(data_packet);
    }
    
    if (node_type == 3)
    {
      int i;
      for(i = 0; i < 27; i++)
      {
        Serial.print(data_packet[i]);
        Serial.print("*");
      }
      Serial.println();
    }
}

void setup()
{
    Serial.begin(115200);	// Debugging/to Pi

    pinMode(4,INPUT);     //Node ID DIP
    pinMode(5,INPUT);     //Node ID DIP
    pinMode(6,INPUT);     //Node ID DIP
    
    pinMode(8,INPUT);     //Node Type DIP
    pinMode(9,INPUT);     //Node Type DIP
    pinMode(11,INPUT);    //Receiver
    
    // Initialise the IO and ISR
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);	 // Bits per sec
    vw_set_rx_pin(rx_pin);
    vw_set_tx_pin(tx_pin);
    vw_rx_start();       // Start the receiver PLL running
    
    //DIP SWITCH CODE
    //node_type = digitalRead(8)^(digitalRead(9)<<1);
    //nodeID_field = digitalRead(3)^(digitalRead(4)<<1)^(digitalRead(5)<<2)^(digitalRead(6)<<3);
    
    //Serial.println(node_type);
    //Serial.println(nodeID_field);
    
    timer.setInterval(4562-380, gather); //we want a packet sent every 4562 ms. each packet takes ~380 ms to send
}

void loop()
{
    timer.run();  
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    
    //CHECK RECEIVE BUFFER
    if (node_type == 1 || node_type == 3)
    {
      //Serial.println("Type 3");
      if (vw_get_message(buf, &buflen)) // Non-blocking
      {
        // Message with a good checksum received, dump it.        
        if (buf[0] == nodeID_field) //Needs to filter out packets from invalid source
        {
          if (node_type == 1)
          { 
            buf[0] = next_hop;
            transmit(buf);  // Forward Any Receive Data
          }
          if (node_type == 3)
          {
            //Forward packet to PI
            int i;
            for(i = 0; i < 27; i++)
            {
              Serial.print(buf[i]);
              Serial.print("*");
            }
            Serial.println();
          }
        }
      }
    }   
}

void transmit(uint8_t* msg)
{
    Serial.print("Sending Data From: ");
    Serial.print(msg[1]);
    Serial.print(" to: ");
    Serial.print(msg[0]);
    Serial.println();
    vw_send(msg, 27);
    vw_wait_tx(); // Wait until the whole message is gone
    delay(200);
}

