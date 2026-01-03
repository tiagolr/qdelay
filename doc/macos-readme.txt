========== QDelay ==========
Copyright (C) 2026 Tilr

Because the builds are unsigned you may have to run the following commands:

sudo xattr -dr com.apple.quarantine /path/to/your/plugin/QDelay.component
sudo xattr -dr com.apple.quarantine /path/to/your/plugin/QDelay.vst3

The command above will recursively remove the quarantine flag from the plugins.

