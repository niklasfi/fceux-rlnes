# fceux-rlnes

This is the code used in the [NES Flappy Bird Reinforcement Learning AI](https://www.youtube.com/watch?v=xM62SpKAZHU)-video showcasing how simple reinforcement learning works to train the flappy bird to fly.

This project was part of a hackathon and therefore put together in a rush. Please forgive any glitches in code quality. Most of the code in this repository is taken from the [fceux emulator project](http://www.fceux.com). We have simply injected our code into the emulation routine to alter the joypad inputs at appropriate times to make the bird fly. [You can have a look at the changes necessary to make this work by comparing the fceux-stock and the master branch.](https://github.com/niklasfi/fceux-rlnes/compare/fceux-stock...master)

## Making this work

- git clone this repository to your machine
- compile the code either by importing the fceux project file into visual studio (works best with visual studio 2010) or following the instructions in the `README` file (not this one, this is `README.md`)
- download the [flappy bird rom](http://forums.nesdev.com/viewtopic.php?f=22&t=11425)
- run bin/fceux and open the downloaded rom
- enjoy