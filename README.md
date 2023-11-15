![](./docs/chippyalt.png)

# Chippy

This is a proof of concept build for a passive/ambient hardware assistant for me to use when I'm doing lab work. This will become drastically more complex a unique use case, but is acting as a starting point for writing generalizable functionality. This template/starter is meant to help answer many of the following questions for new comers to hardeware and this pattern of UX.

### Questions I had starting out

- **Architecture**
  - Having a mental model for handling video/audio streaming so it's query-able and stored in a sane way (sticky sessions if streaming to an API in front of a load balancer, considerations for video storage, how to pair video/audio). I made a multi-modal tool last year, but handling streams feels like a different animal (just sending images on a repeating interval doesn't feel good/right)
  - Having a mental model for how to react to that streamed data and push it to the related device/wearer of that device (ex: I forget to close a refrigerator door at the lab, it recognizes that from the stream, and pushes an audio message to my device)
- **Hardware**
  - Finding the right hardware parts for doing streaming (what cameras are good? what mics are good? How much battery do I need? Are these parts compatiable?)
  - Knowing what should be done on device (ex: gesture or magic word recognition) and if my board can support it (ex: how far can I go with a ESP32-S3? At what point does it lock up?) Would love to see an example of a button press that triggers active recording to start and then later on it can be a gesture based trigger.
  - Basics to electronics and configuring a board so I understand how to read/write data to pins and so I don't short circuit something.
- **Software**
  - Example strategies for processing video/audio (what's the best way to chunk media for this companion use case to avoid painful refactors later)
  - Example strategy for a hardware device that is polling for updates. (How can incoming media get processed server side, and push an event to that device? How is that link best tracked, UUID device ids that are created by the hardware when turned on?)
  - How to maybe switch modes between active/ambient capture. Maybe "ambient" is lower quality and frame rate. Then you can trigger on a device a temporary high capture rate or picture?

### Developing

1. `docker-compose up` from the root repo
2. upload a sketch to your ESP board (TODO: parts/build tutorial)
3. run ngrok so you can hit an external http server and point it at your localhost:3000 api (FYI, to force a http endpoint, add the --scheme=http flag to your command)

### Base Case Architecture

There are two core functionalities to start we're aiming for:

1. Action trigger for a question that takes both an audio clip of the person talking and an image when they began talking. Once processed server side, that should get pushed back to device through a speaker.
2. Streaming audio/video to be saved and processed for querying later (ex: vector embed similarity search). Eventually we want action triggers for start/stop of this.

There's many more beyond that, but step at a time. This project is going to stick to generalizable actions/features, and then people can take it how they want for their personal tools :)
