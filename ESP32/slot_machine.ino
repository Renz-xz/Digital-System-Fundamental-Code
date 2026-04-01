#define TRIGGER_PIN 2   
#define S1_PIN 4        
#define S2_PIN 16       
#define S3_PIN 17       
#define WIN_PIN 5       
#define NUM_SYMBOLS 8

int currentStep = 0;
int val1, val2, val3;

void drawUI(String msg, int n1, int n2, int n3);
void printSymbol(int n);

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(S1_PIN, INPUT); 
  pinMode(S2_PIN, INPUT);
  pinMode(S3_PIN, INPUT); 
  pinMode(WIN_PIN, INPUT);
  
  digitalWrite(TRIGGER_PIN, LOW);
  randomSeed(analogRead(34)); // ใช้ขาที่ว่างเพื่อสุ่มค่าเริ่มต้น [cite: 4]
  
  currentStep = 0;
  val1 = random(0, 8); 
  val2 = random(0, 8);
  val3 = random(0, 8);
  drawUI("READY! PRESS ENTER TO SPIN", val1, val2, val3);
}

void loop() {
  if (Serial.available() > 0) {
    while(Serial.available()) { Serial.read(); delay(2); }

    // --- ตรวจสอบว่าเกมจบหรือยัง (ถ้า Step >= 4 คือจบแล้ว) ---
    if (currentStep >= 4) {
        // แสดงค่าเดิม (val1, val2, val3) ไม่สุ่มใหม่ และเปลี่ยน Status
        drawUI("PLEASE PRESS RESET TO START OVER", val1, val2, val3);
        return; // ออกจาก loop ทันที ไม่ทำงานส่วนที่เหลือ
    }

    // ส่งสัญญาณไป FPGA
    digitalWrite(TRIGGER_PIN, HIGH);
    delay(100); 
    digitalWrite(TRIGGER_PIN, LOW);

    currentStep++;
    String status = "";

    if (currentStep == 1) {
        val1 = random(0, 8); 
        val2 = random(0, 8);
        val3 = random(0, 8); 
        status = "--- SPINNING ---";
    }
    else if (currentStep == 2) {
        val1 = random(0, 8); // สุ่มใหม่ตอนหยุดช่อง 1 [cite: 9]
        val2 = random(0, 8);
        val3 = random(0, 8);
        status = "STOPPED [1]";
    }
    else if (currentStep == 3) {
        val2 = random(0, 8); // สุ่มใหม่ตอนหยุดช่อง 2 [cite: 10]
        val3 = random(0, 8);
        status = "STOPPED [2]";
    }
    else if (currentStep == 4) {
        val3 = random(0, 8); // สุ่มใหม่ตอนหยุดช่อง 3 
        bool win = digitalRead(WIN_PIN);
        status = win ? "!!! JACKPOT !!!" : "YOU LOSE";
    }

    drawUI(status, val1, val2, val3);
  }
}

void drawUI(String msg, int n1, int n2, int n3) {
  Serial.print("\033[2J\033[1;1H"); // ล้างหน้าจอ
  Serial.println("#########################");
  Serial.println("#   FPGA SLOT MACHINE   #");
  Serial.println("#########################");
  Serial.println("#                       #");
  Serial.print  ("#     [ "); printSymbol(n1); 
  Serial.print(" ] [ "); printSymbol(n2); 
  Serial.print(" ] [ "); printSymbol(n3); 
  Serial.println(" ]     #");
  Serial.println("#                       #");
  Serial.println("#########################");
  Serial.println(" STATUS: " + msg);
  Serial.println("-------------------------");
  
  if(currentStep < 4) {
    Serial.println(">> Press ENTER to Action <<");
  } else {
    Serial.println(">> Press RESET on ESP32 to Start Over <<");
  }
}

void printSymbol(int n) {
  String symbols[] = {"🍒", "🍋", "🍊", "🍉", "🍇", "💎", "⭐", "🔔"};
  Serial.print(symbols[n % 8]); 
}