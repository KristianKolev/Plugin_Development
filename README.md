# Plugin Development Suite

A modular framework for prototyping, testing, and packaging Unreal Engine gameplay plugins.

**Status**: WIP — actively evolving components and subsystems.

---

## Project Structure

* **Plugins_Development/**: each feature lives in its own plugin directory.
* **Content/**: sample maps, assets, and data for demonstration and testing.

---

## Active Plugins

### Upgradable System

A data‑driven upgrade framework that lets any actor gain leveling, ranking, or tiered progression:

* **Component + Subsystem**: drop `UUpgradableComponent` onto actors; `UUpgradeManagerSubsystem` handles level state, validation, and timers.
* **Flexible Data Providers**: drop JSON files, DataTables or DataAssets into a single folder. The subsystem scans once and instantiates the providers required for the detected data types.
* **Dynamic Resource Interning**: at runtime, any `FName` resource type is interned into a shared array for minimal memory and fast lookups.
* **Enum‑driven**: extend `EUpgradableAspect` to add your custom upgrade progression types (e.g. Level, Tier, Rank). Extend `EUpgradableCategopry` e.g. Unit, Building, Equipment.
* **Blueprint & C++ API**: high‑level calls such as `RequestUpgradeForActor`, `GetUpgradeLevelForActor`, or batch queries by aspect or path.
* **UMG Integration**: customizable progress bar and countdown widget driven by timelines.

### Resource Management (Upcoming)

* Currency system
* Integration hooks for upgrade cost deduction and refunds
* Event‑based notifications
  
---

## Configuration

Open **Project Settings → Upgrade System Settings** and set the single folder that contains your upgrade definition files.

On startup the subsystem scans this folder once and creates provider instances for every supported data type found. Those providers then parse the results.
Provider detection is driven by a mapping of asset classes to provider types, so adding support for new data formats only requires updating this map.

1. **JSON files**: each file defines an `UpgradePath` (e.g. BasicUnit, AdvancedBuilding, Ring) and a `levels` array with resource costs, upgrade durations, and locked status. Field names can be customized in the **Json Field Names** section of the settings if your JSON schema uses different names.
2. **DataTables**: `UpgradePath` should be the table's name and each row struct (`FUpgradeDefinition`) represents one level.
3. **DataAssets**: Defines `UpgradePath` and `TArray<FUpgradeDefinition>`.
4. **DataAsset**: `UOnLevelUpVisualsDataAsset` bundles meshes, materials, and niagara systems that can be applied through `UUpgradableComponent::ChangeActorVisualsPerUpgradeLevel` in BP to change the visual appearance of an actor as it levels up.

Upgrade data definitions populate the central catalog (`UpgradePathId → TArray<FUpgradeDefinition>`) and the resource name table.

---

## System Architecture & Usage

1. **Attach Component**: add `UUpgradableComponent`, set `UpgradePathId` and `InitialLevel`.
2. **Register**: on `BeginPlay`, the component registers with the subsystem, which stores initial level in a protected map.
3. **Request Upgrade**: The UpgradeManagerSubsystem exposes functions to BPs. For any given component, you only need to cache its ID and can then operate on it through the subsystems API.
4. **Delegates**: There are several delegate to hook into that are defined on the `UUpgradableComponent`.
5. **Queries**: use subsystem methods to retrieve all components by aspect or category, filter by current level, or fetch next‑level costs and upgrade durations.


---

## Roadmap

* Core upgradable system — completed
* Visual countdown & progress widget — completed
* Resource management & inventory plugin — in progress
* Data‑oriented performance tuning — in progress
* Multiplayer replication authority — planned


---

## License & Contributions

See [LICENSE](LICENSE) for license terms. Contributions welcome via issues and pull requests on GitHub.

