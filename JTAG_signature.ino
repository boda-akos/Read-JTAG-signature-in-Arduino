const int TCK=8, TDI=9, TDO=10, TMS=11;

void setup() {
  pinMode(TCK,OUTPUT); pinMode(TDI,OUTPUT); pinMode(TMS,OUTPUT);
  pinMode(TDO,INPUT);
  goReset();
  Serial.begin(115200);
  Serial.println("JTAG ID Code Test "); delay(1000);
}

void tck() { digitalWrite(TCK,HIGH); delayMicroseconds(5); digitalWrite(TCK,LOW); delayMicroseconds(5); }

void goReset() { digitalWrite(TMS,HIGH); for(int i=0;i<5;i++) tck(); }

void toShiftIR() {
  int seq[]={0,1,1,0,0};
  for(int i=0;i<5;i++){ digitalWrite(TMS,seq[i]); tck(); }
}

void toShiftDR_fromExit1IR() {
  int seq[]={1,1,0,0};  // Update-IR, Select-DR, Capture-DR, Shift-DR
  for(int i=0;i<4;i++){ digitalWrite(TMS,seq[i]); tck(); }
}

uint32_t readIDCODE() {
  delay(1);
  goReset();
  delay(4);
  toShiftIR();
  delay(3);
  int ir[3]={1,0,0};                 // IDCODE 0x01, LSB first; confirm from BSDL
  for(int i=0;i<3;i++){ digitalWrite(TDI,ir[i]); digitalWrite(TMS,(i==2)?1:0); tck(); }
  delay(2);
  toShiftDR_fromExit1IR();
  uint32_t id=0;
  for(int i=0;i<32;i++){
    id |= ((uint32_t) digitalRead(TDO)<<i);
    digitalWrite(TMS,(i==31)?1:0);
    tck();
  }
  return id;
}

void loop(void) {
  goReset();                     // TMS=1 ×5 → TLR
  uint32_t id=0;
 
  // TLR → Shift-DR: TMS = 0,1,0,0
  int seq[]={0,1,0,0};
  for(int i=0;i<4;i++){ digitalWrite(TMS,seq[i]); tck(); }
 
  // Read 32 bits, sampling while TCK is HIGH
  for(int i=0;i<32;i++){
    digitalWrite(TMS,(i==31)?1:0);
    digitalWrite(TCK, HIGH);
    delayMicroseconds(5);
    id |= ((uint32_t)digitalRead(TDO)<<i);
    digitalWrite(TCK, LOW);
    delayMicroseconds(5);
  }
  Serial.println(id,HEX);delay(1000);
}
