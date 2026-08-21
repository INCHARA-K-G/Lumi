/*
  ============================================================
  LUMI - TOUCH SENSOR + TWO MAX7219 EYES
  Arduino UNO Q
  ============================================================

  TOUCH SENSOR:
  TTP223 SIG -> D2
  TTP223 VCC -> 3.3V
  TTP223 GND -> GND

  EYES:
  UNO Q D11 / MOSI -> Matrix 1 DIN
  UNO Q D13 / SCK  -> Matrix 1 CLK
  UNO Q D10        -> Matrix 1 CS
  UNO Q 5V         -> Matrix 1 VCC
  UNO Q GND        -> Matrix 1 GND

  Matrix 1 DOUT -> Matrix 2 DIN
  Matrix 1 CLK  -> Matrix 2 CLK
  Matrix 1 CS   -> Matrix 2 CS
  Matrix 1 VCC  -> Matrix 2 VCC
  Matrix 1 GND  -> Matrix 2 GND

  ============================================================
  FINAL EYE BEHAVIOUR
  ============================================================

  IDLE:
  Eyes open and naturally blink.

  LISTENING:
  Pupils move:
  centre -> left -> centre -> right.

  THINKING:
  Both eyes stay fully closed.

  SPEAKING:
  Eyes pulse between normal and smaller shape.

  HAPPY:
  Happy expression.

  CONFUSED:
  Different left/right eye expression.

  ERROR:
  Sad/error expression.

  SLEEP:
  Eyes closed.

  ============================================================
  TOUCH BEHAVIOUR
  ============================================================

  Touch #1 -> micEnabled = true
  Touch #2 -> micEnabled = false
  Touch #3 -> true
  etc.

  ============================================================
  BRIDGE FUNCTIONS
  ============================================================

  isMicEnabled()

  set_eye_state("IDLE")
  set_eye_state("LISTENING")
  set_eye_state("THINKING")
  set_eye_state("SPEAKING")
  set_eye_state("HAPPY")
  set_eye_state("CONFUSED")
  set_eye_state("ERROR")
  set_eye_state("SLEEP")
*/

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Arduino_RouterBridge.h>


// ============================================================
// MAX7219 CONFIGURATION
// ============================================================

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2
#define CS_PIN 10

#define LEFT_EYE_DEVICE 0
#define RIGHT_EYE_DEVICE 1

MD_MAX72XX mx(
  HARDWARE_TYPE,
  CS_PIN,
  MAX_DEVICES
);


// ============================================================
// TOUCH SENSOR
// ============================================================

const int TOUCH_PIN = 2;

int lastTouchReading = LOW;
int stableTouchState = LOW;

unsigned long lastTouchDebounceTime = 0;

const unsigned long TOUCH_DEBOUNCE_MS = 50;

bool micEnabled = false;


// ============================================================
// EYE STATES
// ============================================================

enum EyeState
{
  ST_IDLE,
  ST_LISTENING,
  ST_THINKING,
  ST_SPEAKING,
  ST_HAPPY,
  ST_CONFUSED,
  ST_ERROR,
  ST_SLEEP
};

EyeState currentState = ST_IDLE;

bool stateChanged = true;


// ============================================================
// ANIMATION VARIABLES
// ============================================================

unsigned long lastAnimationUpdate = 0;

unsigned long nextIdleBlinkAt = 0;

bool blinkActive = false;

uint8_t blinkStage = 0;

uint8_t listeningFrame = 0;

uint8_t speakingFrame = 0;


// ============================================================
// TIMING
// ============================================================

const unsigned long IDLE_BLINK_MIN_MS = 3000;
const unsigned long IDLE_BLINK_MAX_MS = 6000;

const unsigned long LISTENING_FRAME_MS = 300;

const unsigned long SPEAKING_FRAME_MS = 220;


// ============================================================
// EYE BITMAPS
// ============================================================


// ------------------------------------------------------------
// NORMAL OPEN EYE
// ------------------------------------------------------------

const uint8_t EYE_NORMAL[8] = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11100111,
  0b11100111,
  0b11111111,
  0b01111110,
  0b00111100
};


