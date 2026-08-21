import json
import signal
import subprocess
import threading
import time

import vosk

from config import VOSK_MODEL_PATH, SAMPLE_RATE


vosk.SetLogLevel(-1)


class SpeechListener:

    def __init__(self, touch_controller):

        self.touch = touch_controller

        print("[Speech] Loading Vosk...")
        self.model = vosk.Model(VOSK_MODEL_PATH)
        print("[Speech] Vosk ready.")


    def listen_once(self):

        # -----------------------------------------
        # Wait for Touch #1
        # -----------------------------------------

        self.touch.wait_until_on()

        print()
        print("🎙️ LISTENING")
        print("Speak now.")
        print("Touch again when you finish speaking.")

        recognizer = vosk.KaldiRecognizer(
            self.model,
            SAMPLE_RATE
        )

        recognized_segments = []

        # -----------------------------------------
        # Open the proven USB microphone path
        # -----------------------------------------

        mic = subprocess.Popen(
            [
                "arecord",
                "-D", "plughw:0,0",
                "-t", "raw",
                "-f", "S16_LE",
                "-r", str(SAMPLE_RATE),
                "-c", "1"
            ],
            stdout=subprocess.PIPE,
            stderr=None
        )

        # Give arecord a moment to open the mic
        time.sleep(0.15)

        try:

            while self.touch.is_on():

                data = mic.stdout.read(4000)

                if not data:
                    time.sleep(0.01)
                    continue

                if recognizer.AcceptWaveform(data):

                    result = json.loads(
                        recognizer.Result()
                    )

                    text = result.get(
                        "text",
                        ""
                    ).strip()

                    if text:

                        recognized_segments.append(text)

                        print(
                            "[Speech] Recognized:",
                            text
                        )

        finally:

            # -----------------------------------------
            # Touch #2 -> stop microphone
            # -----------------------------------------

            if mic.poll() is None:

                mic.send_signal(signal.SIGINT)

                try:
                    mic.wait(timeout=2)

                except subprocess.TimeoutExpired:

                    mic.terminate()

                    try:
                        mic.wait(timeout=1)

                    except subprocess.TimeoutExpired:
                        mic.kill()


        # -----------------------------------------
        # Get the unfinished final words from Vosk
        # -----------------------------------------

        final_result = json.loads(
            recognizer.FinalResult()
        )

        final_text = final_result.get(
            "text",
            ""
        ).strip()

        if final_text:
            recognized_segments.append(final_text)


        complete_text = " ".join(
            recognized_segments
        ).strip()


        print()
        print("🎙️ LISTENING STOPPED")

        if complete_text:
            print("[Speech] Final text:", complete_text)
        else:
            print("[Speech] Final text: [EMPTY]")

        return complete_text
