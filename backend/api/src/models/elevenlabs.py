from io import BufferedReader
import os
import requests
import tempfile
import env

CHUNK_SIZE = 1024

def eleven_labs_text_to_speech(text: str, file_path: str, voice_id="Zlb1dXrM653N07WRdFW3", output_format="mp3_44100_64") -> BufferedReader:
    headers = {
        "Accept": "audio/mpeg",
        "Content-Type": "application/json",
        "xi-api-key": env.env_get_eleven_labs_api_key(),
    }
    json = {
    "text": text,
    "model_id": "eleven_turbo_v2",
    "voice_settings": {
        "stability": 0,
        "similarity_boost": 0,
        "style": 0,
        "use_speaker_boost": True
    }
    }
    # Request
    response = requests.post(f"https://api.elevenlabs.io/v1/text-to-speech/{voice_id}?optimize_streaming_latency=0&output_format={output_format}", headers=headers, json=json)

    # Write all respose chunks to a file (kinda jank saving to disk in this func but w/e)
    with open(file_path, 'wb') as f:
      for chunk in response.iter_content(chunk_size=CHUNK_SIZE):
          if chunk:
              f.write(chunk)

    # Return BufferReader
    return open(file_path, 'rb')
