/*
 * ================================================================
 *  BLIND HELPER - VERSION 1
 * ================================================================
 *
 *  LiDAR-Based Assistive Obstacle Detection Device
 *
 *  Hardware:
 *  ---------------------------------------------------------------
 *  Microcontroller : ESP32-C3 Super Mini
 *  Distance Sensor : TF-Luna LiDAR
 *  Feedback        : Vibration Motor
 *  Motor Driver    : 2N2222 NPN transistor
 *
 *  TF-Luna connection:
 *  ---------------------------------------------------------------
 *  TF-Luna +5V  -> ESP32 5V
 *  TF-Luna GND  -> ESP32 GND
 *  TF-Luna SDA  -> ESP32 GPIO4
 *  TF-Luna SCL  -> ESP32 GPIO3
 *  TF-Luna MODE -> GND
 *
 *  Vibration motor driver:
 *  ---------------------------------------------------------------
 *
 *  ESP32 GPIO1
 *       |
 *      470 ohm
 *       |
 *       +------ 2N2222 Base
 *       |
 *      10k
 *       |
 *      GND
 *
 *  2N2222 Emitter   -> GND
 *  2N2222 Collector -> Motor negative
 *  Motor positive   -> Motor supply
 *
 *  Flyback diode across motor:
 *
 *  Cathode (stripe) -> Motor positive
 *  Anode            -> Motor negative / transistor collector
 *
 *  IMPORTANT:
 *  ESP32 and motor supply must share a common GND.
 *
 * ================================================================
 *  WARNING
 * ================================================================
 *
 *  This is an experimental research prototype.
 *
 *  It is NOT a certified mobility aid or medical device.
 *  It must NOT replace a white cane, guide dog, or professional
 *  orientation and mobility techniques.
 *
 * ================================================================
 */

#include <Wire.h>


// ================================================================
// PIN CONFIGURATION
// ================================================================

#define I2C_SDA 4
#define I2C_SCL 3

#define MOTOR_PWM_PIN 1


// ================================================================
// TF-LUNA CONFIGURATION
// ================================================================

#define TF_LUNA_ADDRESS 0x10
#define DATA_LENGTH 9

/*
 * TF-Luna single measurement command.
 *
 * Command:
 *
 * 5A 05 00 01 60
 */

const uint8_t measurementCommand[5] = {
  0x5A,
  0x05,
  0x00,
  0x01,
  0x60
};


// ================================================================
// MOTOR PWM CONFIGURATION
// ================================================================

#define MOTOR_PWM_FREQUENCY 2000

#define MOTOR_PWM_RESOLUTION 8

#define MOTOR_PWM_MAX 255


// ================================================================
// OBSTACLE WARNING DISTANCES
// ================================================================

/*
 * Motor starts vibrating when an obstacle is at or below 100 cm.
 */

const uint16_t MAX_WARNING_DISTANCE_CM = 100;


/*
 * At or below 15 cm the vibration motor operates at maximum
 * configured intensity.
 */

const uint16_t MIN_WARNING_DISTANCE_CM = 15;


// ================================================================
// MOTOR INTENSITY
// ================================================================

/*
 * Minimum useful motor PWM value.
 *
 * Many vibration motors will not reliably start at extremely
 * low PWM duty cycles.
 */

const uint8_t MIN_MOTOR_DUTY = 100;


/*
 * Maximum motor PWM duty.
 */

const uint8_t MAX_MOTOR_DUTY = 255;


// ================================================================
// TF-LUNA SIGNAL QUALITY
// ================================================================

/*
 * Currently disabled.
 *
 * Later we can determine experimentally what signal-strength
 * threshold should be considered reliable.
 */

const uint16_t MIN_SIGNAL_STRENGTH = 0;


// ================================================================
// GLOBAL VARIABLES
// ================================================================

unsigned long readingCounter = 0;

uint8_t currentMotorDuty = 0;


// ================================================================
// FUNCTION DECLARATIONS
// ================================================================

