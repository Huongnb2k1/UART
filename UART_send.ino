#define RXD2 16
#define TXD2 17
#define HEADER_BYTE1 0x5A
#define HEADER_BYTE2 0xA5
//#define write_TX(x)  ((x) ? (GPIO.out_w1ts = (1 << TX2)) : (GPIO.out_w1tc = (1 << TX2)))

uint8_t receivedMessage[26];
uint8_t save[26];  // save để lưu trữ bản tin
uint8_t count = 0;

uint16_t calculateCRC(const uint8_t *byte_data, uint8_t data_length) {
  uint16_t crc = 0xFFFF;

  for (uint8_t i = 0; i < data_length; i++) {
    crc ^= (uint16_t)byte_data[i] << 8;

    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }

  return crc;
}
void setup() {
 
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
  
  
  uint8_t message1[20] = {0x01, 0x02, 0x03, 0x04, 0x05,
                          0x06, 0x07, 0x08, 0x09, 0x0A,
                          0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
                          0x10,0x11, 0x12, 0x13, 0x14}; 
  uint8_t message2[20] = {0x01, 0x02, 0x03, 0x04, 0x05,
                          0x06, 0x07, 0x08, 0x09, 0x0A,
                          0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
                          0x10,0x11, 0x12, 0x13, 0x14}; 


  sendMessage(0x01, message1, sizeof(message1));
  sendMessage(0x02, message2, sizeof(message2));
}

void loop() {
  receiveMessage();
}

void sendMessage(uint8_t id, uint8_t *byte_data, uint8_t data_length) {
  uint8_t message[26] = {0};

  message[0] = HEADER_BYTE1;
  message[1] = HEADER_BYTE2;


  message[2] = id;

  message[3] = data_length;

 memcpy(&message[4],byte_data, data_length);
  // Đóng gói dữ liệu
  for (uint8_t i = 0; i < data_length ; i++) {
    message[4 + i] = byte_data[i];
  }

  
  uint16_t crc = calculateCRC(message, 24);

  // Đóng gói CRC vào cuối bản tin
  message[24] = (crc >> 8) & 0xFF;
  message[25] = crc & 0xFF;
   Serial2.print("message ");
    Serial2.print(id);
   Serial2.print(": ");
  // Gửi bản tin qua UART
    for (uint8_t i = 0; i < 26 ; i++) {
    Serial2.print(message[i], HEX);
    Serial2.print(" ");
  }
  Serial2.println();
}
void receiveMessage() {
  if (Serial2.available()) {
        uint8_t byteReceived = Serial2.read();

        if (count == 0 && byteReceived == HEADER_BYTE1) {
            save[count++] = byteReceived;  // Lưu byte đầu tiên
        } else if (count == 1 && byteReceived == HEADER_BYTE2) {
            save[count++] = byteReceived;  // Lưu byte thứ hai
        } else if (count > 1 && count < 26) {
            save[count++] = byteReceived;  // Lưu các byte còn lại
        }

        // Nếu đã nhận đủ 26 byte
        if (count == 26) {
            uint16_t crc = calculateCRC(save, 24);  // Tính CRC từ byte 0 đến byte 23
            uint16_t receivedCRC = (save[24] << 8) | save[25];  

            // Kiểm tra CRC
            if (crc == receivedCRC) {
                
                processData(save);
            }

           
            count = 0;
        }
    }

void processData(uint8_t *message) {
    
    uint8_t id = message[2];
    uint8_t data_length = message[3];
    uint8_t byte_data[20];
    memcpy(byte_data, &message[4], data_length);

    
    Serial.print("ID: ");
    Serial.println(id);
    Serial.print("Data: ");
    for (int i = 0; i < data_length; i++) {
        Serial.print(byte_data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}
