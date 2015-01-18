Basic Damage
============
```
pure_dmg = ((skill + 100) / 2)*(base_dmg / 100)
damage =(pure_dmg - (pure_dmg * DamageResist / 100)) * scale * difficulty 
damage_inhead = damage * 2
```
**skill** - current weapon skill
**base_dmg** - base weapon damage
**scale** - attacker scale
**DamageResist** - enemy damage resistance
**difficulty** - from 0.5 to 2, where 1 - normal