bool readTFLuna(
  uint16_t &distanceCm,
  uint16_t &signalStrength,
  float &temperatureC
);

bool verifyChecksum(
  const uint8_t *data,
  uint8_t length
);

void updateVibrationMotor(
  uint16_t distanceCm,
  uint16_t signalStrength
);

void setMotorDuty(
  uint8_t duty
);

void stopMotor();


// ================================================================
// SETUP
// ================================================================

void setup() {

  // --------------------------------------------------------------
  // Serial monitor
  // --------------------------------------------------------------

  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("        BLIND HELPER - VERSION 1");
  Serial.println("========================================");
  Serial.println();

  Serial.println("Initializing system...");


  // --------------------------------------------------------------
  // Configure vibration motor PWM
  // --------------------------------------------------------------

  Serial.println("Initializing vibration motor PWM...");

  if (!ledcAttach(
        MOTOR_PWM_PIN,
        MOTOR_PWM_FREQUENCY,
        MOTOR_PWM_RESOLUTION
      )) {

    Serial.println();
    Serial.println("ERROR:");
    Serial.println("Motor PWM initialization failed.");

    while (true) {

      delay(1000);

    }
  }

  stopMotor();

  Serial.println("Motor PWM initialized.");


  // --------------------------------------------------------------
  // Initialize I2C
  // --------------------------------------------------------------

  Serial.println("Initializing I2C...");

  if (!Wire.begin(
        I2C_SDA,
        I2C_SCL,
        100000
      )) {

    Serial.println();
    Serial.println("ERROR:");
    Serial.println("I2C initialization failed.");

    stopMotor();

    while (true) {

      delay(1000);

    }
  }

  Serial.println("I2C initialized.");


  // --------------------------------------------------------------
  // Check TF-Luna
  // --------------------------------------------------------------

  Serial.println("Searching for TF-Luna...");

  Wire.beginTransmission(TF_LUNA_ADDRESS);

  uint8_t error = Wire.endTransmission();

  if (error == 0) {

    Serial.println(
      "TF-Luna detected at I2C address 0x10."
    );

  }

  else {

    Serial.print(
      "WARNING: TF-Luna not detected. I2C error code: "
    );

    Serial.println(error);

  }


  // --------------------------------------------------------------
  // Display configuration
  // --------------------------------------------------------------

  Serial.println();
  Serial.println("----------------------------------------");
  Serial.println("SYSTEM CONFIGURATION");
  Serial.println("----------------------------------------");

  Serial.print("I2C SDA GPIO: ");
  Serial.println(I2C_SDA);

  Serial.print("I2C SCL GPIO: ");
  Serial.println(I2C_SCL);

  Serial.print("Motor PWM GPIO: ");
  Serial.println(MOTOR_PWM_PIN);

  Serial.print("Motor PWM frequency: ");
  Serial.print(MOTOR_PWM_FREQUENCY);
  Serial.println(" Hz");

  Serial.print("Warning distance: ");
  Serial.print(MAX_WARNING_DISTANCE_CM);
  Serial.println(" cm");

  Serial.print("Maximum danger distance: ");
  Serial.print(MIN_WARNING_DISTANCE_CM);
  Serial.println(" cm");

  Serial.println("----------------------------------------");

  Serial.println();
  Serial.println("Blind Helper ready.");
  Serial.println();

}


// ================================================================
// MAIN LOOP
// ================================================================

