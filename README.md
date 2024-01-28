# metamod-AirSupport
Air Support plugin for Counter-Strike Revisited

# Dependency
* My fork of MetaMod, providing CPP20 modulize HLSDK. https://github.com/xhsu/metamod-p 
* My personal lib Hydrogenium. https://github.com/xhsu/Hydrogenium 

# Installation

1. Find the latest release.
1. Install MetaMod.
1. Put the binary DLL file into `Half-Life\cstrike\addons\metamod\dlls\` **CZero is supported.
1. Put the resources files into `Half-Life\cstrike\` accordingly.
1. Open `Half-Life\czero\addons\metamod\plugins.ini` and add the following as a new line.
```
win32 addons/metamod/dlls/AirSupport.dll
```
* It is recommanded that one put this plugin above AmXModX, a.k.a. `amxmodx_mm.dll`
* This plugin is strictly Windows-exclusive. Under no circumstances can this plugin be successfully compiled without MSVC.

# Configuration
| CVar | Value Range | Default | Explaination |
| ---- | ----------- | :-----: | ------------ |
| airsupport_ct_think | 0 <= val < ∞ | 12 | Interval between calling attempt from CT BOTs. 0 to turn it off. |
| airsupport_ter_think | 0 <= val < ∞ | 0 | Interval between calling attempt from TER BOTs. 0 to turn it off. |
| airsupport_player_cd | 0 <= val < ∞ | 6 | Interval between player calling, resource shared amongst the team. |
| airsupport_pas_speed | 0 <= val < ∞ | 1000 | Flying speed of Precise air strike projectile. |
| airsupport_gunship_radius | 0 <= val < ∞ | 500 | Gunship beacon searching radius. |
| airsupport_gunship_beacon_fx | 0 or 1 | 1 | Gunship beacon searching effect. |
| airsupport_gunship_holding | 0 <= val < ∞ | 25 | Time before gunship leaving the map. |

| Command | Availability | Explaination |
| ------- | :----------: | ------------ |
| airsupport_scanjetspawn | Server | Generate simple jet waypoint for the plugin. |
| airsupport_readjetspawn | Server | Reload jet waypoint binary file. |
| takeradio | Client | Give player a radio `CBasePlayerItem`. |
| airsupport_extra_info | Client | Query the technical info about `AirSupport` plugin which currently running in server. |

### Localization Texts
```
"AIRSUPPORT_REJECT_NO_JET_SPAWN"	"The pilot found nowhere to approach."  
"AIRSUPPORT_REJECT_NO_VALID_TRACELINE"	"The pilot has no clear sight."  
"AIRSUPPORT_REJECT_TIME_OUT"	"Airsupport cancelled due to insufficition communication."  
"AIRSUPPORT_REJECT_COVERED_LOCATION"	"The location cannot be cover by airsupport."  
"AIRSUPPORT_REJECT_HEIGHT_NOT_ENOUGH"	"The location is too elevated! (%s1)"  
"AIRSUPPORT_REJECT_COOLING_DOWN"	"The HQ is preparing for the next round!"  
"AIRSUPPORT_GUNSHIP_DESPAWNING"	"Gunship requires reload and leaving the area."  
"AIRSUPPORT_GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE"	"Another gunship had taken the air supremacy in the area!"  
"AIRSUPPORT_GUNSHIP_RESELECT_TARGET"	"New target had been informed."  
"AIRSUPPORT_HINT_PRESS_AND_HOLD"	"PRESS and HOLD to direct the bombardment."  
"AIRSUPPORT_HINT_RESEL_TARGET"	"Take out your radio again to manually select a target."  
```
# Showcase
## Precise air strike
Features a tracking, precise attack against individual.
![air_strike_0](https://user-images.githubusercontent.com/33283030/229968915-48c7b0e6-6ab3-4ba8-972a-38f1e92092e8.jpg)
![air_strike_1](https://user-images.githubusercontent.com/33283030/229968934-57db1915-ce25-4abd-8428-aac1b2cca247.jpg)
  
## Cluster bomb
To deal with a large group of enemy, use the wide-range cluster charge. This weapon fires multiple explosive projectiles that spread out over a wide area and detonate on impact. The cluster charge can inflict heavy damage and disorient the enemy, giving you an advantage in combat.
![cluster_1](https://user-images.githubusercontent.com/33283030/229970073-24594e0a-13b4-49fe-8f1d-d32b5ac456f7.jpg)
![cluster_2](https://user-images.githubusercontent.com/33283030/229970079-bcbdb789-a625-4cd6-884c-d7f75b7877e4.jpg)
![cluster_4](https://user-images.githubusercontent.com/33283030/229970098-ad3b25ca-9e16-44c0-b526-f09fc17e1d65.jpg)

## Carpet bombardment
To activate the function, you need to mark the pathway with beacons. The jet will then fly over them and release a charge on each one.
![carpet_0](https://user-images.githubusercontent.com/33283030/229970871-2b2de111-2aeb-456b-9aac-9c9f37ad05bd.jpg)
![carpet_1](https://user-images.githubusercontent.com/33283030/229970884-c9535e73-2029-4b4b-9a88-15cff87c9e3f.jpg)

## Gunship strike
Summon a AC130 in the sky and automatically kill all enemies in the zone. Grab out your radio to manually select your target.
![ac130_0](https://user-images.githubusercontent.com/33283030/229971405-7d771cd1-6532-42bf-a2f5-70c64d4e90e5.jpg)
![ac130_1](https://user-images.githubusercontent.com/33283030/229971413-c0457b14-60dd-4d1c-bdab-fca8c0b26b1e.jpg)

## Thermobaric weapon
Expect characters in the game server to be dead.
![fuelair_0](https://user-images.githubusercontent.com/33283030/229971833-98472d73-d0a9-4595-9042-4ab382d7d61c.jpg)
![fuelair_1](https://user-images.githubusercontent.com/33283030/229971850-36402afb-6738-4e91-a12a-ae3ae41ba463.jpg)
![fuelair_2](https://user-images.githubusercontent.com/33283030/229971859-b3baa191-b591-4638-beed-c18b094e5292.jpg)

## White phosphorus bomb
Splash a huge fire pool onto the zone, whoever being ignited at this stage will be burn to death with no exception, even with water.
![phosphorus_0](https://user-images.githubusercontent.com/33283030/229972189-e3ae59fe-3060-442b-b9ee-02da1ffbaebd.jpg)
![phosphorus_1](https://user-images.githubusercontent.com/33283030/229972197-76991708-854f-42fc-82e2-e13faaea06c3.jpg)
![phosphorus_2](https://user-images.githubusercontent.com/33283030/229972222-b902ccd6-819d-46cc-ba10-a8900d8e1e25.jpg)
![phosphorus_3](https://user-images.githubusercontent.com/33283030/229972234-ed2918b6-7af7-4705-8636-ff0b7df9696c.jpg)