// ------------------------------------------------------------
// HALF CLOSED
// ------------------------------------------------------------

const uint8_t EYE_HALF_CLOSED[8] = {
  0b00000000,
  0b00000000,
  0b01111110,
  0b11111111,
  0b11100111,
  0b01111110,
  0b00111100,
  0b00000000
};


// ------------------------------------------------------------
// FULLY CLOSED
// ------------------------------------------------------------

const uint8_t EYE_CLOSED[8] = {
  0b00000000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00000000
};


// ------------------------------------------------------------
// HAPPY
// ------------------------------------------------------------

const uint8_t EYE_HAPPY[8] = {
  0b00000000,
  0b00000000,
  0b01100110,
  0b11111111,
  0b11000011,
  0b10000001,
  0b00000000,
  0b00000000
};


// ------------------------------------------------------------
// ERROR / SAD
// ------------------------------------------------------------

const uint8_t EYE_SAD[8] = {
  0b00000000,
  0b10000001,
  0b11000011,
  0b01100110,
  0b00111100,
  0b00011000,
  0b00000000,
  0b00000000
};


// ------------------------------------------------------------
// CONFUSED LEFT
// ------------------------------------------------------------

const uint8_t EYE_CONFUSED_LEFT[8] = {
  0b00011110,
  0b00111100,
  0b01111110,
  0b11100111,
  0b11000011,
  0b11011011,
  0b01111110,
  0b00111100
};


// ------------------------------------------------------------
// CONFUSED RIGHT
// ------------------------------------------------------------

const uint8_t EYE_CONFUSED_RIGHT[8] = {
  0b01111000,
  0b00111100,
  0b01111110,
  0b11100111,
  0b11000011,
  0b11011011,
  0b01111110,
  0b00111100
};


// ------------------------------------------------------------
// LISTENING - LOOK LEFT
// ------------------------------------------------------------

const uint8_t EYE_LOOK_LEFT[8] = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11110011,
  0b11110011,
  0b11111111,
  0b01111110,
  0b00111100
};


// ------------------------------------------------------------
// LISTENING - LOOK CENTRE
// ------------------------------------------------------------

const uint8_t EYE_LOOK_CENTRE[8] = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11100111,
  0b11100111,
  0b11111111,
  0b01111110,
  0b00111100
};


// ------------------------------------------------------------
// LISTENING - LOOK RIGHT
// ------------------------------------------------------------

const uint8_t EYE_LOOK_RIGHT[8] = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11001111,
  0b11001111,
  0b11111111,
  0b01111110,
  0b00111100
};


// ------------------------------------------------------------
// SPEAKING FRAME
// ------------------------------------------------------------

const uint8_t EYE_SPEAKING[8] = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b11100111,
  0b11100111,
  0b01111110,
  0b00111100,
  0b00011000
};


// ============================================================
// DRAWING FUNCTIONS
// ============================================================

void drawEye(
  uint8_t device,
  const uint8_t bitmap[8]
)
{
  for (uint8_t row = 0; row < 8; row++)
  {
    mx.setRow(
      device,
      row,
      bitmap[row]
    );
  }
}


void drawBothEyes(
  const uint8_t leftBitmap[8],
  const uint8_t rightBitmap[8]
)
{
  drawEye(
    LEFT_EYE_DEVICE,
    leftBitmap
  );

  drawEye(
    RIGHT_EYE_DEVICE,
    rightBitmap
  );
}


void drawMatchingEyes(
  const uint8_t bitmap[8]
)
{
  drawBothEyes(
    bitmap,
    bitmap
  );
}


// ============================================================
// RESET ANIMATION STATE
// ============================================================

void resetAnimationState()
{
  lastAnimationUpdate = millis();

  blinkActive = false;

  blinkStage = 0;

  listeningFrame = 0;

  speakingFrame = 0;

  nextIdleBlinkAt =
    millis() +
    random(
      IDLE_BLINK_MIN_MS,
      IDLE_BLINK_MAX_MS + 1
    );
}


// ============================================================
// DRAW INITIAL FRAME
// ============================================================

