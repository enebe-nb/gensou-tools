# Gensou tools

List of tools:
* Soku Events: captures character and decks from the game and shows in a browser source.
* Challonge: captures math start and results from challonge and shows in a browser source.

To change the css(layout) you must rebuid the javascript code.

## Soku Events
To use it, first add the `soku-events.dll` in your SWRSToys modules and active it, this will create a websocket server to comunicate with the game.
In OBS add the `soku-events.html` as source, the recommended resolution size is 1280x720.

## Challonge
In OBS add the `challonge.html` as source, this is a line feed, so its recommended to use very little height and high width in the resolution.
To select the tournament being showed it uses URL Hash(text after `#` sign in a URL), so in the source filename you should use `challonge.html#api_key=YOUR_CHALLONGE_APIKEY&tournament=TOURNAMENT_ID`, replacing `YOUR_CHALLONGE_APIKEY` and `TOURNAMENT_ID` for the correct values.

# Building
This is required in case you want to modify the code. For layout building only the javascript is enough. All ready tools will be copied to `dist/` folder.

## Javascript
Requires [NodeJS](https://nodejs.org) installed.

First open your console in source's root folder and use `npm install` to download dependencies. Modify the code then use `npm run-script build` to build it.

## Cpp
Requires [CMake](https://cmake.org) and a cpp envoirment installed.

First download the dependencies with `git submodule update --init --recursive`. Then you can generate your building files and compile the source with:
```sh
mkdir build
cd build
cmake ..
cmake --build . --config Release
```