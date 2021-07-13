# rlpc  [Deprecated]
rlpc music player

## Anarchy reigns here. At your own risk.

### TODO:
Yandex.music:
- Playlists
- Two-factor auth

Other:
- Settings

### Working:
- Ya.music search and play demo tracks (FULL plays with login)
- Logging in to Ya.Music
- Play local tracks FLAC, AAC and mp3

### Dependencies:
- qt5
- gstreamer
- taglib
- libyandexmusic
- curl

#### Packages for arch linux
__Arch:__
```bash
sudo pacman -S --needed gstreamer gst-plugins-good qt5-base taglib libcurl-gnutls json-c 
```

### Compile:
```bash
./build.sh
```
