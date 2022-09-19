# JAM.V

*Just Another Music Visualizer*

## Elevator Pitch

JAM.V aims to innovate on existing graphical and music production software to create a free and open-source program that can quickly and easily tie music to 3D CGI effects. Using existing open-source AI software, JAM.V will be able to quickly split music into single-instrument stems. Then, users can create vibrant musical experiences by tying certain stems to specific graphical effects, like lighting, particles, and camera movement. Creative users with a talent for music or musical production can also use their own, non-AI-generated stems to efficiently build a music video that synchronizes perfectly with every part of their new song.


## Detailed Description

"Game Engine" and "Rendering Engine" are common terms these days, but what you won't find anywhere is a Music Video Engine. That's the innovation gap that JAM.V aims to fill by creating industry-standard software to quickly turn a static 3D scene into a dynamic environment where each piece of the scenery synchronizes with the music.

To accomplish this, JAM.V will provide many of the standard features expected of a game engine, including asset management, model rendering, particle rendering, complex lighting and shading, scene management, a "play" state, and more. Luckily for this hackathon project, JAM.V will be forked from [SHOE](https://github.com/crigney3/SHOE), an existing open-source rendering/game engine, so that hackathon work can focus on implementing the AI and audio engineering portions of the project.

The AI portion of the project focuses on two things:

* Splitting user-provided music files into component stems
* Automatically tying musical effects to graphical ones (stretch goal)

Splitting user-provided music files broadens the potential users of the app from just musicians/audio engineers to anyone who wants to make a music video. It also provides much easier testing for developers of JAM.V, since we can easily test on a large amount of different songs and stems. JAM.V will use [Spleeter.ai](https://github.com/deezer/spleeter) to split tracks.

Once users have built a scene they like, and processed their music as stems, JAM.V will provide a suite of tools to allow them to creatively tie events from the song to visual effects in their scene. Below are two lists that show what audio and visual effects will be available to tie together:

**Audio Triggers**
* Current volume
* Volume increase/decrease
* Tempo (or tempo change)
* Frequency in specific range plays (e.g. bass drum hit vs. high vocal note)
* Song/stem begins/ends
* Song/stem is silent

**Visual Effects**
* Increase/decrease intensity of one or more lights
* Change the range, color, or other property of one or more lights
* Emit a particle
* Change the position, scale, or rotation of an object
* Change the texture or material of an object
* Move the camera
* Change the properties of the camera
* Change the skybox texture or modify its color
* Tint the entire scene
* Apply or remove a post-processing effect

My favorite thing about this project is that there are definitely more audio and visual effects that JAM.V can support, and with each one, the opportunity for creative uses of them grows exponentially!

## Techstack

JAM.V is built on several existing open-source programs, including my custom rendering engine, [SHOE](https://github.com/crigney3/SHOE), and [Spleeter.ai](https://github.com/deezer/spleeter) for stem separation. SHOE is written in C++ and hlsl, and Spleeter uses python to interface with the AI and change the training data.

## Goals

### Minimum Viable Product Goals

* Modify existing shaders or create new ones to accept audio data for graphical calculations
* Provide a user-friendly interface for viewing stems
* Split music files into stems with Spleeter
* Convert SHOE's "Play" mode into a musical playback system
* Ensure there is a simple way, either in code or the UI, to align stems with each other so it sounds exactly like the un-split song
* Create code endpoints to link specific events in a stem/audio file to a specific change in graphics, such as a particle emitting everytime JAM.V detects a bass drum note. For just the minimum goal, this won't be linked to individual instruments, but instead focus on core data elements such as "when the volume of stem 1 increases, increase the brightness of this selected light"

### Main Goals

* Integrate Spleeter into SHOE with an interface so users can upload a file, have it processed into stems, and have SHOE immediately know about those stems for graphics changes
* Render a completed music video to a file
* Use training data in Spleeter to automatically classify stems with their instrument (piano, vocals, bass, guitar, drumkit, etc.)
* Create a UI system for quickly linking audio events to graphical events, on top of existing code endpoints

## Demo

Demo to be added soon!