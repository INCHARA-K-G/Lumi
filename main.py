import time

from speech import SpeechListener
from llm import generate_answer
from tts import speak
from eyes import set_eye_state
from touch_control import TouchController


print()
print("==========================================")
print("      LUMI - OFFLINE AI STUDY BUDDY")
print("==========================================")
print()


# ------------------------------------------------------------
# START TOUCH CONTROLLER
# ------------------------------------------------------------

touch = TouchController()


# ------------------------------------------------------------
# LOAD VOSK
# ------------------------------------------------------------

listener = SpeechListener(touch)


# ------------------------------------------------------------
# STARTUP INTRODUCTION
# ------------------------------------------------------------

try:
    print("[Lumi] Starting...")

    # Happy eyes during introduction
    set_eye_state("HAPPY")

    greeting = (
        "Hello! I am Lumi, your offline AI study buddy. "
        "Touch me whenever you want to ask something."
    )

    print("Lumi :", greeting)

    speak(greeting)

    set_eye_state("IDLE")


except Exception as error:
    print("[Startup Error]", error)

    set_eye_state("ERROR")
    time.sleep(2)
    set_eye_state("IDLE")


# ============================================================
# MAIN LOOP
# ============================================================

while True:

    try:

        # ----------------------------------------------------
        # IDLE
        # ----------------------------------------------------

        set_eye_state("IDLE")

        print()
        print("------------------------------------------")
        print("Lumi is ready.")
        print("Touch the sensor to start speaking.")
        print("------------------------------------------")

        # Wait here while mic is OFF.
        # IDLE eye animation continues on MCU.
        touch.wait_until_on()


        # ----------------------------------------------------
        # LISTENING
        # ----------------------------------------------------

        set_eye_state("LISTENING")

        print()
        print("🎙️ MIC ON")
        print("Lumi is listening...")

        # Records until Touch #2 turns mic OFF
        question = listener.listen_once()


        # ----------------------------------------------------
        # NOTHING RECOGNIZED
        # ----------------------------------------------------

        if not question:

            print()
            print("Lumi could not understand you.")

            set_eye_state("CONFUSED")
            time.sleep(1.5)

            set_eye_state("IDLE")

            continue


        # ----------------------------------------------------
        # QUESTION
        # ----------------------------------------------------

        print()
        print("You :", question)


        # ----------------------------------------------------
        # THINKING
        # ----------------------------------------------------

        # MCU will keep eyes CLOSED for THINKING
        set_eye_state("THINKING")

        print()
        print("Lumi is thinking...")

        answer = generate_answer(question)


        if not answer:
            raise RuntimeError(
                "Language model returned an empty answer."
            )


        print()
        print("Lumi :", answer)


        # ----------------------------------------------------
        # SPEAKING
        # ----------------------------------------------------

        set_eye_state("SPEAKING")

        speak(answer)


        # ----------------------------------------------------
        # BACK TO IDLE
        # ----------------------------------------------------

        set_eye_state("IDLE")


    except KeyboardInterrupt:

        print()
        print("Stopping Lumi...")

        set_eye_state("SLEEP")

        break


    except Exception as error:

        print()
        print("==========================================")
        print("               LUMI ERROR")
        print("==========================================")
        print(error)
        print()

        # Error expression
        set_eye_state("ERROR")

        time.sleep(2)

        # Recover instead of crashing
        set_eye_state("IDLE")

        print("Lumi recovered.")
        print("Waiting for another touch.")
