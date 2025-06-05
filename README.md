# Plugin_Development
Project to develop and test custom Unreal Engine gameplay plugins.

Plugin are developed in C++.

## Active development
Currently working on:
Upgradable system plugin - add an actor component that interacts with an upgradable subsystem to give any actor level upgrade functionality.
Resource management system.

### Upgradable Assets system

Drop component onto any Actor to give it "Upgradable" functionality.
Load upgrade paths as Json, DataTables or DataAssets - in Editor Preferences-> Upgrade System Settings
Modify Enums in "UpgradeLevelData.h" to contain your desired naming for Upgrade aspect (Level, tier, rank, etc.) and Upgrade Category (Unit, Equipment, Builging)
