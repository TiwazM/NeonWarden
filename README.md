# NEON WARDEN

**Break the grid. Free the names.**

A cyberpunk action-platformer for the **MEGA65**, presented by **TSW**.

The city leases identities to its citizens. When your courier's name is revoked, fight through its districts, break into the relay network and free the stolen records. Collect credits, buy equipment from PATCH, cross dangerous rooftops and confront the Warden.

Built for native MEGA65 mode, with original pixel art and SID music, keyboard and joystick controls, and a deliberately unforgiving **1991** difficulty mode.

## Features

- Seven stages: **Sump Market, Floodworks, Archive Spire, Crown Rooftops, Coil Gardens, Skyway** and **Warden Core**.
- Directional blade combat and a limited-ammunition pistol.
- Collectible credit drops, equipment upgrades, repairs and a rechargeable force field.
- Scrolling city scenery, timed pit lasers and moving platforms—including a three-platform crossing with an enemy riding along.
- Three lives, separate health and shield displays, relay keys and a final boss keycard.
- Normal and 1991 difficulty, plus a **secret unlockable mode**.
- A top-five high-score table for 1991 runs.
- Native VIC-IV full-colour graphics, DMA-assisted rendering and double buffering.
- Original six-voice SID music with SID sound effects.
- PAL and NTSC support.

## Download and run

Use the latest packaged build from this repository's **Releases** page, when available.

The game is supplied as:

| File | Use |
| --- | --- |
| `NEONWARD.D81` | Mountable disk image containing the game |
| `NEONWARD.PRG` | Self-contained program for a compatible MEGA65 loader |
| `source/` | Assembly source, asset generators and build tools |

### On a MEGA65

1. Copy the release files to your SD card.
2. Mount `NEONWARD.D81` as drive 8.
3. In **native MEGA65 BASIC**, enter:

```basic
LOAD "NEONWARD",8
RUN