void loop() {

  uint16_t distanceCm = 0;

  uint16_t signalStrength = 0;

  float temperatureC = 0.0f;


  // --------------------------------------------------------------
  // Read TF-Luna
  // --------------------------------------------------------------

  if (
    readTFLuna(
      distanceCm,
      signalStrength,
      temperatureC
    )
  ) {

    readingCounter++;


    // ------------------------------------------------------------
    // Update vibration motor
    // ------------------------------------------------------------

    updateVibrationMotor(
      distanceCm,
      signalStrength
    );


    // ------------------------------------------------------------
    // Serial output
    // ------------------------------------------------------------

    Serial.print("#");

    Serial.print(readingCounter);

    Serial.print(" | Distance: ");

    Serial.print(distanceCm);

    Serial.print(" cm");


    Serial.print(" | Signal strength: ");

    Serial.print(signalStrength);


    Serial.print(" | Temperature: ");

    Serial.print(
      temperatureC,
      1
    );

    Serial.print(" C");


    Serial.print(" | Motor PWM: ");

    Serial.print(currentMotorDuty);

    Serial.print("/");

    Serial.println(MOTOR_PWM_MAX);

  }

  else {

    /*
     * Fail-safe behavior:
     *
     * If a valid LiDAR measurement cannot be obtained,
     * stop the motor rather than using an invalid distance.
     */

    stopMotor();

    Serial.println(
      "ERROR: Invalid or incomplete TF-Luna reading. Motor stopped."
    );

  }


  // --------------------------------------------------------------
  // Measurement interval
  // --------------------------------------------------------------

  delay(100);

}


// ================================================================
// VIBRATION CONTROL
// ================================================================

void updateVibrationMotor(
  uint16_t distanceCm,
  uint16_t signalStrength
) {

  /*
   * No warning if:
   *
   * - signal strength is too low
   * - distance is zero
   * - obstacle is outside warning range
   */

  if (
    signalStrength < MIN_SIGNAL_STRENGTH ||
    distanceCm == 0 ||
    distanceCm > MAX_WARNING_DISTANCE_CM
  ) {

    stopMotor();

    return;

  }


  // --------------------------------------------------------------
  // Maximum danger zone
  // --------------------------------------------------------------

  if (
    distanceCm <= MIN_WARNING_DISTANCE_CM
  ) {

    setMotorDuty(
      MAX_MOTOR_DUTY
    );

    return;

  }


  // --------------------------------------------------------------
  // Calculate vibration intensity
  // --------------------------------------------------------------

  /*
   * Distance:
   *
   * 100 cm ---------> weak vibration
   *
   *                  |
   *                  |
   *                  V
   *
   * 15 cm ----------> maximum vibration
   *
   *
   * Arduino map():
   *
   * 100 cm -> MIN_MOTOR_DUTY
   * 15 cm  -> MAX_MOTOR_DUTY
   */

  long duty = map(
    distanceCm,
    MAX_WARNING_DISTANCE_CM,
    MIN_WARNING_DISTANCE_CM,
    MIN_MOTOR_DUTY,
    MAX_MOTOR_DUTY
  );


  // --------------------------------------------------------------
  // Safety constraint
  // --------------------------------------------------------------

  duty = constrain(
    duty,
    MIN_MOTOR_DUTY,
    MAX_MOTOR_DUTY
  );


  // --------------------------------------------------------------
  // Apply motor PWM
  // --------------------------------------------------------------

  setMotorDuty(
    (uint8_t)duty
  );

}


// ================================================================
// SET MOTOR DUTY
// ================================================================

void setMotorDuty(
  uint8_t duty
) {

  currentMotorDuty = duty;

  ledcWrite(
    MOTOR_PWM_PIN,
    currentMotorDuty
  );

}


// ================================================================
// STOP MOTOR
// ================================================================

void stopMotor() {

  currentMotorDuty = 0;

  ledcWrite(
    MOTOR_PWM_PIN,
    0
  );

}


// ================================================================
// READ TF-LUNA
// ================================================================

