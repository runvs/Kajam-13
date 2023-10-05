# Medibellum

This is Medibellum. It is developed using the [JamTemplateCpp](https://github.com/Laguna1989/JamTemplateCpp).

## Releases

* All releases can be found on github: [Link](https://github.com/runvs/Kajam-13/releases)
* There is also this itch.io page: [Link](https://runvs.itch.io/medibellum)

## Game Description

Introducing "Medibellum," an entertaining 2D pixelart autobattler that packs a punch despite being created in just one
month for the thrilling 13th Kajam!

In the realm of Medibellum, you find yourself thrust into an ancient conflict between red team and blue team. As a
valiant strategist, it's your duty to assemble an army of brave warriors and unleash their powerful might upon hordes of
menacing enemies. But be warned, there are flanks!

The game combines the addictive mechanics of an autobattler with the charm of retro-inspired pixel art. With only a
month of development time, Medibellum transports you to a nostalgic realm where your heroes come to life.

## How to play solo

* Start game_server
* Start game_client
* Make sure that you use your local ip (`127.0.0.1`)
* Add one bot to the game
* Press connect

## How to play over network

* Add port forwarding for the machine that runs the server (e.g. on your Fritzbox)
* Server starts game_server
* Server checks [his public ip](https://www.whatsmyip.org/)
* Client(s) start game_client
* Enter public IP of server
* Press connect

## Development Setup

### CMake

1. create build directory `build` at root level
2. open command line (win+r `cmd`)
3. navigate to the just created build folder
4. type `cmake ../`
5. Open generated solution file with Visual Studio

### Clion (Win, Linux)

1. open project folder in clion

### Mac

For building on mac os you have to install cmake and sfml via homebrew: `brew install cmake sfml`
If you use another package manager or want to install the dependencies yourself, you have to modify the paths
in `CMakeLists.txt`.


