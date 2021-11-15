# OpenXCom: From the Ashes [![Workflow Status][workflow-badge]][actions-url]

[workflow-badge]: https://github.com/OpenXcom/OpenXcom/workflows/ci/badge.svg
[actions-url]: https://github.com/OpenXcom/OpenXcom/actions

[![OpenXcom FTA issues](https://img.shields.io/github/issues-raw/723Studio/OpenXcom_FTA?color=1182c3&logo=GitHub&labelColor=282828)](https://github.com/openapoc/openapoc/issues)
[![OpenXcom FTA Discord](https://img.shields.io/discord/699764481105854542?label=Discord&logo=discord&logoColor=ffffff&labelColor=7289DA&color=2c2f33)](https://discord.gg/epmtzH9) 
[![OpenXcom FTA Vkontakte](https://img.shields.io/static/v1?label=VK&logo=vk&logoColor=ffffff&message=Vstupaj%20|%2097&color=282828&labelColor=2787f5)](https://vk.com/xcomfta)
[![OpenXcom FTA Support](https://img.shields.io/static/v1?label=Support&logo=paypal&logoColor=ffffff&message=Me&color=282828&labelColor=2787f5)](http://x-comfromtheashes.tilda.ws/)
<a href="https://www.youtube.com/channel/UCt5SsOdMgRJuFK65Z1xdkhw" target="_blank" rel="nofollow">
<img alt="OpenXcom FTA YouTube Channel Subscribers" src="https://img.shields.io/youtube/channel/subscribers/UCt5SsOdMgRJuFK65Z1xdkhw?label=Youtube&style=social">
</a>
<a href="https://www.twitch.tv/finnik723studio" target="_blank" rel="nofollow">
<img alt="OpenXcom FTA Twitch Status" src="https://img.shields.io/twitch/status/finnik723studio?style=social">
</a>

<p align="center"><a href="https://x-comfromtheashes.tilda.ws" target="_blank" rel="nofollow"><img src="https://i.imgur.com/KPawjTn.png"/></a></p>

> This is a fork of [OpenXcom project](https://openxcom.org) - an open-source clone of the popular "UFO: Enemy Unknown" ("X-COM:
UFO Defense" in the USA release) and "X-COM: Terror From the Deep" videogames
by Microprose, that requires the original files to run, licensed under the GPL and written in C++ / SDL. It was founded by Finnik in June 2019.

> Current fork of the game engine inherits all features from OXC and OXCE ongoing development and 
provides several exclusive game mechanics. See [project's wiki](https://github.com/723Studio/OpenXcom_FTA/wiki) for more details.



## Table of Contents

* [Copyright](#copyright)
* [Overview and Features](#overview-and-features)
* [FAQ](#faq)
* [Installation](#installation)
* [Mods](#mods)
* [Directory Locations](#directory-locations)
  * [Windows](#windows)
  * [Mac OS X](#mac-os-x)
  * [Linux](#linux)
* [Configuration](#configuration)
* [Development](#development)


## Copyright
All rights for the original game and its resources belong to their respective owners. We do not encourage and do not support any form of illegal usage of the original game. We strongly advise to purchase the original game on GOG or other platforms. Pirate ISOs are not supported and will cause issues such as crashes and map problems with OpenXcom FTA.

## Overview and Features 
The X-Com initiative failed, with alien agents infiltrating the ranks of the governments of most major powers. The agency's base is in poor condition due to lack of funding. Most of the personnel were dismissed or killed. To win, the player will have to revive X-Com literally from the ashes ...

- Expand the gaming experience while maintaining the original setting and spirit of the game.
- The mod adding new content, missions and mechanics.
- The game has an alternative start and a prototype of the faction and diplomacy system.
- A covert operations system and some other original features are planned.
- ...

## FAQ
Q: At what stage is the development of the game?
> the game is in pre-alpha stage.

Q: Where can I join the community and chat or participate in development?
> join to our Discord channel https://discord.gg/epmtzH9 where you can find more then 180 fans.

Q: How and where can I translate the game into my language?
> answer.

Q: I am an artist, how can I help the project?
> answer.

Q: I can help the development if I test the game, where can I post bug reports?
> answer.

Q: I have some great ideas where can I share them?
> answer.

## Installation

OpenXcom requires a vanilla copy of the X-COM resources -- from either or both
of the original games.  If you own the games on Steam, the Windows installer
will automatically detect it and copy the resources over for you.

If you want to copy things over manually, you can find the Steam game folders
at:

    UFO: "Steam\SteamApps\common\XCom UFO Defense\XCOM"
    TFTD: "Steam\SteamApps\common\X-COM Terror from the Deep\TFD"

Do not use modded versions (e.g. with XcomUtil) as they may cause bugs and
crashes.  Copy the UFO subfolders to the UFO subdirectory in OpenXcom's data
or user folder and/or the TFTD subfolders to the TFTD subdirectory in OpenXcom's
data or user folder (see below for folder locations).

## Mods

Current engine version is designed to be used together with [X-COM: From the Ashes
mod](https://github.com/723Studio/X-Com-From-the-Ashes). Still, currently it is fully compatible 
with all OXC an OXCE mods, and does not have any game breaking changes. If you
face any bug with mod, please, let us know with created issue in this repository.
All major changes in this code, compared to OpenXcom or OpenXcom Extended are
locked behind `ftaGame` property (see [project's wiki](https://github.com/723Studio/OpenXcom_FTA/wiki) for more info.)

## Directory Locations

OpenXcom has three directory locations that it searches for user and game files:

<table>
  <tr>
    <th>Folder Type</th>
    <th>Folder Contents</th>
  </tr>
  <tr>
    <td>user</td>
    <td>mods, savegames, screenshots</td>
  </tr>
  <tr>
    <td>config</td>
    <td>game configuration</td>
  </tr>
  <tr>
    <td>data</td>
    <td>UFO and TFTD data files, standard mods, common resources</td>
  </tr>
</table>

Each of these default to different paths on different operating systems (shown
below).  For the user and config directories, OpenXcom will search a list of
directories and use the first one that already exists.  If none exist, it will
create a directory and use that.  When searching for files in the data
directory, OpenXcom will search through all of the named directories, so some
files can be installed in one directory and others in another.  This gives
you some flexibility in case you can't copy UFO or TFTD resource files to some
system locations.  You can also specify your own path for each of these by
passing a commandline argument when running OpenXcom.  For example:

    openxcom -data "$HOME/bin/OpenXcom/usr/share/openxcom"

or, if you have a fully self-contained installation:

    openxcom -data "$HOME/games/openxcom/data" -user "$HOME/games/openxcom/user" -config "$HOME/games/openxcom/config"

### Windows

User and Config folder:
- C:\Documents and Settings\\\<user\>\My Documents\OpenXcom (Windows 2000/XP)
- C:\Users\\\<user\>\Documents\OpenXcom (Windows Vista/7)
- \<game directory\>\user
- .\user

Data folders:
- C:\Documents and Settings\\\<user\>\My Documents\OpenXcom\data (Windows 2000/XP)
- C:\Users\\\<user\>\Documents\OpenXcom\data (Windows Vista/7/8)
- \<game directory\>
- . (the current directory)

### Mac OS X

User and Config folder:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/Library/Application Support/OpenXcom
- $HOME/.openxcom
- ./user

Data folders:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/Library/Application Support/OpenXcom (if $XDG\_DATA\_HOME is not defined)
- $XDG\_DATA\_DIRS/openxcom (for each directory in $XDG\_DATA\_DIRS if $XDG\_DATA\_DIRS is defined)
- /Users/Shared/OpenXcom
- . (the current directory)

### Linux

User folder:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/.local/share/openxcom (if $XDG\_DATA\_HOME is not defined)
- $HOME/.openxcom
- ./user

Config folder:
- $XDG\_CONFIG\_HOME/openxcom (if $XDG\_CONFIG\_HOME is defined)
- $HOME/.config/openxcom (if $XDG\_CONFIG\_HOME is not defined)

Data folders:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/.local/share/openxcom (if $XDG\_DATA\_HOME is not defined)
- $XDG\_DATA\_DIRS/openxcom (for each directory in $XDG\_DATA\_DIRS if $XDG\_DATA\_DIRS is defined)
- /usr/local/share/openxcom
- /usr/share/openxcom
- . (the current directory)

## Configuration

OpenXcom has a variety of game settings and extras that can be customized, both
in-game and out-game. These options are global and affect any old or new
savegame.

For more details please check the [wiki](https://ufopaedia.org/index.php/Options_(OpenXcom)).

## Development

OpenXcom requires the following developer libraries:

- [SDL](http://www.libsdl.org) (libsdl1.2)
- [SDL\_mixer](http://www.libsdl.org/projects/SDL_mixer/) (libsdl-mixer1.2)
- [SDL\_gfx](http://www.ferzkopp.net/joomla/content/view/19/14/) (libsdl-gfx1.2), version 2.0.22 or later
- [SDL\_image](http://www.libsdl.org/projects/SDL_image/) (libsdl-image1.2)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp), version 0.5.3 or later

The source code includes files for the following build tools:

- Microsoft Visual C++ 2010 or newer
- Xcode
- Make (see Makefile.simple)
- CMake

It's also been tested on a variety of other tools on Windows/Mac/Linux. More
detailed compiling instructions are available at the
[wiki](https://ufopaedia.org/index.php/Compiling_(OpenXcom)), along with
pre-compiled dependency packages.