bool readTFLuna(
  uint16_t &distanceCm,
  uint16_t &signalStrength,
  float &temperatureC
) {

  // --------------------------------------------------------------
  // Send measurement command
  // --------------------------------------------------------------

  Wire.beginTransmission(
    TF_LUNA_ADDRESS
  );


  Wire.write(
    measurementCommand,
    sizeof(measurementCommand)
  );


  uint8_t transmissionResult =
    Wire.endTransmission();


  if (
    transmissionResult != 0
  ) {

    Serial.print(
      "I2C transmission error: "
    );

    Serial.println(
      transmissionResult
    );

    return false;

  }


  /*
   * Give TF-Luna a short amount of time before requesting
   * the measurement frame.
   */

  delayMicroseconds(500);


  // --------------------------------------------------------------
  // Request TF-Luna data
  // --------------------------------------------------------------

  uint8_t receivedBytes =
    Wire.requestFrom(
      TF_LUNA_ADDRESS,
      (uint8_t)DATA_LENGTH
    );


  // --------------------------------------------------------------
  // Check frame length
  // --------------------------------------------------------------

  if (
    receivedBytes != DATA_LENGTH
  ) {

    Serial.print(
      "Expected 9 bytes, received: "
    );

    Serial.println(
      receivedBytes
    );


    /*
     * Clear remaining I2C buffer.
     */

    while (
      Wire.available()
    ) {

      Wire.read();

    }


    return false;

  }


  // --------------------------------------------------------------
  // Read frame
  // --------------------------------------------------------------

  uint8_t data[DATA_LENGTH];


  for (
    uint8_t i = 0;
    i < DATA_LENGTH;
    i++
  ) {

    if (
      !Wire.available()
    ) {

      return false;

    }


    data[i] =
      Wire.read();

  }


  // --------------------------------------------------------------
  // Verify TF-Luna frame header
  // --------------------------------------------------------------

  /*
   * Valid TF-Luna measurement frame begins with:
   *
   * 0x59 0x59
   */

  if (
    data[0] != 0x59 ||
    data[1] != 0x59
  ) {

    Serial.print(
      "Invalid frame header: 0x"
    );

    Serial.print(
      data[0],
      HEX
    );

    Serial.print(
      " 0x"
    );

    Serial.println(
      data[1],
      HEX
    );


    return false;

  }


  // --------------------------------------------------------------
  // Verify checksum
  // --------------------------------------------------------------

  if (
    !verifyChecksum(
      data,
      DATA_LENGTH
    )
  ) {

    Serial.println(
      "Checksum error."
    );

    return false;

  }


  // --------------------------------------------------------------
  // Decode distance
  // --------------------------------------------------------------

  /*
   * Distance:
   *
   * Byte 2 = low byte
   * Byte 3 = high byte
   */

  distanceCm =
    (uint16_t)data[2] |
    ((uint16_t)data[3] << 8);


  // --------------------------------------------------------------
  // Decode signal strength
  // --------------------------------------------------------------

  /*
   * Signal strength:
   *
   * Byte 4 = low byte
   * Byte 5 = high byte
   */

  signalStrength =
    (uint16_t)data[4] |
    ((uint16_t)data[5] << 8);


  // --------------------------------------------------------------
  // Decode TF-Luna internal temperature
  // --------------------------------------------------------------

  /*
   * Temperature:
   *
   * Byte 6 = low byte
   * Byte 7 = high byte
   *
   * TF-Luna conversion:
   *
   * Temperature C =
   *
   * rawTemperature / 8 - 256
   */

  int16_t rawTemperature =
    (int16_t)(
      (uint16_t)data[6] |
      ((uint16_t)data[7] << 8)
    );


  temperatureC =
    rawTemperature / 8.0f
    - 256.0f;


  return true;

}


// ================================================================
// VERIFY TF-LUNA CHECKSUM
// ================================================================

bool verifyChecksum(
  const uint8_t *data,
  uint8_t length
) {

  uint16_t checksum = 0;


  /*
   * Add bytes 0 through 7.
   */

  for (
    uint8_t i = 0;
    i < length - 1;
    i++
  ) {

    checksum += data[i];

  }


  /*
   * TF-Luna checksum is the lower 8 bits
   * of the calculated sum.
   */

  uint8_t calculatedChecksum =
    checksum & 0xFF;


  /*
   * Byte 8 contains the checksum transmitted
   * by the TF-Luna.
   */

  return (
    calculatedChecksum ==
    data[length - 1]
  );

}
