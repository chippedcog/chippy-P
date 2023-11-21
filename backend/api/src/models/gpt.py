import requests
from openai import OpenAI
import env

openai_client = OpenAI(api_key=env.env_get_open_ai_api_key())

def gpt_completion_image_caption(image_base64, prompt="What's in this image?", max_tokens=300) -> str:
    print(f"[gpt_completion_image_caption] start")
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {env.env_get_open_ai_api_key()}"
    }
    json = {
        "model": "gpt-4-vision-preview",
        "messages": [
            {
                "role": "user",
                "content": [
                    { "type": "text", "text": prompt },
                    { "type": "image_url", "image_url": { "url": f"data:image/jpeg;base64,{image_base64}" } }
                ]
            }
        ],
        "max_tokens": max_tokens
    }
    # request
    response = requests.post("https://api.openai.com/v1/chat/completions", headers=headers, json=json)
    # request text
    response_text = response.json()["choices"][0]["message"]["content"]
    print(f"[gpt_completion_image_caption] text: {response_text}")
    return response_text

def whisper_speech_to_text(file_bytes):
    print(f"[whisper_speech_to_text] start")
    # transcribe
    transcript = openai_client.audio.transcriptions.create(
        model="whisper-1",
        file=file_bytes,
        language="en"
    )
    transcript_text = transcript.text
    # return transcription text
    print(f"[whisper_speech_to_text] text: {transcript_text}")
    return transcript_text
