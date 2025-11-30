# 🏎️ Projekt z przedmiotu Grafika 3D - Wyscig samochodowy 3D

***

##  Autor

* **Imię i Nazwisko:** [Julia Przezdzik]
* **Numer Albumu:** [418 000]

***

## Krótki Opis Funkcjonalności

Aplikacja jest interaktywną symulacją wyścigu samochodowego (dla jednego gracza) na pustynnej trasie. Wykorzystuje bibliotekę **SFML** i renderuje geometrię w **OpenGL (Fixed Pipeline)**, rozbudowaną o **Shadery GLSL** dla zaawansowanego oświetlenia Phonga.

### Główne Cechy:

* **Wizualizacja 3D:** Scena z modelami aut, statycznymi roslinami oraz teksturowanymi zasobami.
* **Logika Gry:** Wyścig o długości **300.0f** jednostek, rozpoczynany klawiszem **Spacja**.
* **Efekt Cząsteczkowy:** Dynamiczny system cząsteczek kurzu (piasku) generowany za poruszającymi się samochodami.

***

##  Instrukcja Uruchomienia

Dla MacOS: 
```bash
clang++ main.cpp -o CarRace -std=c++17 -xc++ --stdlib=libc++ \
    -Wno-deprecated-declarations \
    -isysroot $(xcrun --show-sdk-path) \
    -I/opt/homebrew/opt/sfml/include \
    -I$(xcrun --show-sdk-path)/usr/include \
    -L/opt/homebrew/opt/sfml/lib \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -framework OpenGL -framework GLUT


```

## Interakcja z programem
* ** Sterowanie kamerą: strzałki, przyciski O/P (przyblizanie/oddalanie)
* **Dwa rodzaje kamery: widok z gory (sterowalny), widok ruchomy zza samochodu - zmiana trybu kamery za pomocą klawisza C
* ** Sterowanie pojazdem: klawisze W/S (jazda w przod/tyl), Q - nitro

## Prezentacja gry
* **  Link do filmiku przedstawiajacego gre:https://drive.google.com/file/d/1D5IslLVTD3ksAeZBsXGbh9-RISnK7tNh/view?usp=share_link
