![](./docs/chippyalt.jpg)

# chippy-P

Blueprints for common UX patterns when doing embedded hardware. Clonis the repo, docker-compose up the api, and get ready to slam your head into the world of embedded systems and c++!

This is a community effort to do minimal demos of UX patterns for hardware that can drive AI assistants, with high quality documentation for new folks to learn arduino/esp32/hardware. Often examples are incomplete, bespoke for hardware that isn't documented well, aren't communicating with external apis, and aren't handling concurrency. These are all challenges for experienced developers, and explict examples are lacking. This repo is here to change that!

### Structure for Blueprints

Each demo consists of two parts, a sketch/project directory you can find in `/device` and an API `backend/api/.../routes.py` file that serves the needed functionality by the hardware build.

```
chippy-P/
├── backend/
    └── api/
        ├── api.py
        └── blueprints/
            ├── ...
            └── sketch_narrator_camera/
                ├── ...
                └── routes.py
└── device/
    ├── ...
    └── sketch_narrator_camera
```

We are being opinionated about these demos, in that all have to utilize the Sanic API, PGVector (if needed), ESP32 chips/boards, and PlatformIO setups (instead of the Ardunio IDE, which you'll see some examples of from early learning). This will reduce context switching and let people see various approaches on common UX. **You should also expect a parts list and board build in each device directory** so you can find the parts you need to recreate.


### Starting the API and Uploading Your Device Code (aka 'flashing')

1. `docker-compose up` from the root repo
2. Run ngrok to have a proxy to point at your localhost:3000 api (force a http endpoint by adding the --scheme=http flag)
3. Install the [Arduino IDE](https://www.arduino.cc/en/software) for "sketch_xyz" projects. This is a simple way to get started. Install [Platform IO](https://platformio.org/) VSCode integration for those without a "sketch_" prefix, which takes a bit longer to setup but is an embedded engineering tool norm for complex projects (also compiles/uploads faster)
4. Set an `env.h` for the project you want to run on a board (TODO: have some way to take the ngrok proxy endpoint and pass it in with a setup script)
5. Upload a sketch to your ESP board (TODO: parts/build tutorial should be provided in each project folder)
6. Play!

### Contribute

We're going to be selective about what gets included in this repo so we can create a clear path for learning. It's more about patterns than complex system builds. Check the Github Issues for requests for contribution on what we think will be UX patterns for hardware companions!
