import os
import subprocess
import tempfile

from config import PIPER_BINARY, PIPER_VOICE_MODEL


def speak(text: str):
    # Temporary WAV file
    with tempfile.NamedTemporaryFile(
        suffix=".wav",
        delete=False
    ) as temp_file:
        wav_path = temp_file.name

    try:
        # Generate speech as a normal WAV file
        subprocess.run(
            [
                PIPER_BINARY,
                "--model",
                PIPER_VOICE_MODEL,
                "--output_file",
                wav_path,
            ],
            input=text,
            text=True,
            check=True,
        )

        # Play through PipeWire default sink
        subprocess.run(
            [
                "pw-play",
                wav_path
            ],
            check=True,
        )

    finally:
        if os.path.exists(wav_path):
            os.remove(wav_path)


if __name__ == "__main__":
    speak("Hello. This is a Bluetooth audio test from Lumi.")