void drawStateInitialFrame()
{
  switch (currentState)
  {

    // --------------------------------------------------------
    // IDLE
    // --------------------------------------------------------

    case ST_IDLE:

      drawMatchingEyes(
        EYE_NORMAL
      );

      break;


    // --------------------------------------------------------
    // LISTENING
    // --------------------------------------------------------

    case ST_LISTENING:

      drawMatchingEyes(
        EYE_LOOK_CENTRE
      );

      break;


    // --------------------------------------------------------
    // THINKING
    // --------------------------------------------------------

    case ST_THINKING:

      drawMatchingEyes(
        EYE_CLOSED
      );

      break;


    // --------------------------------------------------------
    // SPEAKING
    // --------------------------------------------------------

    case ST_SPEAKING:

      drawMatchingEyes(
        EYE_NORMAL
      );

      break;


    // --------------------------------------------------------
    // HAPPY
    // --------------------------------------------------------

    case ST_HAPPY:

      drawMatchingEyes(
        EYE_HAPPY
      );

      break;


    // --------------------------------------------------------
    // CONFUSED
    // --------------------------------------------------------

    case ST_CONFUSED:

      drawBothEyes(
        EYE_CONFUSED_LEFT,
        EYE_CONFUSED_RIGHT
      );

      break;


    // --------------------------------------------------------
    // ERROR
    // --------------------------------------------------------

    case ST_ERROR:

      drawMatchingEyes(
        EYE_SAD
      );

      break;


    // --------------------------------------------------------
    // SLEEP
    // --------------------------------------------------------

    case ST_SLEEP:

      drawMatchingEyes(
        EYE_CLOSED
      );

      break;
  }
}


// ============================================================
// IDLE ANIMATION
// ============================================================

void updateIdle()
{
  unsigned long now = millis();


  // Start blink
  if (!blinkActive)
  {
    if (now >= nextIdleBlinkAt)
    {
      blinkActive = true;

      blinkStage = 1;

      lastAnimationUpdate = now;

      drawMatchingEyes(
        EYE_CLOSED
      );
    }

    return;
  }


  // Open again after short blink
  if (
    blinkStage == 1 &&
    now - lastAnimationUpdate >= 140
  )
  {
    drawMatchingEyes(
      EYE_NORMAL
    );

    blinkActive = false;

    blinkStage = 0;

    nextIdleBlinkAt =
      now +
      random(
        IDLE_BLINK_MIN_MS,
        IDLE_BLINK_MAX_MS + 1
      );
  }
}


// ============================================================
// LISTENING ANIMATION
// ============================================================

void updateListening()
{
  unsigned long now = millis();


  if (
    now - lastAnimationUpdate <
    LISTENING_FRAME_MS
  )
  {
    return;
  }


  lastAnimationUpdate = now;


  listeningFrame =
    (listeningFrame + 1) % 4;


  /*
      CENTRE
        ↓
      LEFT
        ↓
      CENTRE
        ↓
      RIGHT
        ↓
      repeat
  */


  switch (listeningFrame)
  {
    case 0:

      drawMatchingEyes(
        EYE_LOOK_CENTRE
      );

      break;


    case 1:

      drawMatchingEyes(
        EYE_LOOK_LEFT
      );

      break;


    case 2:

      drawMatchingEyes(
        EYE_LOOK_CENTRE
      );

      break;


    case 3:

      drawMatchingEyes(
        EYE_LOOK_RIGHT
      );

      break;
  }
}


// ============================================================
// THINKING
// ============================================================

void updateThinking()
{
  /*
    Nothing needs to animate.

    EYE_CLOSED is drawn once when
    entering THINKING state.
  */
}


// ============================================================
// SPEAKING ANIMATION
// ============================================================

void updateSpeaking()
{
  unsigned long now = millis();


  if (
    now - lastAnimationUpdate <
    SPEAKING_FRAME_MS
  )
  {
    return;
  }


  lastAnimationUpdate = now;


  speakingFrame =
    (speakingFrame + 1) % 2;


  if (speakingFrame == 0)
  {
    drawMatchingEyes(
      EYE_NORMAL
    );
  }
  else
  {
    drawMatchingEyes(
      EYE_SPEAKING
    );
  }
}


