# config.py

import os

# ---------------------------------------------------
# Project Directories
# ---------------------------------------------------

# Folder where config.py is located
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Models folder
MODELS_DIR = os.path.join(BASE_DIR, "models")


# ---------------------------------------------------
# Speech-to-Text (Vosk)
# ---------------------------------------------------

VOSK_MODEL_PATH = os.path.join(
    MODELS_DIR,
    "vosk",
    "vosk-model-small-en-us-0.15"
)

# USB microphone sample rate
SAMPLE_RATE = 44100


# ---------------------------------------------------
# Large Language Model (Qwen)
# ---------------------------------------------------

# Path to llama.cpp executable
LLAMA_CPP_BINARY = os.path.join(
    BASE_DIR,
    "llama-bin",
    "llama-cli"
)

# Path to Qwen GGUF model
QWEN_MODEL_PATH = os.path.join(
    MODELS_DIR,
    "qwen",
    "Qwen2.5-0.5B-Instruct-Q4_K_M.gguf"
)

# Maximum number of generated tokens
MAX_TOKENS = 50


# ---------------------------------------------------
# Text-to-Speech (Piper)
# ---------------------------------------------------

# Piper executable
PIPER_BINARY = "piper"

# Piper voice model
PIPER_VOICE_MODEL = os.path.join(
    MODELS_DIR,
    "piper",
    "en_US-lessac-medium.onnx"
)
