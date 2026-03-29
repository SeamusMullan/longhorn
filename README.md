# Longhorn

**Longhorn** is a fast, visually appealing application launcher and menu utility for Linux. It acts as a drop-in replacement for tools like `dmenu` or `rofi`, bringing a modern frosted glass (aero glass) aesthetic to Wayland and X11 environments.

It reads newline-separated items from `stdin`, creates an interactive graphic filter in a dynamic frosted-glass bar, and prints the user's selection to `stdout`.

## Table of Contents

- Basic Usage
- Controls
- Command Line Options
- Configuration File
- Included Scripts

## Basic Usage

Similarly to `dmenu`, Longhorn receives data through `stdin` and writes the exact matched choice back to `stdout`.

Choose from a simple list of options:

```bash
echo -e "Firefox\nDiscord\nTerminal\nFiles" | longhorn
```

Use it horizontally at the bottom of the screen with a custom prompt:

```bash
echo -e "Yes\nNo\nCancel" | longhorn -b -p "Are you sure? "
```

Display items vertically in a 10-line spotlight window:

```bash
cat items.txt | longhorn -l 10
```

## Controls

Longhorn features full keyboard and mouse support:

- **Typing:** Immediately starts fuzzy-filtering the input list.
- **Arrows (`Up`/`Down`/`Left`/`Right`):** Move the selection cursor through the options.
- **`Enter`:** Confirm the currently selected item and output it.
- **`Escape`:** Close the launcher (returns exit code `1` with no output).
- **`Tab`:** Autocomplete the currently typed input with your current selection.
- **Mouse Click:** Clicks highlight and instantly confirm an item.
- **Mouse Scroll:** Scrolls through list items (in vertical view).

## Command Line Options

CLI flags override any options specified in your configuration file.

| Flag | Argument | Description |
| :--- | :--- | :--- |
| `-b` | None | Pin the menu to the bottom of the screen (instead of the top). |
| `-l` | `<number>` | Enable vertical list mode with a set number of visible item lines. (Default `0`, which uses a single horizontal line). |
| `-p` | `<string>` | Set a custom prompt string. (Default: `>`) |
| `-fn` | `<path>` | Path to a custom TTFile font file. |
| `-fs` | `<size>` | Font size in points. (Default: `16`) |
| `-c` | `<path>` | Load a custom INI config file instead of the default. |
| `-nh` | None | Disable selection history. By default, Longhorn remembers your most commonly chosen items and pins them to the top. |

## Configuration File

Longhorn automatically searches for an INI file located at `~/.config/longhorn/config.ini`. Command Line flags will take precedence over any settings here.

Here is an example `config.ini`:

```ini
[appearance]
# Layout options: top, bottom, center, left, right
layout = center
prompt = "run: "
bottom = false
lines = 8

[font]
font_path = /usr/share/fonts/TTF/JetBrainsMono-Regular.ttf
font_size = 18

[theme]
corner_radius = 8.0
# The tint layered over the frosted glass background (RGBA, 0.0 - 1.0)
tint_r = 0.1
tint_g = 0.1
tint_b = 0.15
tint_a = 0.4
```

## Built-in History

By default, Longhorn remembers your selection frequency. When you confirm a selection, it gets logged in `~/.cache/longhorn/history`. Subsequent launches will bias commonly chosen items to the front of the list automatically.
*To disable this behavior, pass the `-nh` flag.*

## Included Scripts

If you have Longhorn installed in your system `$PATH`, it comes bundled with pre-made helper scripts imitating classic Unix workflow commands:

1. **`longhorn_run`**: Scans your system's `$PATH` for executables and launches them, essentially acting as a 1:1 `dmenu_run` replacement.

   ```bash
   # Launch a program runner with a vertical, 15-line appearance
   longhorn_run -l 15
   ```

2. **`longhorn_path`**: Helper tool that simply yields the list of executables to be plugged into other command chains.
