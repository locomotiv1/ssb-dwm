# SSB-DWM

This bar is build only for dwm.

Other x11 clients  dont support it as far as i know

## Configuration

This project follows suckless configuration principles

#### **Config.h**
 
This  is your main config file where you make any changes to the bar

When you add your custom functions you need to remember to declare and format them properly

#### **Config.def.h**

It acts as a blueprint for default configuration/fallback.

## Installation

Copy the repo and run `sudo make clean install`


Add `ssb-dwm &` to your `.xinitrc` just before you execute dwm
