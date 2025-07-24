# Physical Materials for RTS Terrain System

## Overview
This directory contains physical materials that define the terrain properties for the survival racing game. These materials are referenced by the `USurvivalMovementComponent` for terrain detection and movement modification.

## Required Physical Materials

### 1. PM_Grass.uasset
- **Movement Multiplier**: 1.0x (base speed)
- **Calorie Burn Multiplier**: 1.0x (base burn rate)
- **Usage**: Default terrain type for open fields and safe areas
- **Material Name Must Contain**: "Grass"

### 2. PM_Mud.uasset  
- **Movement Multiplier**: 0.714x (1.4x resistance)
- **Calorie Burn Multiplier**: 1.4x
- **Usage**: Challenging terrain requiring extra effort
- **Material Name Must Contain**: "Mud"

### 3. PM_Water.uasset
- **Movement Multiplier**: 0.625x (1.6x resistance) 
- **Calorie Burn Multiplier**: 1.6x
- **Usage**: River crossings and wet terrain
- **Material Name Must Contain**: "Water"

### 4. PM_Rock.uasset
- **Movement Multiplier**: 0.833x (1.2x resistance)
- **Calorie Burn Multiplier**: 1.2x  
- **Usage**: Rocky terrain and cliff areas
- **Material Name Must Contain**: "Rock"

## Creation Instructions (UE5 Editor Required)

1. **Create Physical Materials**:
   - In Content Browser, right-click → Physics → Physical Material
   - Name according to the pattern above (PM_Grass, PM_Mud, etc.)
   - Configure friction, restitution, and other physics properties as needed

2. **Apply to Landscape Materials**:
   - Create landscape material assets
   - Assign appropriate physical materials to each layer
   - Ensure material names contain the required keywords for detection

3. **Testing**:
   - Use the terrain detection system in `USurvivalMovementComponent::DetectTerrainType()`
   - Verify `OnTerrainChanged` delegate broadcasts correctly
   - Test audio footstep changes when moving between terrain types

## Integration with Movement System

The physical materials are detected by:
```cpp
// In SurvivalMovementComponent.cpp
if (PhysMat->GetName().Contains("Grass"))
{
    SetTerrainResistance(1.0f);
    NewTerrainType = ETerrainType::Grass;
}
// ... etc for other terrain types
```

## Audio Integration

Each terrain type can have associated footstep sounds configured in the movement component:
- `GrassFootstepSound`
- `MudFootstepSound` 
- `WaterFootstepSound`
- `RockFootstepSound`

These are played automatically when the character moves across different terrain types.