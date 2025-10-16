# Tamagotchi

> **Disclaimer:**  
> This is a **practice project** and is meant for learning purposes only.  
> Some parts of the project are in **German**.

---

## 🧩 About

This project is a digital pet (Tamagotchi) built using an Arduino Uno.
The goal is to simulate a small, interactive creature that you can feed, play with, and take care of over time.
You interact with the pet through buttons, and your pet is shown on a LCD.

All graphics and sprites are self-made pixel art, designed specifically for this project.

---

## 🕹️ Features
- Pet stats: Happy, Hungry, Sleepy
- Automatic time-based events (pet gets hungry over time)

- Feed your digital pet
- Shop for food and accessories
- Let it sleep to recover energy
- Play mini-games to boost happiness

Neglecting your pet will decrease its stats and eventually, it can die.

Also, your data gets saved in the EEPROM.

---

## 🛠️ Hardware & Components

- Arduino Uno
- Nokia LCD 5110
- Push Buttons x3
- Breadboard & Jumpers
- Resistors

---

## ⚙️ Tech / Libraries

I've used these libraries:

```
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EEPROM.h
```