// ============================================================
// TOUCH SENSOR
// ============================================================

void updateTouchSensor()
{
  int reading =
    digitalRead(
      TOUCH_PIN
    );


  // Raw touch signal changed
  if (
    reading !=
    lastTouchReading
  )
  {
    lastTouchDebounceTime =
      millis();

    lastTouchReading =
      reading;
  }


  // Accept only stable signal
  if (
    millis() -
    lastTouchDebounceTime >
    TOUCH_DEBOUNCE_MS
  )
  {
    if (
      reading !=
      stableTouchState
    )
    {
      stableTouchState =
        reading;


      // Toggle only on press
      if (
        stableTouchState ==
        HIGH
      )
      {
        micEnabled =
          !micEnabled;
      }
    }
  }
}


// ============================================================
// BRIDGE FUNCTION - TOUCH
// ============================================================

bool isMicEnabled()
{
  return micEnabled;
}


// ============================================================
// BRIDGE FUNCTION - EYES
// ============================================================

void set_eye_state(
  String state
)
{
  state.trim();

  state.toUpperCase();


  EyeState requestedState;


  if (
    state == "IDLE"
  )
  {
    requestedState =
      ST_IDLE;
  }

  else if (
    state == "LISTENING"
  )
  {
    requestedState =
      ST_LISTENING;
  }

  else if (
    state == "THINKING"
  )
  {
    requestedState =
      ST_THINKING;
  }

  else if (
    state == "SPEAKING"
  )
  {
    requestedState =
      ST_SPEAKING;
  }

  else if (
    state == "HAPPY"
  )
  {
    requestedState =
      ST_HAPPY;
  }

  else if (
    state == "CONFUSED"
  )
  {
    requestedState =
      ST_CONFUSED;
  }

  else if (
    state == "ERROR"
  )
  {
    requestedState =
      ST_ERROR;
  }

  else if (
    state == "SLEEP"
  )
  {
    requestedState =
      ST_SLEEP;
  }

  else
  {
    // Unknown state
    return;
  }


  currentState =
    requestedState;

  stateChanged =
    true;
}


// ============================================================
// SETUP
// ============================================================

void setup()
{

  // ----------------------------------------------------------
  // TOUCH
  // ----------------------------------------------------------

  pinMode(
    TOUCH_PIN,
    INPUT
  );


  // ----------------------------------------------------------
  // MAX7219
  // ----------------------------------------------------------

  mx.begin();


  // Brightness: 0 - 15
  mx.control(
    MD_MAX72XX::INTENSITY,
    1
  );


  mx.clear();


  randomSeed(
    micros()
  );


  resetAnimationState();

  drawStateInitialFrame();


  // ----------------------------------------------------------
  // ROUTER BRIDGE
  // ----------------------------------------------------------

  Bridge.begin();


  // Linux controls eyes

  Bridge.provide_safe(
    "set_eye_state",
    set_eye_state
  );


  // App Lab reads mic state

  Bridge.provide(
    "isMicEnabled",
    isMicEnabled
  );
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{

  // ----------------------------------------------------------
  // ALWAYS READ TOUCH
  // ----------------------------------------------------------

  updateTouchSensor();


  // ----------------------------------------------------------
  // PROCESS EYE STATE CHANGE
  // ----------------------------------------------------------

  if (stateChanged)
  {
    stateChanged =
      false;


    resetAnimationState();


    drawStateInitialFrame();
  }


  // ----------------------------------------------------------
  // RUN CURRENT ANIMATION
  // ----------------------------------------------------------

  switch (currentState)
  {

    case ST_IDLE:

      updateIdle();

      break;


    case ST_LISTENING:

      updateListening();

      break;


    case ST_THINKING:

      updateThinking();

      break;


    case ST_SPEAKING:

      updateSpeaking();

      break;


    case ST_HAPPY:
    case ST_CONFUSED:
    case ST_ERROR:
    case ST_SLEEP:

      // Static expressions

      break;
  }
}